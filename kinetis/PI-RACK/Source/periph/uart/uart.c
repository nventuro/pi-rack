#include "uart.h"
#include <string.h>

void uartInit(void)
{
	SIM->SCGC |= SIM_SCGC_UART1_MASK;
	
	LOG_UART->C1 = 0x00;
	LOG_UART->C2 = UART_C2_TE_MASK;
	
	LOG_UART->BDH = UART_BDH_SBR(0);
	LOG_UART->BDL = UART_BDL_SBR(11);
	
	return;
}

void uartSend8(u8 data)
{
	while (!(LOG_UART->S1 & UART_S1_TDRE_MASK))
		;
   
   LOG_UART->D = data;
   
   return;
}

void uartSendArray(char *arr, int size)
{
	int i;
	for (i = 0; i < size; ++i)
	{
		uartSend8(arr[i]);
	}
}

void uartSendString(char *str)
{
	uartSendArray(str, strlen(str));
}
