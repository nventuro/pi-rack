#ifndef SADC_H_
#define SADC_H_

#include "MKE02Z2.h"
#include "utils.h"
#include "core_cm0plus.h"

void sadc_Init(void);
void sadc_InitChannel(int channel);

bool sadc_IsConversionDone(void);

void sadc_StartSingleConversion(int channel);
void sadc_GetSingleResult(int *result);

void sadc_SetFIFOMode(int depth); // depth is the number of channels to be read
void sadc_StartFIFOConversion(int channels[]);
void sadc_GetFIFOResults(int results[]);

#endif /* SADC_H_ */
