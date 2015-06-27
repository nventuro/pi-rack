#include "ascii.h"
#include "utils.h"

int intPow(int base, int exp);

int2d asciiToInt2d(char *buff, int *read)
{
	int2d result = 0;
	
	bool is_neg = _FALSE;
	bool seen_point = _FALSE;
	int decimals_seen = 0;
	
	char val; 
	while ((val = buff[(*read)++]) != ESCAPE_SEPARATOR)
	{
		if (val == '.')
		{
			seen_point = _TRUE;
		}
		else if (val == '-')
		{
			is_neg = _TRUE;
		}
		else
		{
			result = result * 10 + val - '0';
			if (seen_point)
			{
				decimals_seen++;
			}
		}
	}
	
	if (decimals_seen < 2)
	{
		result *= intPow(10, 2 - decimals_seen);
	}
	else if (decimals_seen > 2)
	{
		result /= intPow(10, decimals_seen - 2);
	}
	
	if (is_neg)
	{
		result *= -1;
	}
	
	return result;
}

int asciiToString(char *src, int *read, char *dst, int max_length)
{
	int dst_idx = 0;
	while ((dst_idx < max_length) && (src[*read] != ESCAPE_SEPARATOR))
	{
		char val = src[(*read)++];
		if (val == ESCAPE_WHITESPACE)
		{
			val = ' ';
		}
		dst[dst_idx++] = val;
	}

	// Read up to one character after the separator
	while (src[(*read)++] != ESCAPE_SEPARATOR)
		;
	
	return dst_idx;
}

int int2dToASCII(char *buff, int2d value, int max_chars)
{
	if (value < 0)
	{
		value *= -1;
		(*buff) = '-';
	}
	else
	{
		(*buff) = ' ';
	}
	
	buff++;
	
	// Position where the decimal point would be (counting from the right, last character is position 1),
	// if the number was constrained to max_chars digits.
	// A dec_pos of 0 means the decimal point isn't displayed (it's right after the last digit).
	// A dec_pos of 1 means the decimal point is the last digit (in which case we use a length of max_chars - 1
	// and don't display the decimal point).
	// A dec_pos between 2 and max_chars - 1 means the decimal point is displayed.
	// A dec_pos >= max_chars means the number cannot be correctly displayed (it's too small).
	// A dec_pos < 0 means the number cannot be correctly displayed (it's too large).
	int dec_pos = -1;
	
	int upper_range;
	int lower_range;
	
	do
	{
		dec_pos++;
		
		upper_range = intPow(10, max_chars - dec_pos) - 1;
		lower_range = intPow(10, max_chars - 1 - dec_pos);
		
		if ((value / 100) > upper_range) // If value is too large
		{
			break;
		}
		
		if (dec_pos == (max_chars - 1)) // Value is either too small, or as small as possible (dec_pos being 
										// equal to maa_chars means there's no leading zero before the decimal point)
		{
			break;
		}
	}
	while (!(((value / 100) <= upper_range) && ((value / 100) >= lower_range)));
	
	int write_pos = 0;
	if (dec_pos == 1)
	{
		buff[write_pos++] = ' '; // If dec_pos is 1, we lose one of the digits (else there'd be no digit after the decimal point)
	}
	
	int written_digits = 0;
	while (write_pos < max_chars)
	{
		if (((write_pos) == (max_chars - dec_pos)) && (dec_pos != 1)) // We skip the decimal point position rule for dec_pos = 1 (otherwise the point would be at the end)
		{
			buff[write_pos] = '.';
		}
		else
		{
			buff[write_pos] = (value / intPow(10, max_chars + 2 - 1 - dec_pos - written_digits) % 10) + '0';
			
			written_digits++;
		}
		     
		write_pos++;
	}
	
	return max_chars + 1; // This always writes the same number of characters (the +1 is because of the whitespace or '-' character at the start)
}

int intPow(int base, int exp)
{
	int result = 1;

	while ((exp--) > 0)
	{
		result *= base;
	}

    return result;
}
