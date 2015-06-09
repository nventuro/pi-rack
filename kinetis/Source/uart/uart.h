#ifndef UART_H_
#define UART_H_

#include "MKE02Z2.h"
#include "macros.h"

#define LOG_UART UART1

void uartInit(void);
void uartSend8(u8 data);
void uartSendArray(char *arr, int size);
void uartSendString(char *str);

#endif /* UART_H_ */
