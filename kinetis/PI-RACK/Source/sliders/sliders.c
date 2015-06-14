#include "sliders.h"
#include "pinmap.h"
#include "periph/sadc/sadc.h"
#include "periph/rti/rti.h"

void sliders_Tick(void *data, int period, int id);
void sliders_Measure(void);

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

	int i;
	for (i = 0; i < SLIDERS_AMOUNT; ++i)
	{
		sadc_InitChannel(channels[i]);	
	}
	
	sliders_Measure();
	
	rti_Init();
	rti_Register(sliders_Tick, NULL, RTI_MS_TO_TICKS(5), RTI_NOW);
}

void sliders_Set(int *value, int min, int max)
{
}

/*
#include "periph/uart/uart.h"
#include "utils.h"
*/

void sliders_Tick(void *data, int period, int id)
{
	if (sadc_IsConversionDone())
	{
		int results[SLIDERS_AMOUNT];
		sadc_GetFIFOResults(results);
		sliders_Measure();
		
		/*
		char buffer[50];
		int i;
		for (i = 0; i < SLIDERS_AMOUNT; ++i)
		{
			int len = itoa(results[i], buffer, 10);
			uartSendArray(buffer, len);
			uartSendString(", ");
		}
		uartSendString("\n\r");
		*/
	}	
}

void sliders_Measure(void)
{
	sadc_StartFIFOConversion(channels);
}
