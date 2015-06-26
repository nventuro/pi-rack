#include "lcd.h"
#include "MKE02Z2.h"

#define LCD_RS_SET_INSTR() PIN_OFF(LCD_RS)
#define LCD_RS_SET_DATA() PIN_ON(LCD_RS)

#define LCD_DATA_LOW GLUE2(GPIO_PT, LCD_DATA_LOW_PORT, LCD_DATA_LOW_START)
#define LCD_DATA_HIGH GLUE2(GPIO_PT, LCD_DATA_HIGH_PORT, LCD_DATA_HIGH_START)

#define LCD_DATA_SHIFT(lcd_data) (lcd_data < GPIO_PTE0 ? lcd_data : (lcd_data - GPIO_PTE0))

#define LCD_CLEAR_DISP BIT(0)

#define LCD_CURSOR_HOME BIT(1)

#define LCD_ENTRY_MODE BIT(2)
#define LCD_MOVE_DIR_UP BIT(1)
#define LCD_MOVE_DIR_DOWN 0
#define LCD_NO_SHIFT 0
#define LCD_SHIFT BIT(0)

#define LCD_DISP_CTL BIT(3)
#define LCD_DISP_ON BIT(2)
#define LCD_DISP_OFF 0
#define LCD_CURSOR_ON BIT(1)
#define LCD_CURSOR_OFF 0
#define LCD_BLINK_ON BIT(0)
#define LCD_BLINK_OFF 0

#define LCD_CUR_DISP_SHIFT BIT(4)
#define LCD_MOVE_CUR 0
#define LCD_SHIFT_DISP BIT(3)
#define LCD_SHIFT_RIGHT BIT(2)
#define LCD_SHIFT_LEFT 0

#define LCD_FUNC_SET BIT(5)
#define LCD_4BIT 0
#define LCD_8BIT BIT(4)
#define LCD_1LINE 0
#define LCD_2LINE BIT(3)
#define LCD_SMALL_FONT 0
#define LCD_LARGE_FONT BIT(2)

#define LCD_WRITE_DDRAM BIT(7)

// Some displays work with a very short enable pulse. For those, make LCD_ENABLE_NOPS lower
#define LCD_ENABLE_NOPS 50

#define LCD_SHORT_DELAY_US 200
#define LCD_LONG_DELAY_US 3200

#define LCD_DELAY_US_TO_TICKS(us) (20*us) // For a 20MHz clock

char lcd_memory[LCD_MEMORY];

struct
{
	lcd_type type;
	int index;
	int initStage;
} lcd_data;

bool lcd_isInit = _FALSE;

void lcd_InitCallback (void);
void lcd_PrintCallback (void);

void lcd_EnableStrobe(void);
void lcd_ChangePeriod(int new_period_us);
void lcd_SetData(int data);

void lcd_Init(lcd_type type)
{
	if (lcd_isInit == _TRUE)
		return;
	
	// Set pins as output
	
	// Low nibble
	PIN_MAKE_OUTPUT_MASK((0xF) << LCD_DATA_SHIFT(LCD_DATA_LOW), LCD_DATA_LOW_GPIO_PORT);
	
	// High nibble
	PIN_MAKE_OUTPUT_MASK((0xF) << LCD_DATA_SHIFT(LCD_DATA_HIGH), LCD_DATA_HIGH_GPIO_PORT);
	
	PIN_MAKE_OUTPUT(LCD_ENABLE);
	PIN_MAKE_OUTPUT(LCD_RS);
	
	PIN_OFF(LCD_ENABLE);
		
	// Initialize the driver's memory
	lcd_data.type = type;
	
	for (int i = 0; i < LCD_MEMORY; i++)
	{
		lcd_memory[i] = ' ';
	}
	
	lcd_data.index = 0;

	// Device initialization
	lcd_data.initStage = 0;
	
	// The LCD is implemented using the PIT
	SIM->SCGC |= SIM_SCGC_PIT_MASK;
	
	PIT->MCR = 0x00; // Enable module
	
	NVIC_EnableIRQ((LCD_PIT_CH == 0) ? PIT_CH0_IRQn : PIT_CH1_IRQn); // Enable interrupts on the NVIC
	
	lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(80000)); // Wait for about 80ms so the LCD has time to initialize

	// Wait for initialization to end
	while (lcd_isInit != _TRUE)
		;
		
	return;

}

void lcd_ISR(void)
{
	PIT->CHANNEL[LCD_PIT_CH].TFLG |= PIT_TFLG_TIF_MASK; // Clear interrupt flag
	
	if (!lcd_isInit)
	{
		lcd_InitCallback();
	}
	else
	{
		lcd_PrintCallback();
	}
}

void lcd_InitCallback (void)
{
	switch (lcd_data.initStage)
	{
		case 0:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_FUNC_SET | LCD_8BIT);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 1;
			
			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(6400)); // Wait for about 6.4ms
						
			break;
			
		case 1:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_FUNC_SET | LCD_8BIT);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 2;
			
			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(320)); // Wait for about 320us
			
			break;
		case 2:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_FUNC_SET | LCD_8BIT);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 3;

			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_SHORT_DELAY_US));
			
			break;
		case 3:
			// 8 bit mode stablished.
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_FUNC_SET | LCD_8BIT | LCD_2LINE | LCD_SMALL_FONT);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 4;

			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_SHORT_DELAY_US));
			
			break;
		case 4:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_DISP_CTL | LCD_DISP_OFF);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 5;
			
			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_SHORT_DELAY_US));
			
			break;
			
		case 5:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_CLEAR_DISP);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 6;

			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_LONG_DELAY_US));

			break;
			
		case 6:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_ENTRY_MODE | LCD_MOVE_DIR_UP | LCD_NO_SHIFT);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 7;
			
			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_SHORT_DELAY_US));			
			
			break;
			
		case 7:
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_DISP_CTL | LCD_DISP_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
			lcd_EnableStrobe();
			
			lcd_data.initStage = 8;
			
			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_SHORT_DELAY_US));
			
			break;
			
		case 8:
			lcd_ChangePeriod(LCD_DELAY_US_TO_TICKS(LCD_SHORT_DELAY_US)); // Printing period 
			
			lcd_isInit = _TRUE;
			
			break;
	}
		
}

void lcd_ChangePeriod(int new_period_us)
{	
	PIT->CHANNEL[LCD_PIT_CH].TCTRL &= ~PIT_TCTRL_TIE_MASK; // Disable interrupts
	PIT->CHANNEL[LCD_PIT_CH].TCTRL &= ~PIT_TCTRL_TEN_MASK; // Stop counting
	PIT->CHANNEL[LCD_PIT_CH].TFLG |= PIT_TFLG_TIF_MASK; // Clear interrupt flag
	
	// Load the new period
	PIT->CHANNEL[LCD_PIT_CH].LDVAL = LCD_DELAY_US_TO_TICKS(new_period_us); // Wait for about 80ms so the LCD has time to initialize
	
	PIT->CHANNEL[LCD_PIT_CH].TCTRL |= PIT_TCTRL_TEN_MASK; // Start counting
	PIT->CHANNEL[LCD_PIT_CH].TCTRL |= PIT_TCTRL_TIE_MASK; // Enable interrupts
}

void lcd_PrintCallback(void)
{
	LCD_RS_SET_DATA();
	lcd_SetData(lcd_memory[lcd_data.index++]);
	lcd_EnableStrobe();
		
	if (lcd_data.type == LCD_1602)
	{
		if (lcd_data.index == LCD_1602_COLS)
		{
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_WRITE_DDRAM | 0x40);
			lcd_EnableStrobe();
		}
		else if (lcd_data.index == 2 * LCD_1602_COLS)
		{
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_WRITE_DDRAM | 0x00);
			lcd_EnableStrobe();

			lcd_data.index = 0;
		}
	}
	else if (lcd_data.type == LCD_2004)
	{
		if (lcd_data.index == LCD_2004_COLS)
		{
			lcd_data.index = 40;
		}
		else if (lcd_data.index == 2 *LCD_2004_COLS)
		{
			lcd_data.index = 60;	
		}
		else if (lcd_data.index == 3 * LCD_2004_COLS)
		{
			lcd_data.index = 20;
		}
		else if (lcd_data.index == 4 * LCD_2004_COLS)
		{
			LCD_RS_SET_INSTR();
			lcd_SetData(LCD_WRITE_DDRAM | 0x00);
			lcd_EnableStrobe();
			
			lcd_data.index = 0;
		}
	}
}

void lcd_EnableStrobe(void)
{
	PIN_ON(LCD_ENABLE);
	
	for (int i = 0; i < LCD_ENABLE_NOPS; i++)
	{
		asm("nop");
	}
	
	PIN_OFF(LCD_ENABLE);
}

void lcd_SetData(int data)
{
	// Low nibble
	LCD_DATA_LOW_GPIO_PORT->PDOR = ((LCD_DATA_LOW_GPIO_PORT->PDOR) & ~(0xF << LCD_DATA_SHIFT(LCD_DATA_LOW))) | ((data & 0x0F) << LCD_DATA_SHIFT(LCD_DATA_LOW));
	
	// High nibble
	LCD_DATA_HIGH_GPIO_PORT->PDOR = ((LCD_DATA_HIGH_GPIO_PORT->PDOR) & ~(0xF << LCD_DATA_SHIFT(LCD_DATA_HIGH))) | (((data & 0xF0) >> 4) << LCD_DATA_SHIFT(LCD_DATA_HIGH));
}

void lcd_Print(char* string)
{
	int i = 0;	
	
	if (lcd_data.type == LCD_1602)
	{
		while ((string[i] != '\0') && (i < 32))
		{
			lcd_memory[i] = string[i];
			i++;
		}
		while (i < 32)
			lcd_memory[i++] = ' ';
	}
	else if (lcd_data.type == LCD_2004)
	{
		while ((string[i] != '\0') && (i < 80))
		{
			lcd_memory[i] = string[i];
			i++;
		}
		while (i < 80)
			lcd_memory[i++] = ' ';
	}
}

void lcd_PrintRow(char* string, int row)
{
	int i = 0;
	
	if (lcd_data.type == LCD_1602)
	{
		if (row >= 2)
			error("lcd: attempt to access an invalid row.\n");
		
		while ((string[i] != '\0') && (i < 16))
		{
			lcd_memory[i+row*16] = string[i];
			i++;
		}
		while (i < 16)
			lcd_memory[i++ + row*16] = ' ';
	}
	else if (lcd_data.type == LCD_2004)
	{
		if (row >= 4)
			error("lcd: attempt to access an invalid row.\n");
		
		while ((string[i] != '\0') && (i < 20))
		{
			lcd_memory[i+row*20] = string[i];
			i++;
		}
		while (i < 20)
			lcd_memory[i++ + row*20] = ' ';
	}
}

void lcd_ClearScreen(void)
{
	for (int i = 0; i < LCD_MEMORY; ++i)
	{
		lcd_memory[i] = ' ';
	}
}
