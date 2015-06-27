#ifndef ASCII_H_
#define ASCII_H_

#include "protocol.h"

// The different parameter values are stored in a somewhat special format: they are fixed point, but the value stored 
// is actually the represented value times 100. Therefore, each value holds two decimal digits.
// For example, 205 is actually 2.05, 5 is 0.05, and 1000 is 10.00
typedef int int2d;

// Reads an ASCII value in buff[*read], until a ESCAPE_SEPARATOR. read is automatically incremented so that buff[*read] is
// the first character after ESCAPE_SEPARATOR.
int2d asciiToInt2d(char *buff, int *read);

// Stores the ESCAPE_SEPARATOR terminated string that starts in src[*read] in dst, doing the corresponding substitutions
// (according to protocol.h). read is automatically incremented so that buff[*read] is the first character after ESCAPE_SEPARATOR.
// If the string is longer than max_length, only max_length characters are written.
// Returns the number of written characters
int asciiToString(char *src, int *read, char *dst, int max_length);

// Writes a string representation of value in buff, using only max_chars digits (including the decimal point, if necessary). An extra
// character is added at the beginning, which is either a '-' if value is negative, or a whitespace if it's not.
// Returns the number of written characters (which is always max_chars + 1)
int int2dToASCII(char *buff, int2d value, int max_chars);

#endif /* ASCII_H_ */
