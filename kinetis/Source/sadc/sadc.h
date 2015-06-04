#ifndef SADC_H_
#define SADC_H_

#include "MKE02Z2.h"
#include "macros.h"
#include "core_cm0plus.h"

void sadcInit(void);
void sadcInitChannel(int channel);

#define SADC_START_CONVERSION(channel) (ADC->SC1 = /*(ADC->SC1 & ~ADC_SC1_ADCH_MASK) | */channel) // // Read SC1, clearing the channel selection bits, and select a new channel

#define SADC_CONVERSION_DONE() (((ADC->SC1 & ADC_SC1_COCO_MASK) != 0) ? _TRUE : _FALSE) 

#define SADC_GET_RESULT() (ADC->R)

#endif /* SADC_H_ */
