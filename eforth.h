#ifndef __EFORTH_SRC_EFORTH_H
#define __EFORTH_SRC_EFORTH_H
#include <Arduino.h>

typedef uint16_t    U16;
typedef uint8_t     U8;
typedef int16_t     S16;

#define	LOGICAL(f) ((f) ? 0xffff : 0)
#define LOWER(x,y) ((U16)(x)<(U16)(y))
#define	pop()      { top = *S--; }
#define	push(v)	   { *++S = top; top = (v); }
#define data       ((U16*)0)
#define cData      ((U8*)0)

#define FETCH(a)   (pgm_read_word(&code[a]))
#define AT(a)      ((a<0x900) ? data[a>>1] : FETCH(a>>1))
#define BYTE(a)    ((int)(pgm_read_byte(&cCode[a])))
#define CAT(a)     ((a<0x900) ? (int)cData[a] : BYTE(a))

#endif // __EFORTH_SRC_EFORTH_H
