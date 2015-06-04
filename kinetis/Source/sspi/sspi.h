#ifndef SSPI_H_
#define SSPI_H_

#include "macros.h"

#define SPI SPI1
#define SPI0_ALT // Map the SPI0 signals to PTE0, PTE1, PTE2 and PTE3

void sspiInit(bool master);

// Master functions
u8 sspiSend(u8 data); // Sends (and receives) data

// Slave functions
bool sspiDataReady(void); // Informs if new data has been received
u8 sspiReceive(void); // Reads the received data
u8 sspiBusyRead(void);
u16 sspiBusyRead16(void);
u32 sspiBusyRead32(void);
void sspiStoreForSending(u8 data); // Stores data so that it can be sent on the next reception

#endif /* SSPI_H_ */
