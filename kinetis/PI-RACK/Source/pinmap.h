#ifndef PINMAP_H_
#define PINMAP_H_

#include "MKE02Z2.h"
#include "utils.h"

// Inputs
#define PEDAL GPIO_PTB2

#define BUTTON_LEFT GPIO_PTD0
#define BUTTON_RIGHT GPIO_PTA0

// Three position selector
#define SELECTOR_1_A GPIO_PTE5
#define SELECTOR_1_B GPIO_PTE6

// Two position selector
#define SELECTOR_2_A GPIO_PTC5

// Four position selector
#define SELECTOR_3_A GPIO_PTE4
#define SELECTOR_3_B GPIO_PTB4
#define SELECTOR_3_C GPIO_PTD1

// Outputs
#define POWER_LED GPIO_PTB0
#define STATUS_LED GPIO_PTD5

// The LEDs are common anode (they must be brought low to be turned on)
#define LED_ON(led) (PIN_OFF(led))
#define LED_OFF(led) (PIN_ON(led))

#define SLIDER_1_ADC_CH 8
#define SLIDER_2_ADC_CH 9
#define SLIDER_3_ADC_CH 10
#define SLIDER_4_ADC_CH 11
#define VOL_SLIDER_ADC_CH 15

void gpioPinAssignements(void);

#endif /* PINMAP_H_ */
