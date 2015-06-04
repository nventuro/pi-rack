#include "sspi.h"
#include "MKE02Z2.h"

void sspiInit(bool master)
{
	// Send the bus clock to the module
	SIM->SCGC |= (SPI == SPI0) ? SIM_SCGC_SPI0_MASK : SIM_SCGC_SPI1_MASK;
	
	// Alternate pin mapping
#ifdef SPI0_ALT
	SIM->PINSEL |= SIM_PINSEL_SPI0PS_MASK;
#endif
	
	if (master == _TRUE)
		SPI->C1 |= SPI_C1_MSTR_MASK;
	
	// Active low, edges at start, lsb first
	SPI->C1 |= SPI_C1_CPOL_MASK | SPI_C1_CPHA_MASK | SPI_C1_LSBFE_MASK;

	// Slave-select auto generation
	SPI->C1 |= SPI_C1_SSOE_MASK;
	SPI->C2 |= SPI_C2_MODFEN_MASK;
	    
	// The bus clock is 20MHz. It is first divided by 5, and then by 8, for a total division by 40 and a SPI clock of 500kHz.
	SPI->BR = SPI_BR_SPPR(0x04) | SPI_BR_SPR(0x02); // 0x04 == 100 for a prescaler divisor by 5, 0x02 = 0010 for a second divisor by 8 
	
	// Module enable
	SPI->C1 |= SPI_C1_SPE_MASK;
	    
	return;
}

u8 sspiSend(u8 data)
{
	while ((SPI->S & SPI_S_SPTEF_MASK) == 0)
		;
	SPI->D = data;
	
	while ((SPI->S & SPI_S_SPRF_MASK) == 0)
		;
	
	return SPI->D;
}

bool sspiDataReady(void)
{
	if ((SPI->S & SPI_S_SPRF_MASK) != 0)
		return _TRUE;
	else
		return _FALSE;
}

u8 sspiReceive(void)
{
	return SPI->D;
}

void sspiStoreForSending(u8 data)
{
	if ((SPI->S & SPI_S_SPTEF_MASK) != 0)
		SPI->D = data; // Only store the new data if the transmit buffer is empty (otherwise, there's data already stored there)
	
	return;
}

u8 sspiBusyRead(void)
{
	while (sspiDataReady() == _FALSE)
			;
		
	return sspiReceive();
}

u16 sspiBusyRead16(void)
{
	u16 data = 0;
	
	//MSB first
	
	data = sspiBusyRead();
	data = (data << 8) | sspiBusyRead();
	
	return data;
}

u32 sspiBusyRead32(void)
{
	u32 data = 0;
	
	//MSB first
	
	data = sspiBusyRead();
	data = (data << 8) | sspiBusyRead();
	data = (data << 8) | sspiBusyRead();
	data = (data << 8) | sspiBusyRead();
	
	return data;
}
