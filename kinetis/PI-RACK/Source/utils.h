#ifndef UTILS_H_
#define UTILS_H_

#ifndef	s8
#define s8		char
#endif
#ifndef	u8
#define u8		unsigned char
#endif
#ifndef	s16
#define s16		short
#endif
#ifndef	u16
#define u16		unsigned short
#endif
#ifndef	s32
#define s32		long
#endif
#ifndef	u32
#define u32		unsigned long
#endif
#ifndef	sint
#define sint		int
#endif
#ifndef	uint
#define uint		unsigned int
#endif
#ifndef	s64
#define s64		long long
#endif
#ifndef	u64
#define u64		unsigned long long
#endif

#define GLUE(a,b) GLUE_AGAIN (a,b)
#define GLUE_AGAIN(a,b) a##b

#define GLUE2(a,b,c) GLUE_AGAIN2(a,b,c)
#define GLUE_AGAIN2(a,b,c) a##b##c

#define IS_EVEN(x) ((x % 2 == 0 ? 1 : 0))
#define IS_ODD(x) (!IS_EVEN(x))

#define BIT(n) (1<<(n))

typedef int bool;
#define _TRUE 1
#define _FALSE 0

#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define PIN_MASK(pin) (pin < GPIO_PTE0 ? (1 << pin) : (1 << (pin - GPIO_PTE0)))
#define PUE_MASK(pin) PIN_MASK(pin)

#define GPIO_PORT(pin) (pin < GPIO_PTE0 ? GPIOA : GPIOB)

#define PIN_MAKE_OUTPUT(pin) PIN_MAKE_OUTPUT_PORT(pin, GPIO_PORT(pin))
#define PIN_MAKE_INPUT(pin)  PIN_MAKE_INPUT_PORT(pin, GPIO_PORT(pin))

#define PIN_MAKE_OUTPUT_PORT(pin, port) (PIN_MAKE_OUTPUT_MASK(PIN_MASK(pin), port))
#define PIN_MAKE_INPUT_PORT(pin, port)  (PIN_MAKE_INPUT_MASK(PIN_MASK(pin), port))

#define PIN_MAKE_OUTPUT_MASK(pin_mask, port) (port->PDDR |= pin_mask)
#define PIN_MAKE_INPUT_MASK(pin_mask, port)  (port->PIDR &= ~(pin_mask))

#define PIN_ENABLE_PULLUP(pin) do { if (pin < GPIO_PTE0) { PORT->PUEL |= (1 << pin); } else { PORT->PUEH |= (1 << (pin - GPIO_PTE0)); } } while (0)

#define PIN_ON(pin) (PIN_ON_PORT(pin, GPIO_PORT(pin)))
#define PIN_OFF(pin) (PIN_OFF_PORT(pin, GPIO_PORT(pin)))

#define PIN_ON_PORT(pin, port) (port->PSOR = PIN_MASK(pin))
#define PIN_OFF_PORT(pin, port) (port->PCOR = PIN_MASK(pin))

#define IS_PIN_ON(pin) (IS_PIN_ON_PORT(pin, GPIO_PORT(pin)))
#define IS_PIN_OFF(pin) (IS_PIN_OFF_PORT(pin, GPIO_PORT(pin)))

#define IS_PIN_ON_PORT(pin, port) (!IS_PIN_OFF_PORT(pin, port))
#define IS_PIN_OFF_PORT(pin, port) ((port->PDIR & PIN_MASK(pin)) == 0)

int itoa(int value, char *sp, int base);

void error(char *error_msg); // Halts execution

#endif /* UTILS_H_ */
