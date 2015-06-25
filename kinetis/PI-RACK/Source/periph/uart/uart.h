#ifndef UART_H_
#define UART_H_

#include "MKE02Z2.h"
#include "utils.h"

#define UART_MODULE UART1

void uartInit(void);
void uartSend8(u8 data);
void uartSendArray(char *arr, int size);
void uartSendString(char *str);

bool uartIsDataReady(void);
char uartRead(void);

#endif /* UART_H_ */
