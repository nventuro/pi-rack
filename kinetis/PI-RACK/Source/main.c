#include "periph/uart/uart.h"
#include "periph/rti/rti.h"
#include "lcd/lcd.h"
#include "sliders/sliders.h"
#include "pinmap.h"
#include "utils.h"
#include "MKE02Z2.h"
#include <string.h>

void hardwareInit(void);

void testPrint(void *data, int period, int id);
void pinsPolling(void *data, int period, int id);

void loadEffectsFromHost(void);
void initializeEffects(void);
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

#define MAX_EFFECTS 10
#define EFFECT_NAME_LENGTH 15
#define MAX_EFFECT_PARAMS 4
#define PARAM_NAME_LENGTH 5

#define NO_PARAM_MSG "----"
#define NO_PARAM_MSG_LENGHT 4 // 4 == strlen("----")

#define VOLUME_MSG "Volume"
#define VOLUME_MSG_LENGTH 6 // 6 == strlen("Volume")

#define PARAM_VALUE_LENGTH 4

// The different parameter values are stored in a somewhat special format: they are fixed point, but the value stored 
// is actually the represented value times 100. Therefore, each value holds two decimal digits.
// For example, 205 is actually 2.05, 5 is 0.05, and 1000 is 10.00
typedef int int2d;

int2d parseASCIIValue(char *buff, int *read);
void asciiToString(char *src, int *read, char *dst, int max_length);
int intPow(int base, int exp);
void int2dToASCII(char *buff, int2d value, int max_chars);

typedef struct
{
	bool in_use;
	char name[PARAM_NAME_LENGTH];
	
	int2d min;
	int2d max;
	int2d regular;
	int2d current;
} effect_parameter_data_t;

typedef struct
{
	bool in_use;
	char name[EFFECT_NAME_LENGTH];
	effect_parameter_data_t params[MAX_EFFECT_PARAMS];	
} effect_data_t;

effect_data_t effects[MAX_EFFECTS];

int currEffect;
int loadedEffects;

int main (void)
{
	hardwareInit();
		
	if ((SIM->SRSID & SIM_SRSID_WDOG_MASK) != 0) // Halt execution on watchdog reset
	{
		while(1)
			;
	}
	
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
			
	rti_Init();
	rti_Register(testPrint, NULL, RTI_MS_TO_TICKS(20), RTI_NOW);
	rti_Register(pinsPolling, NULL, RTI_MS_TO_TICKS(5), RTI_NOW);
	
	// Change for curr effect value
	slider_values[0].value = &values[0];
	slider_values[1].value = &values[1];
	slider_values[2].value = &values[2];
	slider_values[3].value = &values[3];
	slider_values[4].value = &values[4];
	
	sliders_Init();
	sliders_Set(slider_values);
	
	while (1)
		;
}

void testPrint(void *data, int period, int id)
{
	char buffer[50];
	int write = 0;
	
	if (buttons[PEDAL_IDX].status)
	{
		write += itoa(currEffect + 1, buffer + write, 10); // Effect number
	}
	else
	{
		write += itoa(0, buffer + write, 10); // Effect number
	}
	
	buffer[write++] = ' ';
	write += itoa(values[0], buffer + write, 10); // Effect value 1
	buffer[write++] = ' ';
	write += itoa(values[1], buffer + write, 10); // Effect value 2
	buffer[write++] = ' ';
	write += itoa(values[2], buffer + write, 10); // Effect value 3
	buffer[write++] = ' ';
	write += itoa(values[3], buffer + write, 10); // Effect value 4
	
	buffer[write++] = ' ';
	write += itoa(values[4], buffer + write, 10); // Volume
	
	buffer[write++] = '\n';
	buffer[write++] = '\r';
	
	uartSendArray(buffer, write);
}

#define ESCAPE_WHITESPACE '%'
#define ESCAPE_SEPARATOR '_'
#define ESCAPE_EFFECT_END '?'
#define ESCAPE_MESSAGE_END '&'

void loadEffectsFromHost(void)
{
	initializeEffects();
	
	char received[500];
	for (int i = 0; i < 500; ++i)
	{
		received[i] = '\0';
	}
	
		
	int write = 0;
	bool done = _FALSE;
	while ((!done) && (write < 500))
	{
		while (!uartIsDataReady())
			;
		
		received[write] = uartRead();
		if (received[write] == ESCAPE_MESSAGE_END)
		{
			done = _TRUE;
		}
		
		write++;
	}
	
	if (write > 500) 
	{
		error("Received message is over 500 characters and contains no ESCAPE_MESSAGE_END character.");
	}

	int read = 0;
	int effect_idx = 0;
	while ((read < write) && (received[read] != ESCAPE_MESSAGE_END))
	{
		// First comes the effect's name
		effects[effect_idx].in_use = _TRUE;
		
		asciiToString(received, &read, effects[effect_idx].name, EFFECT_NAME_LENGTH);
		
		// We're now at the parameters
		
		int param_idx = 0;
		
		while (received[read] != ESCAPE_EFFECT_END)
		{
			// Parameter name
			asciiToString(received, &read, effects[effect_idx].params[param_idx].name, PARAM_NAME_LENGTH);

			// Parameter values, in order: min - regular - max
			effects[effect_idx].params[param_idx].min = parseASCIIValue(received, &read);
			effects[effect_idx].params[param_idx].regular = parseASCIIValue(received, &read);
			effects[effect_idx].params[param_idx].max = parseASCIIValue(received, &read);
			
			// Each parameters starts set at the regular value
			effects[effect_idx].params[param_idx].current = effects[effect_idx].params[param_idx].regular;
			
			effects[effect_idx].params[param_idx].in_use = _TRUE;
			param_idx++;
			
			// We're now at the next parameter (or next effect, or message ending)
		}
		
		effect_idx++;
		
		read++; // Advance to the next effect (or message ending)
	}
	
	loadedEffects = effect_idx;
}

void initializeEffects(void)
{
	for (int i = 0; i < MAX_EFFECTS; ++i)
	{
		effects[i].in_use = _FALSE;
		for (int n = 0; n < EFFECT_NAME_LENGTH; ++n)
		{
			effects[i].name[n] = ' ';
		}
		
		for (int j = 0; j < MAX_EFFECT_PARAMS; ++j)
		{
			effects[i].params[j].in_use = _FALSE;
			for (int n = 0; n < PARAM_NAME_LENGTH; ++n)
			{
				effects[i].params[j].name[n] = ' ';
			}
		}
	}
}

void pinsPolling(void *data, int period, int id)
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
			currEffect = loadedEffects - 1;
		}
	}
	
	if (buttons[RIGHT_IDX].status)
	{
		currEffect++;
		if (currEffect >= loadedEffects)
		{
			currEffect = 0;
		}
	}
	
	printCurrentEffect();
}

void printCurrentEffect(void)
{
	lcd_ClearScreen();
	
	// Effect name
	strncpy(&(lcd_memory[0 * LCD_2004_COLS]), effects[currEffect].name, EFFECT_NAME_LENGTH);
	
	// Parameters
	for (int i = 0; i < MAX_EFFECT_PARAMS; ++i)
	{
		int start_row = ((i / 2) + 1);
		int start_col = (i % 2) * LCD_2004_COLS / 2;
				
		if (effects[currEffect].params[i].in_use)
		{
			strncpy(&(lcd_memory[start_row * LCD_2004_COLS + start_col]), effects[currEffect].params[i].name, PARAM_NAME_LENGTH);
			
			// Parameter values are written next to the parameter name, with an empty space between (hence the +1)
			int2dToASCII(&(lcd_memory[start_row * LCD_2004_COLS + start_col + PARAM_NAME_LENGTH + 1]), effects[currEffect].params[i].current, PARAM_VALUE_LENGTH);
		}
		else
		{
			strncpy(&(lcd_memory[start_row * LCD_2004_COLS + start_col]), NO_PARAM_MSG, min(PARAM_NAME_LENGTH, NO_PARAM_MSG_LENGHT));
		}
	}
	
	// Volume
	strncpy(&(lcd_memory[3 * LCD_2004_COLS]), VOLUME_MSG, VOLUME_MSG_LENGTH);
}


void asciiToString(char *src, int *read, char *dst, int max_length)
{
	int dst_idx = 0;
	while ((dst_idx < max_length) && (src[*read] != ESCAPE_SEPARATOR))
	{
		char val = src[(*read)++];
		if (val == ESCAPE_WHITESPACE)
		{
			val = ' ';
		}
		dst[dst_idx++] = val;
	}

	// Read up to one character after the separator
	while (src[(*read)++] != ESCAPE_SEPARATOR)
		;
}

int2d parseASCIIValue(char *buff, int *read)
{
	int2d result = 0;
	
	bool is_neg = _FALSE;
	bool seen_point = _FALSE;
	int decimals_seen = 0;
	
	char val; 
	while ((val = buff[(*read)++]) != ESCAPE_SEPARATOR)
	{
		if (val == '.')
		{
			seen_point = _TRUE;
		}
		else if (val == '-')
		{
			is_neg = _TRUE;
		}
		else
		{
			result = result * 10 + val - '0';
			if (seen_point)
			{
				decimals_seen++;
			}
		}
	}
	
	if (decimals_seen < 2)
	{
		result *= intPow(10, 2 - decimals_seen);
	}
	else if (decimals_seen > 2)
	{
		result /= intPow(10, decimals_seen - 2);
	}
	
	if (is_neg)
	{
		result *= -1;
	}
	
	return result;
}

void int2dToASCII(char *buff, int2d value, int max_chars)
{
	if (value < 0)
	{
		value *= -1;
		(*buff) = '-';
	}
	
	buff++;
	
	// Position where the decimal point would be (counting from the right, last character is position 1),
	// if the number was constrained to max_chars digits.
	// A dec_pos of 0 means the decimal point isn't displayed (it's right after the last digit).
	// A dec_pos of 1 means the decimal point is the last digit (in which case we use a length of max_chars - 1
	// and don't display the decimal point).
	// A dec_pos between 2 and max_chars - 1 means the decimal point is displayed.
	// A dec_pos >= max_chars means the number cannot be correctly displayed (it's too small).
	// A dec_pos < 0 means the number cannot be correctly displayed (it's too large).
	int dec_pos = -1;
	
	int upper_range;
	int lower_range;
	
	do
	{
		dec_pos++;
		
		upper_range = intPow(10, max_chars - dec_pos) - 1;
		lower_range = intPow(10, max_chars - 1 - dec_pos);
		
		if ((value / 100) > upper_range) // If value is too large
		{
			break;
		}
		
		if (dec_pos == (max_chars - 1)) // Value is either too small, or as small as possible (dec_pos being 
										// equal to maa_chars means there's no leading zero before the decimal point)
		{
			break;
		}
	}
	while (!(((value / 100) <= upper_range) && ((value / 100) >= lower_range)));
	
	int write_pos = (dec_pos != 1 ? 0 : 1); // If dec_pos is 1, we lose one of the digits (else there'd be no digit after the decimal point)
	int written_digits = 0;
	while (write_pos < max_chars)
	{
		if (((write_pos) == (max_chars - dec_pos)) && (dec_pos != 1)) // We skip the decimal point position rule for dec_pos = 1 (otherwise the point would be at the end)
		{
			buff[write_pos] = '.';
		}
		else
		{
			buff[write_pos] = (value / intPow(10, max_chars + 2 - 1 - dec_pos - written_digits) % 10) + '0';
			
			written_digits++;
		}
		     
		write_pos++;
	}
}

int intPow(int base, int exp)
{
	int result = 1;

	while ((exp--) > 0)
	{
		result *= base;
	}

    return result;
}

void hardwareInit(void)
{
    // Unlock watchdog
    WDOG->CNT = 0x20C5; 
    WDOG->CNT = 0x28D9;	 
 
    // Reconfigure watchdog (disabled, but updates are enabled)
    WDOG->CS1 = WDOG_CS1_UPDATE_MASK;
    WDOG->CS2 = 0x00; // Bus clock, no prescaler (divides by 256)
    WDOG->TOVAL8B.TOVALL = 0; // 50ns * 256 * 78 ~= 1ms
    WDOG->TOVAL8B.TOVALH = 80;
    WDOG->WIN = 0xF0; // Window mode is disabled
   
    // Enable FLASH and SWD
    SIM->SCGC = SIM_SCGC_FLASH_MASK | SIM_SCGC_SWD_MASK;
    
    // Enable the RESET and SWD pins
    SIM->SOPT = SIM_SOPT_RSTPE_MASK | SIM_SOPT_SWDE_MASK;
    
    SIM->BUSDIV = 0x00; // Don't divide the bus clock (both are 20MHz)
	
	// Enable OSC
	OSC->CR = OSC_CR_RANGE_MASK | OSC_CR_OSCOS_MASK | OSC_CR_OSCEN_MASK;
	
	// Wait for OSC to be initialized
	while(!(OSC->CR & OSC_CR_OSCINIT_MASK))
		;
		
	// Set the divider for a 10MHz input
	ICS->C1 = (ICS->C1 & ~(ICS_C1_RDIV_MASK)) | ICS_C1_RDIV(3);

	// Change the FLL reference clock to external clock
	ICS->C1 =  ICS->C1 & ~ICS_C1_IREFS_MASK;
		 
	while(ICS->S & ICS_S_IREFST_MASK)
		;
		
	// Wait for the FLL to lock
	while(!(ICS->S & ICS_S_LOCK_MASK))
		;
	
	ICS->C2 = (ICS->C2 & ~(ICS_C2_BDIV_MASK)) | ICS_C2_BDIV(1); // BDIV is 1 for a 20MHz clock
	
	// Clear the loss of lock bit
	ICS->S |= ICS_S_LOLS_MASK;	
	
	return;
}
