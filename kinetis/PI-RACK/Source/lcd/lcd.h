#ifndef _LCD_H
#define _LCD_H

#include "utils.h"

typedef enum
{
	LCD_1602,	// 16 characters, 2 rows
	LCD_2004	// 20 characters, 4 rows
} lcd_type;

#define LCD_1602_COLS 16
#define LCD_1602_ROWS 2

#define LCD_2004_COLS 20
#define LCD_2004_ROWS 4

#define LCD_MEMORY (LCD_2004_ROWS * LCD_2004_COLS)
extern char lcd_memory[LCD_MEMORY];
// After calling lcd_Init(), the LCD's screen will match what's on lcd_memory. A new row commences every (lcd_type)_COLS characters.
// If the memory can hold more characters than the LCD can (because it's of a smaller size), only the first characters are used.

#define LCD_PIT_CH 1 // The LCD on the Kinetis uses this PIT (periodic interrupt timer) channel for internal timing

// The device's pins should be connected to the MCU pins described below

// The data pins are divided in two nibbles to aid with the mapping to physical pins. These nibbles must be mapped to 4 consecutive
// GPIO pins.

// LCD_DATA_LOW is PTG0 to PTG3
#define LCD_DATA_LOW_PORT G
#define LCD_DATA_LOW_GPIO_PORT GPIOB
#define LCD_DATA_LOW_START 0

// LCD_DATA_HIGH is PTE0 to PTE3
#define LCD_DATA_HIGH_PORT E
#define LCD_DATA_HIGH_GPIO_PORT GPIOB
#define LCD_DATA_HIGH_START 0

#define LCD_ENABLE GPIO_PTF0  // The enable (clock) pin.
#define LCD_RS GPIO_PTF1 // The Register Select pin.
// The Read/Write pin should be connected to ground.

void lcd_Init(lcd_type type);
// Initializes the LCD module. type determines the number of rows and columns the module expects the device to have. A wrong value on type will 
// cause faulty behavior. After lcd_Init is called, type cannot be changed.

// While this is set to _TRUE, the LCD module won't update the screen contents
extern bool updatingScreen;

void lcd_Print(char* string);
// Prints a string on the device, starting on the first row and first column. 
// If the string is not null-terminated, or too long, the module will only print as many characters as the device is capable of displaying.
// If the string has less characters than the device can display, the remaining characters are left empty.
// This deletes any message previously printed on the device.
// Calling lcd_Print is the same as copying string in lcd_memory, filling the rest of it with whitespaces.

void lcd_PrintRow(char* string, int row);
// Prints a string on the device, starting on the first column of row. The first row is row 0.
// If the string is not null-terminated, or too long, the module will only print as many characters as the device is capable of displaying.
// Only the contents of the selected row will be modified.
// If the string has less characters than the device can display, the remaining characters are left empty.
// This deletes any message previously printed on the device (on that row).
// Calling lcd_Print is the same as copying string in lcd_memory, on the positions corresponding to the
// selected row, filling the rest of it with whitespaces.

void lcd_ClearScreen(void);
// Clears the screen contents. This actually clears lcd_memory, and can be called before writing new content so that no stray characters
// are mistakenly drawn.

void lcd_ISR(void);
// Interrupt function. To be registered on the vector table under the PIT[LCD_PIT_CH] interrupt

#endif
