#ifndef _RTI_H
#define _RTI_H

#include "utils.h"

#define RTI_FREQ (1000) // Hz. 
// In order to change this value, RTI_PRESCALER in rti.c must be changed and the file recompiled.
// Note that divider base also affects rti divider (check tables 2-7 and 2-8 of MC9S12(...).pdf)
#define RTI_PER (1.0/RTI_FREQ) // seconds

#define RTI_MS_TO_TICKS(ms) (((ms)*RTI_FREQ)/1000) // Converts miliseconds to rti_time

#define RTI_PIT_CH 0 // The RTI on the Kinetis uses this PIT (periodic interrupt timer) channel

typedef void (*rti_ptr) (void *data, int period, int id); // A function callback for registering in the RTI

void rti_Init(void);
// Initializes the RTI module. After this call, rti_Register can be called, and registered callbacks will start executing

int rti_Register(rti_ptr callback, void *data, int period, int delay);
// Registers a callback function to be called periodically every period*RTI_PER seconds, after an initial delay of delay*RTI_PER seconds.
// period and delay can be set using RTI_MS_TO_TICKS(ms).
// When callback is called, it receives data, period and its id. 
// callback is called with interrupts inhibited and MUST NOT disinhibit them.
// Returns the id of the registed callback (to be used later to change its parameters, such as period). 
	 
#define RTI_ALWAYS 1 // period for a function that will always be called (its frequency is RTI_FREQ)
#define RTI_ONCE 0 // period for a function that will only be called once after a certain delay
#define RTI_NOW 1 // delay for a function that will be called for the first time as soon as the RTI interrupts the CPU

void rti_SetPeriod(int id, int period); 
// Changes the period of a registered id

void rti_Cancel(int n); 
// Cancels a registered id

void rti_ISR(void); 
// Interrupt function. To be registered on the vector table under the PIT[RTI_PIT_CH] interrupt

#endif
