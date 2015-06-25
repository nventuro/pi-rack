#include "sliders.h"
#include "pinmap.h"
#include "periph/sadc/sadc.h"
#include "periph/rti/rti.h"

void sliders_Tick(void *data, int period, int id);
void sliders_Measure(void);

slider_data_t slider_data[SLIDERS_AMOUNT];

int channels[SLIDERS_AMOUNT];

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

void sliders_Set(slider_data_t data[])
{
	for (int i = 0; i < SLIDERS_AMOUNT; ++i)
	{
		slider_data[i].value = data[i].value;
		slider_data[i].min = data[i].min;
		slider_data[i].max = data[i].max;
	}
}

void sliders_Tick(void *data, int period, int id)
{
	if (sadc_IsConversionDone())
	{
		int results[SLIDERS_AMOUNT];
		sadc_GetFIFOResults(results);
		sliders_Measure();
		
		for (int i = 0; i < SLIDERS_AMOUNT; ++i)
		{
			slider_ProcessMeasurement(results[i], i);
		}	
	}	
}

void sliders_Measure(void)
{
	sadc_StartFIFOConversion(channels);
}

void slider_ProcessMeasurement(int result, int index)
{
	/*// We need to determine where result lies: either on the maintain, slight modify 
	ADC_MAX_VALUE /2 */
	
	*(slider_data[index].value) = result;
}
