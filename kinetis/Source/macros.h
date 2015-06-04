#ifndef MACROS_H_
#define MACROS_H_

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

#endif /* MACROS_H_ */
