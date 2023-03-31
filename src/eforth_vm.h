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
 *            S                   R
 *            |                   |
 *    top -> [S0, S1, S2,..., R1, R0] <- rtop
 * @endcode
 * Note: Dr. Ting uses U8 (0~255) for wrap-around control
 */
#ifndef __EFORTH_VM_H
#define __EFORTH_VM_H
#include "eforth_core.h"

namespace EfVM {
///@name VM registers
///@{
extern IU  IP;                    ///< instruction pointer, IU is 16-bit, opcode is 8-bit
extern IU  PC;                    ///< program counter, IU is 16-bit
extern DU  *DS;                   ///< data stack pointer, Dr. Ting's stack
extern DU  *RS;                   ///< return stack pointer, Dr. Ting's rack
extern DU  top;                   ///< ALU (i.e. cached top of stack value)
extern DU  rtop;                  ///< cached loop counter on return stack
///@}
///@name MMU
///@{
extern PGM_P  _rom;               ///< ROM, Forth word stored in Arduino Flash Memory
extern U8     *_ram;              ///< RAM, memory block for user define dictionary
///@}
///@name IO Stream interface
///@{
extern Stream *io;
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
DU   DEPTH()      { return (DU)((U8*)DS - RAM(FORTH_STACK_ADDR)) >> 1; }
void PUSH(DU v)   { *++DS = top;  top  = v; }
void RPUSH(DU v)  { *--RS = rtop; rtop = v; }
#define POP()     (top  = *DS--)
#define RPOP()    (rtop = *RS++)
#define DTOP(d)   { *DS = (d)&0xffff; top = (d)>>16; }

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
int constexpr opEXIT  = 1;
int constexpr opENTER = 2;
int constexpr opDOLIT = 6;
int constexpr opEXEC  = 8;

#define DEBUG(s,v)  printf((s),(v))
void TAB() {
    LOG("\n");
    for (int i=0; i<tTAB; i++) LOG("  ");
}
void TRACE(U8 op)
{
    if (!tCNT) return;                       /// * skip if not tracing or end of program
    // indent call depth
    if (op==opENTER) {
    	TAB();
    	tTAB++;
    }
    IU pc = (op==opENTER) ? (PC | BGET(IP)) : PC;
    // display IP:PC[opcode]
    LOG_H(" ", IP-1);                        /// * mem pointer
    LOG_H(":", pc);                          /// * program counter, for indirect thread
    LOG_H("[", op); LOG("]");                /// * opcode to be executed
    // dump stack
    DU s = DEPTH() - 1;                      ///< stack depth (minus top)
    while (s-- > 0) {
        DU *vp = ((DU*)DS - s);
        DU v   = *vp;
        LOG_H("_", v);
    }
    LOG_H("_", top);
    LOG("_");
    /// special opcode handlers for DOLIT, ENTER, EXIT
    switch (op) {
    case opDOLIT: LOG_H("$", GET(IP)); LOG(" "); break;
    case opEXIT:  LOG(";");  --tTAB;             break;
    case opEXEC: pc = top; /** no break */
    case opENTER:                                 /// * display word name
    	for (--pc; (BGET(pc) & 0x7f)>0x20; pc--); /// * retract pointer to word name (ASCII range: 0x21~0x7f)
    	int len = BGET(pc++) & 0x1f;              /// Forth allows 31 char max
    	for (int i=0; i<len; i++, pc++) {
    		LOG_C((char)BGET(pc));
    	}
    	LOG(" :");
        break;
    }
}
#else
    
#define DEBUG(s,v)
#define TAB()         /* skip */
#define TRACE(op)     /* skip */
    
#endif // EXE_TRACE
///@}
    
}; // namespace EfVM

#endif // __EFORTH_VM_H
