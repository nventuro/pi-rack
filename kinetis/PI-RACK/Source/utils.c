#include "utils.h"

int itoa(int value, char *sp, int base)
{
    char tmp[16];// be careful with the length of the buffer
    char *tp = tmp;
    int i;
    unsigned v;

    int sign = (base == 10 && value < 0);    
    if (sign)
    {
        v = -value;
    }
    else
    {
        v = (unsigned)value;
    }

    while (v || tp == tmp)
    {
        i = v % base;
        v /= base;
        if (i < 10)
          *tp++ = i+'0';
        else
          *tp++ = i + 'a' - 10;
    }

    int len = tp - tmp;

    if (sign) 
    {
        *sp++ = '-';
        len++;
    }

    while (tp > tmp)
    {
        *sp++ = *--tp;
    }

    return len;
}

void error(char *error_msg)
{
	// error_msg can be inspected using the debugger
	
	while(1)
		;
}
