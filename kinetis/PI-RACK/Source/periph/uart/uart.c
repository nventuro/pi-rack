#include "uart.h"
#include <string.h>

bool uartIsInit = _FALSE;

void uartInit(void)
{
	if (uartIsInit)
	{
		return;
	}
	
	SIM->SCGC |= SIM_SCGC_UART1_MASK;
	
	UART_MODULE->C1 = 0x00;
	UART_MODULE->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK; // Transmit and receive
	
	UART_MODULE->BDH = UART_BDH_SBR(0);
	UART_MODULE->BDL = UART_BDL_SBR(11);

	uartIsInit = _TRUE;
	
	return;
}

void uartSend8(u8 data)
{
	while (!(UART_MODULE->S1 & UART_S1_TDRE_MASK))
		;
   
	UART_MODULE->D = data;
   
   return;
}

void uartSendArray(char *arr, int size)
{
	for (int i = 0; i < size; ++i)
	{
		uartSend8(arr[i]);
	}
}

void uartSendString(char *str)
{
	uartSendArray(str, strlen(str));
}

bool uartIsDataReady(void)
{
	return (UART_MODULE->S1 & UART_S1_RDRF_MASK);
}

char uartRead(void)
{
	return UART_MODULE->D;
}
