#include "sadc.h"
#include "MKE02Z2.h"

void sadcInit(void)
{
	SIM->SCGC |= SIM_SCGC_ADC_MASK;
	
	// SC1 does not need to be written, the default values (single conversion, no interrupts) are good
	
	// SC2 does not need to be written, the default values (no compare, software trigger) are good
	
	// SC3: Bus clock divided by 4 for a frequency of 5MHz (max is 8MHz). High speed, short sameple time, 12-bit conversion.
    ADC->SC3 = (0x00 << ADC_SC3_ADICLK_SHIFT) | (0x02 << ADC_SC3_ADIV_SHIFT) | (0x02 << ADC_SC3_MODE_SHIFT);
	
    // FIFO is disabled, so SC4 is not written (disabled by default)
	
	return;
}

void sadcInitChannel(int channel)
{
	// Disable regular I/O for the requested ADC channel
	ADC->APCTL1 |= 1 << channel;
}
