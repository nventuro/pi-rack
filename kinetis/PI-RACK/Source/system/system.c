#include "system.h"

#include "periph/uart/uart.h"
#include "periph/rti/rti.h"
#include "lcd/lcd.h"
#include "sliders/sliders.h"
#include "pinmap.h"
#include "utils.h"
#include "MKE02Z2.h"
#include <string.h>
#include "protocol.h"
#include "ascii/ascii.h"
#include "effects/effects.h"

#define SYSTEM_LOOP_PERIOD_MS 10

void enableNextPass(void *data, int period, int id);

void testPrint(void);
void pinsPolling(void);
void printCurrentEffect(void);

int values[5];
slider_data_t slider_values[5];

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

void systemLoop(void);

bool waitingForNextPass;

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
		
		// Change for curr effect value
		slider_values[0].value = &values[0];
		slider_values[1].value = &values[1];
		slider_values[2].value = &values[2];
		slider_values[3].value = &values[3];
		slider_values[4].value = &values[4];
		
		sliders_Init();
		sliders_Set(slider_values);
		
	rti_Init();
	rti_Register(enableNextPass, NULL, RTI_MS_TO_TICKS(SYSTEM_LOOP_PERIOD_MS), RTI_NOW);
	
	systemLoop();
}

void systemLoop(void)
{
	while (_TRUE)
	{
		while (waitingForNextPass)
			;
		
		pinsPolling();
		
		testPrint();
		
		printCurrentEffect();
		
		waitingForNextPass = _TRUE;
	}
}

void enableNextPass(void *data, int period, int id)
{
	waitingForNextPass = _FALSE;
}

void printCurrentEffect(void)
{
	lcd_ClearScreen();
	
	// Effect name
	strncpy(&(lcd_memory[0 * LCD_2004_COLS]), effects[currEffect].name, EFFECT_NAME_LENGTH);
	
	// Parameters
	for (int i = 0; i < MAX_EFFECT_PARAMS; ++i)
	{
		int start_row = i;
		int start_col = LCD_2004_COLS / 2;
		
		// Separator
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
	
	// Volume
	strncpy(&(lcd_memory[2 * LCD_2004_COLS]), VOLUME_MSG, VOLUME_MSG_LENGTH);
	strncpy(&(lcd_memory[3 * LCD_2004_COLS]), "[-|-----]", 9);
}


void testPrint(void)
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
	write += itoa(values[4], &(buffer[write]), 10); // Volume
	
	buffer[write++] = '\n';
	buffer[write++] = '\r';
	
	uartSendArray(buffer, write);
}

void pinsPolling(void)
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
	
	printCurrentEffect();
}
