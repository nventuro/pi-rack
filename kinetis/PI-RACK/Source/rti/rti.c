#include "MKE02Z2.h"
#include "rti.h"
#include <stdlib.h>

#define RTI_SETPRESCALER(presc) (RTICTL = presc)
#define RTI_ENABLE_INTERRUPTS() (CRGINT_RTIE = 1)
#define RTI_CLEAR_FLAG() (CRGFLG_RTIF = 1)
#define RTI_SET_CLOCK_SOURCE(src) (CLKSEL_PLLSEL = src)

#define RTI_MAX_FCNS 20
#define RTI_IS_VALID_ID(id) (((id >= 0) && (id < RTI_MAX_FCNS)) ? _TRUE : _FALSE)

typedef struct {
	int period;
	int count;
	rti_ptr callback;
	void *data;
} rti_cb;

bool rti_isInit = _FALSE;

rti_cb rti_tbl[RTI_MAX_FCNS];

void rti_Init()
{
	if (rti_isInit == _TRUE)
		return;
	
	rti_isInit = _TRUE;
	
	int i;
	for (i = 0; i < RTI_MAX_FCNS; i++)
	{
		rti_tbl[i].callback = NULL;
	}
	
	// The RTI is implemented using the PIT
	SIM->SCGC |= SIM_SCGC_PIT_MASK;
	
	PIT->MCR = 0x00; // Enable module
	PIT->CHANNEL[RTI_PIT_CH].LDVAL = 20 * 1000 * 1000 / RTI_FREQ; // For a 20MHz clock
	PIT->CHANNEL[RTI_PIT_CH].TCTRL |= PIT_TCTRL_TIE_MASK; // Enable interrupts
	
	PIT->CHANNEL[RTI_PIT_CH].TCTRL |= PIT_TCTRL_TEN_MASK; // Start counting
	
	NVIC_EnableIRQ((RTI_PIT_CH == 0) ? PIT_CH0_IRQn : PIT_CH1_IRQn); // Enable interrupts on the NVIC
	
	return;
}


int rti_Register (rti_ptr callback, void *data, int period, int delay)
{
	int i;
	
	for (i = 0; i < RTI_MAX_FCNS; i++) 
	{
		if (rti_tbl[i].callback == NULL) 
		{
			rti_tbl[i].callback = callback;
			rti_tbl[i].data = data;
			rti_tbl[i].period = period;
			rti_tbl[i].count = delay;
			break;
		}
	}
		
	if (i == RTI_MAX_FCNS)
	{
		error("rti: attempted to store a callback but the memory is full.\n");
	}
	
	return i;
}


void rti_SetPeriod(int id, int period)
{
	if (!RTI_IS_VALID_ID(id))
	{
		return;
	}
	
	rti_tbl[id].period = period;
	
	return;
}


void rti_Cancel(int id)
{
	if (!RTI_IS_VALID_ID(id))
		return;
	
	rti_tbl[id].callback = NULL;
}


void rti_ISR(void)
{
	PIT->CHANNEL[RTI_PIT_CH].TFLG |= PIT_TFLG_TIF_MASK; // Clear interrupt flag
	
	int i;
	for (i = 0; i < RTI_MAX_FCNS; i++) 
	{
		if (rti_tbl[i].callback != NULL) 
		{
			if ((--rti_tbl[i].count) == 0) 
			{	
				rti_tbl[i].callback(rti_tbl[i].data, rti_tbl[i].period, i);
				
				if (rti_tbl[i].count != 0) // If the callback deleted itself and registered another function in the same place, count wont be 0
					break;
				
				if (rti_tbl[i].period == RTI_ONCE)
					rti_tbl[i].callback = NULL;
				else
					rti_tbl[i].count = rti_tbl[i].period;
			}
		}
	}

	return;
}
