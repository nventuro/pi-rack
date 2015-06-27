#include "effects.h"
#include "periph/uart/uart.h"

effect_data_t effects[MAX_EFFECTS];

int currEffect;
int numLoadedEffects;

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
			effects[i].params[j].current = 0;
			for (int n = 0; n < PARAM_NAME_LENGTH; ++n)
			{
				effects[i].params[j].name[n] = ' ';
			}
		}
	}
}

void loadEffectsFromHost(void)
{
	uartInit();
	
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
			effects[effect_idx].params[param_idx].min = asciiToInt2d(received, &read);
			effects[effect_idx].params[param_idx].regular = asciiToInt2d(received, &read);
			effects[effect_idx].params[param_idx].max = asciiToInt2d(received, &read);
			
			// Each parameters starts set at the regular value
			effects[effect_idx].params[param_idx].current = effects[effect_idx].params[param_idx].regular;
			
			effects[effect_idx].params[param_idx].in_use = _TRUE;
			param_idx++;
			
			// We're now at the next parameter (or next effect, or message ending)
		}
		
		effect_idx++;
		
		read++; // Advance to the next effect (or message ending)
	}
	
	numLoadedEffects = effect_idx;
}
