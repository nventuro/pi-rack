#include "sliders.h"
#include "pinmap.h"
#include "periph/sadc/sadc.h"
#include "periph/rti/rti.h"

#define SLIDERS_AMOUNT 5
#define SLIDER_MAX_VAL 4095 // 12 bit conversion

int channels[SLIDERS_AMOUNT];
int values[SLIDERS_AMOUNT]; // From 0 to 100

void sliders_Measure(void);
void sliders_Tick(void *data, int period, int id);

void sliders_Init(void)
{
	sadc_Init();
	sadc_SetFIFOMode(SLIDERS_AMOUNT);

	channels[0] = SLIDER_1_ADC_CH;
	channels[1] = SLIDER_2_ADC_CH;
	channels[2] = SLIDER_3_ADC_CH;
	channels[3] = SLIDER_4_ADC_CH;
	channels[4] = VOL_SLIDER_ADC_CH;

	for (int i = 0; i < SLIDERS_AMOUNT; ++i)
	{
		sadc_InitChannel(channels[i]);	
	}
	
	sliders_Measure();
	
	rti_Init();
	rti_Register(sliders_Tick, NULL, RTI_MS_TO_TICKS(5), RTI_NOW);
}

void sliders_Measure(void)
{
	sadc_StartFIFOConversion(channels);
}

void sliders_Tick(void *data, int period, int id)
{
	if (sadc_IsConversionDone())
	{
		int results[SLIDERS_AMOUNT];
		sadc_GetFIFOResults(results);
		
		for (int i = 0; i < SLIDERS_AMOUNT; ++i)
		{
			values[i] = (results[i] * 100) / SLIDER_MAX_VAL;
		}
		
		sliders_Measure();
	}	
}

slider_pos sliders_GetPos(int param_idx)
{
	int val = values[param_idx];
	
	if (val <= 2) // From 0% to 2%
	{
		return BIG_DECREASE;
	}
	else if (val < 45) // From 2% to 45%
	{
		return SLIGHT_DECREASE;
	}
	else if (val <= 55) // From 45% to 55%
	{
		return KEEP;
	}
	else if (val < 98) // From 55% to 98%
	{
		return SLIGHT_INCREASE;
	}
	else // From 98% to 100%
	{
		return BIG_INCREASE;
	}
}

int sliders_GetVolume(void)
{
	return values[SLIDERS_AMOUNT - 1]; // The volume slider is the last slider
}
