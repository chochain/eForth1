#ifndef __EFORTH_SRC_EFORTH_H
#define __EFORTH_SRC_EFORTH_H
#include <avr/pgmspace.h>

typedef uint16_t    U16;
typedef uint8_t     U8;
typedef int16_t     S16;

#define	LOGICAL(f) ((f) ? 0xffff : 0)
#define LOWER(x,y) ((U16)(x)<(U16)(y))
#define	pop()      { top = *S--; }
#define	push(v)	   { *++S = top; top = (v); }
#define ds16       ((U16*)0)
#define ds8        ((U8*)0)

#define FETCH(a)   (pgm_read_word(cs16+(a)))                     // read from flash
#define AT(a)      ((a<0x900) ? ds16[a>>1] : FETCH(a>>1))
#define BYTE(a)    (pgm_read_byte(cs8+(a)))
#define CAT(a)     ((a<0x900) ? (int)ds8[a] : (int)BYTE(a))

#endif // __EFORTH_SRC_EFORTH_H
