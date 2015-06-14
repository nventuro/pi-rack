#include "sadc.h"
#include "MKE02Z2.h"

bool FIFOMode;
int FIFODepth;

void sadc_Init(void)
{
	SIM->SCGC |= SIM_SCGC_ADC_MASK;
	
	// SC1 does not need to be written, the default values (single conversion, no interrupts) are good
	
	// SC2 does not need to be written, the default values (no compare, software trigger) are good
	
	// SC3: Bus clock divided by 4 for a frequency of 5MHz (max is 8MHz). High speed, short sameple time, 12-bit conversion.
    ADC->SC3 = (0x00 << ADC_SC3_ADICLK_SHIFT) | (0x02 << ADC_SC3_ADIV_SHIFT) | (0x02 << ADC_SC3_MODE_SHIFT);
	
    // FIFO is enabled separately, so SC4 is not written (disabled by default)
    FIFOMode = _FALSE;
    
	return;
}

void sadc_InitChannel(int channel)
{
	// Disable regular I/O for the requested ADC channel
	ADC->APCTL1 |= 1 << channel;
}

bool sadc_IsConversionDone(void)
{
	return ((ADC->SC1 & ADC_SC1_COCO_MASK) != 0) ? _TRUE : _FALSE;
}

void sadc_StartSingleConversion(int channel)
{
	if (FIFOMode)
	{
		error("sadc: attempt to do a single conversion while in FIFO mode.");
	}
	else
	{
		ADC->SC1 = (ADC->SC1 & ~ADC_SC1_ADCH_MASK) | ADC_SC1_ADCH(channel); // // Read SC1, clearing the channel selection bits, and select a new channel
	}
}

void sadc_GetSingleResult(int *result)
{
	if (!sadc_IsConversionDone())
	{
		error("sadc: attempt to read an unfinished conversion's result.");
	}
	else if (FIFOMode)
	{
		error("sadc: attempt to read a single conversion result while in FIFO mode.");
	}
	else
	{
		*result = ADC->R;	
	}
}

void sadc_SetFIFOMode(int depth)
{
	FIFOMode = _TRUE;
	FIFODepth = depth;
	
	ADC->SC4 = ADC_SC4_AFDEP(depth - 1); // The ADC module interprets a depth value of 1 as a 2 channel FIFO
}

void sadc_StartFIFOConversion(int channels[])
{
	if (!FIFOMode)
	{
		error("sadc: attempt to do a FIFO conversion while in single mode.");
	}
	else
	{
		int i;
		for (i = 0; i < FIFODepth; ++i)
		{
			ADC->SC1 = (ADC->SC1 & ~ADC_SC1_ADCH_MASK) | ADC_SC1_ADCH(channels[i]); // Clear the channel selection bits and select each channel, in order
		}
	}
}

void sadc_GetFIFOResults(int results[])
{
	if (!sadc_IsConversionDone())
	{
		error("sadc: attempt to read an unfinished conversion's result.");
	}
	else if (!FIFOMode)
	{
		error("sadc: attempt to read a FIFO conversion result while in single mode.");
	}
	else
	{
		int i;
		for (i = 0; i < FIFODepth; ++i)
		{
			results[i] = ADC->R;
		}
	}
}
