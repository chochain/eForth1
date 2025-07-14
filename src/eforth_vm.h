/**
 * @file
 * @brief eForth Virtual Machine module header
 *
 * ### eForth Memory map
 *
 * @code
 *     0x0000-0x1fff ROM (8K Flash memory)
 *     0x2000-0x24ff RAM (2K dynamic memory)
 *         0x2000-0x201f User Variables
 *         0x2020-0x23ff User Dictionary
 *         0x2400-0x247f Data/Return Stacks
 *         0x2480-0x24ff TIB (Terminal Input Buffer)
 *         0x2500        heap
 * @endcode
 *
 * ### Data and Return Stack
 *
 * @code
 *          S                   R
 *          |                   |
 *    T -> [S0, S1, S2,..., R1, R0] <- I
 * @endcode
 * Note: Dr. Ting uses U8 (0~255) for wrap-around control
 */
#ifndef __EFORTH_VM_H
#define __EFORTH_VM_H
#include "eforth_core.h"

namespace EfVM {
///@name MMU & IO interfaces
///@{
extern PGM_P    _rom;             ///< ROM, Forth word stored in Arduino Flash Memory
extern U8       *_ram;            ///< RAM, memory block for user define dictionary
extern StreamIO *io;              ///< Stream or UART IO interface
///@}
#define RAM_FLAG       0xe000     /**< RAM ranger      (0x2000~0x7fff) */
#define IDX_MASK       0x07ff     /**< RAM index mask  (0x0000~0x07ff) */
#define IRET_FLAG      0x8000     /**< interrupt return flag           */
#define BOOL(f)        ((f) ? TRUE : FALSE)
#define RAM(i)         &_ram[(i) - FORTH_RAM_ADDR]
///
/// byte (8-bit) fetch from either RAM or ROM depends on filtered range
///
U8 BGET(U16 d) {
    return (U8)((d&RAM_FLAG) ? _ram[d&IDX_MASK] : pgm_read_byte(_rom+d));
}
///
/// word (16-bit) fetch from either RAM or ROM depends on filtered range
///
U16 GET(U16 d) {
    return (d&RAM_FLAG)
        ? ((U16)_ram[d&IDX_MASK]<<8) | _ram[(d+1)&IDX_MASK]
        : ((U16)pgm_read_byte(_rom+d)<<8) | pgm_read_byte(_rom+d+1);
}
#define BSET(d, c)     (_ram[(d)&IDX_MASK]=(U8)(c))
void SET(U16 d, U16 v) {
	BSET(d,   v>>8);
	BSET(d+1, v&0xff);
}
#define S2D(h, l) (((S32)(h)<<16) | ((l)&0xffff))
///
/// push a value onto stack top
///
#define DEPTH()  ((DU)((U8*)S - RAM(FORTH_STACK_ADDR)) >> 1)
#define PUSH(v)  { *++S = T; T = (v); }
#define RPUSH(v) { *--R = I; I = (v); }
#define POP()    (T = *S--)
#define RPOP()   (I = *R++)
#define DTOP(d)  { *S = (d)&0xffff; T = (d)>>16; }

///
///@name Tracing
///@{
#if EXE_TRACE
int tCNT;           ///< tracing depth counters
int tTAB;           ///< tracing indentation counter
///@}
///
///@name Tracing Functions
///@{
int constexpr opEXIT  = 1;    /// cross-check with OPCODE enum in eforth_opcode.h
int constexpr opENTER = 2;
int constexpr opBYTE  = 6;
int constexpr opDOLIT = 7;
int constexpr opEXEC  = 9;

#define DEBUG(s,v)  if (tCNT) printf((s),(v))
#define TAB()       if (tCNT) {           \
    LOG("\n");                            \
    for (int i=0; i<tTAB; i++) LOG("  "); \
}
void TRACE(U8 op, U16 ip, U16 top, S16 s)
{
    if (!tCNT) return;                       /// * skip if not tracing or end of program
    // indent call depth
    if (op==opENTER) {
    	TAB();
    	tTAB++;
    }
    // display IP:W[opcode]
    U16 w = ip - 1;
    LOG_H(" ", w);                           /// * mem pointer
	LOG_H("[", op); LOG("]");                /// * opcode to be executed
    // dump stack
    for (int i=0; i<s; i++) {
        LOG_H("_", *((DU*)RAM(FORTH_STACK_ADDR) + (i+1)));
    }
    LOG_H("_", top);
    LOG("_");
    /// special opcode handlers for DOLIT, ENTER, EXIT
    switch (op) {
    case opBYTE:  LOG_H("$", BGET(ip)); LOG(" "); break;
    case opDOLIT: LOG_H("$", GET(ip));  LOG(" "); break;
    case opEXIT:  LOG(";");  --tTAB;              break;
    case opEXEC: w = top; /** no break */
    case opENTER:                                 /// * display word name
    	for (--w; (BGET(w) & 0x7f)>0x20; w--);    /// * retract pointer to word name (ASCII range: 0x21~0x7f)
    	int len = BGET(w++) & 0x1f;               /// Forth allows 31 char max
    	for (int i=0; i<len; i++, w++) {
    		LOG_C((char)BGET(w));
    	}
    	LOG(" :");
        break;
    }
}
#else
    
#define DEBUG(s,v)
#define TAB()                /* skip */
#define TRACE(op, ip, t, s)  /* skip */
    
#endif // EXE_TRACE
///@}
    
}; // namespace EfVM

#endif // __EFORTH_VM_H
