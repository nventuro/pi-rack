#include "system.h"
#include "periph/uart/uart.h"
#include "periph/rti/rti.h"
#include "sliders/sliders.h"
#include "lcd/lcd.h"
#include <string.h>
#include "pinmap.h"
#include "protocol.h"
#include "ascii/ascii.h"
#include "effects/effects.h"
#include "utils.h"

#define SYSTEM_LOOP_PERIOD_MS 10

#define EFFECT_UPDATE_SLOWDOWN 10

void systemLoop(void);
void enableNextPass(void *data, int period, int id);

void updatePinStatus(void);
void updateEffectValues(void);
void printToLCD(void);
void sendDataToHost(void);
void intToBar(char *dst, int value, int min, int max, int length);

enum {
	PEDAL_IDX,
	LEFT_IDX,
	RIGHT_IDX,
	TOTAL_BUTTONS
} buttons_idx;

typedef struct
{	
	bool toggleable; // false for regular push buttons
	bool status; // Indicates toggle status for toggleable buttons, or button presses (rising edge only) for non toggleable
	bool waiting_for_release;
	int gpio;
} button_data_t;

button_data_t buttons[TOTAL_BUTTONS];

bool waitingForNextPass;
int passesUntilEffectUpdate;

void systemStart(void)
{
	gpioPinAssignements();
		
	LED_OFF(POWER_LED);
	LED_OFF(STATUS_LED);

	lcd_Init(LCD_2004);
	lcd_PrintRow("AudioSytems online", 0);
	lcd_PrintRow("     Please wait", 3);
	
	buttons[PEDAL_IDX].status = _FALSE;
	buttons[PEDAL_IDX].waiting_for_release = _FALSE;
	buttons[PEDAL_IDX].gpio = PEDAL;
	buttons[PEDAL_IDX].toggleable = _TRUE;
	
	buttons[LEFT_IDX].status = _FALSE;
	buttons[LEFT_IDX].waiting_for_release = _FALSE;
	buttons[LEFT_IDX].gpio = BUTTON_LEFT;
	buttons[LEFT_IDX].toggleable = _FALSE;
		
	buttons[RIGHT_IDX].status = _FALSE;
	buttons[RIGHT_IDX].waiting_for_release = _FALSE;
	buttons[RIGHT_IDX].gpio = BUTTON_RIGHT;
	buttons[RIGHT_IDX].toggleable = _FALSE;
	
	uartInit();
		
	loadEffectsFromHost();
	currEffect = 0;
	
	LED_ON(POWER_LED);
		
	sliders_Init();
		
	rti_Init();
	rti_Register(enableNextPass, NULL, RTI_MS_TO_TICKS(SYSTEM_LOOP_PERIOD_MS), RTI_NOW);
	
	waitingForNextPass = _FALSE;
	passesUntilEffectUpdate = 0;
	
	systemLoop();
}

void systemLoop(void)
{
	while (_TRUE)
	{
		while (waitingForNextPass)
			;
		
		updatePinStatus();
		
		if (buttons[PEDAL_IDX].status)
		{
			LED_ON(STATUS_LED);
		}
		else
		{
			LED_OFF(STATUS_LED);
		}
		
		if (buttons[LEFT_IDX].status)
		{
			currEffect--;
			if (currEffect < 0)
			{
				currEffect = numLoadedEffects - 1;
			}
		}
		
		if (buttons[RIGHT_IDX].status)
		{
			currEffect++;
			if (currEffect >= numLoadedEffects)
			{
				currEffect = 0;
			}
		}
		
		passesUntilEffectUpdate = (passesUntilEffectUpdate + 1) % EFFECT_UPDATE_SLOWDOWN;
		if (passesUntilEffectUpdate == 0)
		{
			updateEffectValues();	
		}
			
		printToLCD();
		
		sendDataToHost();
		
		waitingForNextPass = _TRUE;
	}
}

void enableNextPass(void *data, int period, int id)
{
	waitingForNextPass = _FALSE;
}

void updatePinStatus(void)
{
	for (int i = 0; i < TOTAL_BUTTONS; ++i)
	{
		if (IS_PIN_ON(buttons[i].gpio))
		{
			if (buttons[i].waiting_for_release)
			{
				if (!buttons[i].toggleable && buttons[i].status)
				{
					buttons[i].status = _FALSE; // The button's edge has already been processed, do nothing for toggleables
				}
			}
			else // Process button rising edge
			{
				if (buttons[i].toggleable)
				{
					buttons[i].status = !buttons[i].status;
				}
				else
				{
					buttons[i].status = _TRUE;
				}
				buttons[i].waiting_for_release = _TRUE;
			}
		}
		else
		{
			if (!buttons[i].toggleable)
			{
				buttons[i].status = _FALSE;
			}
			buttons[i].waiting_for_release = _FALSE;
		}
	}
}

void updateEffectValues(void)
{
	// Parameters
	for (int i = 0; i < MAX_EFFECT_PARAMS; ++i)
	{
		if (!effects[currEffect].params[i].in_use)
		{
			continue;
		}
		
		slider_pos pos = sliders_GetPos(i);
		effects[currEffect].params[i].current += getParameterIncrement(pos, effects[currEffect].params[i].min, effects[currEffect].params[i].max);
		
		if (effects[currEffect].params[i].current > effects[currEffect].params[i].max)
		{
			effects[currEffect].params[i].current = effects[currEffect].params[i].max;
		}
		else if (effects[currEffect].params[i].current < effects[currEffect].params[i].min)
		{
			effects[currEffect].params[i].current = effects[currEffect].params[i].min;
		}
	}
	
	// Volume
	int vol = sliders_GetVolume();
	volume = (vol * (MAX_VOLUME - MIN_VOLUME)) / 100 + MIN_VOLUME;
}

void printToLCD(void)
{
	updatingScreen = _TRUE;
	
	lcd_ClearScreen();
	
	// Effect name
	strncpy(&(lcd_memory[0 * LCD_2004_COLS]), effects[currEffect].name, EFFECT_NAME_LENGTH);
	
	// Volume
	strncpy(&(lcd_memory[2 * LCD_2004_COLS]), VOLUME_MSG, VOLUME_MSG_LENGTH);
	intToBar(&(lcd_memory[3 * LCD_2004_COLS]), volume, MIN_VOLUME, MAX_VOLUME, VOLUME_BAR_LENGTH);
	
	// Parameters
	for (int i = 0; i < MAX_EFFECT_PARAMS; ++i)
	{
		int start_row = i;
		int start_col = LCD_2004_COLS / 2;
		
		// Vertical separator
		lcd_memory[start_row * LCD_2004_COLS + start_col - 1] = '|';
		
		if (effects[currEffect].params[i].in_use)
		{
			strncpy(&(lcd_memory[start_row * LCD_2004_COLS + start_col]), effects[currEffect].params[i].name, PARAM_NAME_LENGTH);
			
			// Parameter values are written next to the parameter name
			int2dToASCII(&(lcd_memory[start_row * LCD_2004_COLS + start_col + PARAM_NAME_LENGTH]), effects[currEffect].params[i].current, PARAM_VALUE_LENGTH);
		}
		else
		{
			strncpy(&(lcd_memory[start_row * LCD_2004_COLS + start_col]), NO_PARAM_MSG, min(PARAM_NAME_LENGTH, NO_PARAM_MSG_LENGHT));
		}
	}
	
	updatingScreen = _FALSE;
}

void intToBar(char *dst, int value, int min, int max, int length)
{
	*(dst++) = '[';
		
	int notch = ((value - min) * (length - 2 - 1)) / (max - min);
	
	for (int i = 0; i < (length - 2); ++i)
	{
		if (i == notch)
		{
			*(dst++) = '|';
		}
		else
		{
			*(dst++) = '-';
		}
	}
	
	*(dst) = ']';
}

void sendDataToHost(void)
{
	char buffer[50];
	int write = 0;
	
	if (buttons[PEDAL_IDX].status)
	{
		write += itoa(currEffect + 1, &(buffer[write]), 10); // Effect number
	}
	else
	{
		write += itoa(0, &(buffer[write]), 10); // Effect number
	}
	
	buffer[write++] = ' ';
	write += int2dToASCII(&(buffer[write]), effects[currEffect].params[0].current, PARAM_VALUE_LENGTH); // Effect value 1
	buffer[write++] = ' ';
	write += int2dToASCII(&(buffer[write]), effects[currEffect].params[1].current, PARAM_VALUE_LENGTH); // Effect value 2
	buffer[write++] = ' ';
	write += int2dToASCII(&(buffer[write]), effects[currEffect].params[2].current, PARAM_VALUE_LENGTH); // Effect value 3
	buffer[write++] = ' ';
	write += int2dToASCII(&(buffer[write]), effects[currEffect].params[3].current, PARAM_VALUE_LENGTH); // Effect value 4
	
	buffer[write++] = ' ';
	write += int2dToASCII(&(buffer[write]), volume, PARAM_VALUE_LENGTH); // Volume
	
	buffer[write++] = '\n';
	buffer[write++] = '\r';
	
	uartSendArray(buffer, write);
}
