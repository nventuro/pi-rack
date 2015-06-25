#include "periph/uart/uart.h"
#include "periph/rti/rti.h"
#include "lcd/lcd.h"
#include "sliders/sliders.h"
#include "pinmap.h"
#include "utils.h"
#include "MKE02Z2.h"

void hardwareInit(void);

void testPrint(void *data, int period, int id);
void pinsPolling(void *data, int period, int id);

void loadEffectsFromHost(void);
void initializeEffects(void);

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
#define EFFECT_NAME_LENGTH 20
#define MAX_EFFECT_PARAMS 4
#define PARAM_NAME_LENGTH 5

typedef struct
{
	bool in_use;
	char name[PARAM_NAME_LENGTH];
	int min;
	int max;
	int regular;
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
	lcd_PrintRow("Waiting for host", 1);
	
	uartInit();
	
	loadEffectsFromHost();
	currEffect = 0;
	
	LED_ON(POWER_LED);
			
	rti_Init();
	rti_Register(testPrint, NULL, RTI_MS_TO_TICKS(20), RTI_NOW);
	rti_Register(pinsPolling, NULL, RTI_MS_TO_TICKS(5), RTI_NOW);
	
	slider_values[0].value = &values[0];
	slider_values[1].value = &values[1];
	slider_values[2].value = &values[2];
	slider_values[3].value = &values[3];
	slider_values[4].value = &values[4];
	
	sliders_Init();
	sliders_Set(slider_values);
	
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
		int eff_name_idx = 0;
		while ((eff_name_idx < EFFECT_NAME_LENGTH) && (received[read] != ESCAPE_SEPARATOR))
		{
			effects[effect_idx].name[eff_name_idx++] = received[read++];
		}
		
		// Read up to one character after the separator
		while (received[read++] != ESCAPE_SEPARATOR)
			;
		
		// We're now at the parameters
		
		int param_idx = 0;
		
		while (received[read] != ESCAPE_EFFECT_END)
		{
			int param_name_idx = 0;
			while ((param_name_idx < PARAM_NAME_LENGTH) && (received[read] != ESCAPE_SEPARATOR))
			{
				effects[effect_idx].params[param_idx].name[param_name_idx++] = received[read++];
			}
			
			// Read up to one character after the separator
			while (received[read++] != ESCAPE_SEPARATOR)
				;
			
			// We're now at the parameter values
			
			/// WIP
			// Skip all parameter values
			int param_value_idx = 0;
			while (param_value_idx < 3)
			{
				while (received[read++] != ESCAPE_SEPARATOR)
					;
				
				param_value_idx++;
			}
			/// WIP END
			
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
		if (buttons[i].toggleable)
		{
			if (!buttons[i].waiting_for_release)
			{
				if (IS_PIN_ON(buttons[i].gpio))
				{
					buttons[i].status = !buttons[i].status;
					buttons[i].waiting_for_release = _TRUE;
				}
			}
			else
			{
				if (IS_PIN_OFF(buttons[i].gpio))
				{
					buttons[i].waiting_for_release = _FALSE;
				}
			}
		}
		else
		{
			if (IS_PIN_ON(buttons[i].gpio))
			{
				if (buttons[i].waiting_for_release)
				{
					if (buttons[i].status) 
					{
						buttons[i].status = _FALSE; // The button's edge has already been processed
					}
				}
				else // If button was released, press
				{
					buttons[i].status = _TRUE;
					buttons[i].waiting_for_release = _TRUE;
				}
			}
			else
			{
				buttons[i].status = _FALSE;
				buttons[i].waiting_for_release = _FALSE;
			}
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
