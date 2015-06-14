#include "sadc/sadc.h"
#include "uart/uart.h"
#include "rti/rti.h"
#include "lcd/lcd.h"
#include "pinmap.h"
#include "utils.h"
#include <stdlib.h>

void hardwareInit(void);

void print(void *data, int period, int id);

int main (void)
{
	hardwareInit();
		
	if ((SIM->SRSID & SIM_SRSID_WDOG_MASK) != 0) // Halt execution on watchdog reset
	{
		while(1)
			;
	}
	
	uartInit();
	
	rti_Init();
	rti_Register(print, NULL, RTI_MS_TO_TICKS(1000), RTI_NOW);
	
	uartSendString("AudioSystems - IO module online.\n\r");


	lcd_Init(LCD_2004);
	lcd_Print("Hola este es un super mensaje!");
	
	while (1)
	{
	}
}

void print(void *data, int period, int id)
{
	uartSendString("Periodic!\n\r");
}

void hardwareInit(void)
{
    // Unlock watchdog
    WDOG->CNT = 0x20C5; 
    WDOG->CNT = 0x28D9;	 
 
    // Reconfigure watchdog (disabled, but updates are enabled)
    WDOG->CS1 = WDOG_CS1_UPDATE_MASK;
    WDOG->CS2 = 0x00; // Bus clock, no prescaler (divides by 256)
    WDOG->TOVAL8B.TOVALL = 0; // 50ns * 256 * 78 ~= 1ms
    WDOG->TOVAL8B.TOVALH = 80;
    WDOG->WIN = 0xF0; // Window mode is disabled
   
    // Enable FLASH and SWD
    SIM->SCGC = SIM_SCGC_FLASH_MASK | SIM_SCGC_SWD_MASK;
    
    // Enable the RESET and SWD pins
    SIM->SOPT = SIM_SOPT_RSTPE_MASK | SIM_SOPT_SWDE_MASK;
    
    SIM->BUSDIV = 0x00; // Don't divide the bus clock (both are 20MHz)
	
	// Enable OSC
	OSC->CR = OSC_CR_RANGE_MASK | OSC_CR_OSCOS_MASK | OSC_CR_OSCEN_MASK;
	
	// Wait for OSC to be initialized
	while(!(OSC->CR & OSC_CR_OSCINIT_MASK))
		;
		
	// Set the divider for a 10MHz input
	ICS->C1 = (ICS->C1 & ~(ICS_C1_RDIV_MASK)) | ICS_C1_RDIV(3);

	// Change the FLL reference clock to external clock
	ICS->C1 =  ICS->C1 & ~ICS_C1_IREFS_MASK;
		 
	while(ICS->S & ICS_S_IREFST_MASK)
		;
		
	// Wait for the FLL to lock
	while(!(ICS->S & ICS_S_LOCK_MASK))
		;
	
	ICS->C2 = (ICS->C2 & ~(ICS_C2_BDIV_MASK)) | ICS_C2_BDIV(1); // BDIV is 1 for a 20MHz clock
	
	// Clear the loss of lock bit
	ICS->S |= ICS_S_LOLS_MASK;	
	
	return;
}
