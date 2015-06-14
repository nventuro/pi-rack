#include "periph/uart/uart.h"
#include "lcd/lcd.h"
#include "sliders/sliders.h"
#include "MKE02Z2.h"

void hardwareInit(void);
void gpioPinAssignements(void);

int main (void)
{
	hardwareInit();
		
	if ((SIM->SRSID & SIM_SRSID_WDOG_MASK) != 0) // Halt execution on watchdog reset
	{
		while(1)
			;
	}
	
	uartInit();
	uartSendString("AudioSystems - IO module online.\n\r");

	lcd_Init(LCD_2004);
	lcd_Print("AudioSytems online");
	
	sliders_Init();
	
	while (1)
		;
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
	
void gpioPinAssignements(void)
{
	//PIN_MAKE_INPUT(PEDAL);
	//PIN_MAKE_INPUT(BUTTON_LEFT);
	//PIN_MAKE_INPUT(BUTTON_RIGHT);
	
	//PIN_MAKE_OUTPUT(POWER_LED);
	//PIN_MAKE_OUTPUT(STATUS_LED);
	
	//PORT->PUEL |= PORT_PUEL_PTCPE5_MASK; // Enable pullup for PTC5
}
