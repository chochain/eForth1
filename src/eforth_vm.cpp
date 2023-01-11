/**
 * @file
 * @brief eForth Virtual Machine module
 * Note: indirect threading model - computed label jumping
 *       replaced EfVMSub module - vtable subroutine calling
 *
 * ####eForth Memory map
 *
 * @code
 *     0x0000-0x1fff ROM (8K Flash memory)
 *     0x2000-0x27ff RAM (2K dynamic memory)
 *         0x2000-0x201f User Variables
 *         0x2020-0x23ff User Dictionary
 *         0x2400-0x24bf Return/Data Stack
 *         0x24c0-0x253f TIB (Terminal Input Buffer)
 * @endcode
 *
 * ####Data and Return Stack
 *
 * @code
 *            S                   R
 *            |                   |
 *    top -> [S0, S1, S2,..., R1, R0]
 * @endcode
 * Note: Dr. Ting uses U8 (0~255) for wrap-around control
 */
#include "eforth_core.h"

namespace EfVM {

static Stream *io;
///
///@name Control
///@{
IU  PC;                         ///< PC (program counter, IU is 16-bit)
IU  IP;                         ///< IP (instruction pointer, IU is 16-bit)
U8  R;                          ///< return stack index (0-255)
U8  S;                          ///< data stack index (0-255)
S16 top;                        ///< ALU (i.e. cached top of stack value)
///@}
///
///@name Storage
///@{
PGM_P cRom;                     ///< ROM, Forth word stored in Arduino Flash Memory
U8    *cData;                   ///< RAM, memory block for user define dictionary
S16   *cStack;                  ///< pointer to stack/rack memory block
///@}
#define RAM_FLAG       0xe000   /**< RAM ranger      (0x2000~0xffff) */
#define OFF_MASK       0x07ff   /**< RAM offset mask (0x0000~0x07ff) */
#define BOOL(f)        ((f) ? TRUE : FALSE)
///
/// byte (8-bit) fetch from either RAM or ROM depends on filtered range
///
U8   BGET(U16 d)       {
    return (U8)((d&RAM_FLAG) ? cData[d&OFF_MASK] : pgm_read_byte(cRom+d));
}
///
/// word (16-bit) fetch from either RAM or ROM depends on filtered range
///
U16  GET(U16 d)        {
    return (d&RAM_FLAG)
        ? *((U16*)&cData[d&OFF_MASK])
        : (U16)pgm_read_byte(cRom+d) + ((U16)pgm_read_byte(cRom+d+1)<<8);
}
#define BSET(d, c)     (cData[(d)&OFF_MASK]=(U8)(c))
#define SET(d, v)      (*((S16*)&cData[(d)&OFF_MASK])=(v))
#define SS(s)          (cStack[s])
#define RS(r)          (cStack[RS_TOP - (r)])
#define RS_TOP         (FORTH_STACK_SZ>>1)
///
/// push a value onto stack top
///
void PUSH(S16 v)       { SS(++S) = top; top = v; }
void RPUSH(S16 v)      { RS(++R) = v; }
#define POP()          (top=SS(S ? S-- : S))
#define RPOP()         (RS(R ? R-- : R))
#define DTOP(d)        { SS(S) = (d)&0xffff; top = (d)>>16; }
///
/// update program counter (ready to fetch), advance instruction pointer
///
void NEXT() { PC=GET(IP); IP+=sizeof(IU); }
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
void _trc_on()  { tCNT++;               NEXT(); }
void _trc_off() { tCNT -= tCNT ? 1 : 0; NEXT(); }

#define TRACE(s, v)   if (tCNT) { LOG_H(s, v); }
#define TRACE_COLON() if (tCNT) {              \
    LOG("\n");                                 \
    for (int i=0; i<tTAB; i++) LOG("  ");      \
    tTAB++;                                    \
    LOG(":");                                  \
}
#define TRACE_EXIT()  if (tCNT) {              \
    LOG(" ;");                                 \
    tTAB--;                                    \
}
///
/// display the 'word' to be executed
///
void TRACE_WORD()
{
    if (!tCNT) return;
    if (!PC || BGET(PC)==opEXIT) return;
    IU pc = PC-1;
    for (; (BGET(pc) & 0x7f)>0x20; pc--);  // retract pointer to word name (ASCII range: 0x21~0x7f)

    for (int s=(S>=3 ? S-3 : 0), s0=s; s<S; s++) {
        LOG_H(s==s0 ? " " : "_", SS(s+1));
    }
    LOG_H(S==0 ? " " : "_", top);
    LOG("_");
    int len = BGET(pc++) & 0x1f;          // Forth allows 31 char max
    for (int i=0; i<len; i++, pc++) {
        LOG_C((char)BGET(pc));
    }
}
#else  // !EXE_TRACE
#define TRACE(s, v)
#define TRACE_COLON()
#define TRACE_EXIT()
#define TRACE_WORD()
#endif // EXE_TRACE
///@}
//
// Forth Virtual Machine primitive functions
//
/// virtual machine initializer
///
void _init() {
    R = S = PC = IP = top = 0;  ///> setup control variables
#if EXE_TRACE
    tCNT = 1; tTAB = 0;         ///> setup tracing variables
#endif  // EXE_TRACE
    
    /// FORTH_UVAR_ADDR;
    ///   'TIB console input buffer pointer
    ///   BASE current radix for numeric ops
    ///   CP,  top of dictionary, same as HERE
    ///   CONTEXT name field of last word
    ///   LAST, same as CONTEXT
    ///   'EVAL eval mode (interpreter or compiler)
    ///   'ABORT exception rescue handler (QUIT)
    ///   tmp storage (alternative to return stack)
    IU p = FORTH_UVAR_ADDR;    ///> setup Forth user variables
    SET(p,   FORTH_TIB_ADDR);  /// * set 'TIB pointer
    SET(p+2, 10);              /// * set BASE to 10
    SET(p+4, FORTH_DIC_ADDR);  /// * top of dictionary
    ///
    /// display init prompt
    ///
    LOG("\n"); LOG(APP_NAME); LOG(" "); LOG(MAJOR_VERSION);
}
    
void _txsto()               /// (c -- ) send a char to console
{
#if !EXE_TRACE
    LOG_C((char)top);
#else  // !EXE_TRACE
    switch (top) {
    case 0xa: LOG("<LF>");  break;
    case 0xd: LOG("<CR>");  break;
    case 0x8: LOG("<TAB>"); break;
    default:
        if (tCNT) { LOG("<"); LOG_C((char)top); LOG(">"); }
        else      LOG_C((char)top);
    }
#endif // !EXE_TRACE
    POP();
    NEXT();
}
///@}
void _execu()               /// (a -- ) take execution address from data stack and execute the token
{
    PC = (IU)top;           ///> fetch program counter
    POP();
}
void _ummod()               /// (udl udh u -- ur uq) unsigned divide of a double by single
{
    U32 d = (U32)top;       ///> CC: auto variable uses C stack 
    U32 m = ((U32)SS(S)<<16) + (U16)SS(S-1);
    POP();
    SS(S) = (S16)(m % d);   ///> remainder
    top   = (S16)(m / d);   ///> quotient
    NEXT();
}
void _delay()               /// (n -- ) delay n milli-second
{
    U32 t  = millis() + top;///> calculate break time
    POP();
    while (millis()<t) {    ///> loop until break time reached
        ef_yield();         ///> or, run hardware tasks while waiting
    }
    NEXT();
}
///@}
}; // namespace EfVM
///
/// eForth virtual machine initialization
///
///> internal (user) variables
///> *  'TIB    = FORTH_TIB_ADDR (pointer to input buffer)
///> *  BASE    = 0x10           (numerical base 0xa for decimal, 0x10 for hex)
///> *  CP      = here           (pointer to top of dictionary, first memory location to add new word)
///> *  CONTEXT = last           (pointer to name field of the most recently defined word in dictionary)
///> *  LAST    = last           (pointer to name field of last word in dictionary)
///> *  'EVAL   = INTER          ($COMPILE for compiler or $INTERPRET for interpreter)
///> *  ABORT   = QUIT           (pointer to error handler, QUIT is the main loop)
///> *  tmp     = 0              (scratch pad)
///
using namespace EfVM;

void vm_init(PGM_P rom, U8 *cdata, void *io_stream) {
    io     = (Stream *)io_stream;
    cRom   = rom;
    cData  = cdata;
    cStack = (S16*)&cdata[FORTH_STACK_ADDR - FORTH_RAM_ADDR];
    
    _init();                   /// * resetting user variables
}
///
/// eForth virtual machine outer interpreter (single-step) execution unit
/// @return
///   0 - exit
/// Note:
///   vm_outer      - computed label (25% faster)
///
int vm_outer() {
    static void* vt[] = {      ///< computed label lookup table
    &&L_opNOP,        // 0
    &&L_opBYE,
    &&L_opQRX,
    &&L_opTXSTO,
    &&L_opDOCON,
    &&L_opDOLIT,
    &&L_opDOVAR,
    &&L_opENTER,
    &&L_opEXIT,
    &&L_opEXECU,
    &&L_opDONEXT,     // 10
    &&L_opQBRAN,
    &&L_opBRAN,
    &&L_opSTORE,
    &&L_opPSTOR,
    &&L_opAT,
    &&L_opCSTOR,
    &&L_opCAT,
    &&L_opRFROM,
    &&L_opRAT,
    &&L_opTOR,
    &&L_opDROP,       // 20
    &&L_opDUP,
    &&L_opSWAP,
    &&L_opOVER,
    &&L_opROT,
    &&L_opPICK,
    &&L_opAND,
    &&L_opOR,
    &&L_opXOR,
    &&L_opINV,        // 30
    &&L_opLSH,
    &&L_opRSH,
    &&L_opADD,
    &&L_opSUB,
    &&L_opMUL,
    &&L_opDIV,
    &&L_opMOD,
    &&L_opNEG,
    &&L_opGT,
    &&L_opEQ,         // 40
    &&L_opLT,
    &&L_opZGT,
    &&L_opZEQ,
    &&L_opZLT,
    &&L_opONEP,
    &&L_opONEM,
    &&L_opQDUP,
    &&L_opDEPTH,
    &&L_opULESS,
    &&L_opUMMOD,      // 50
    &&L_opUMSTAR,
    &&L_opMSTAR,
    &&L_opDNEG,
    &&L_opDADD,
    &&L_opDSUB,
    &&L_opDELAY,
    &&L_opCLK,
    &&L_opPIN,
    &&L_opMAP,
    &&L_opIN,         // 60
    &&L_opOUT,
    &&L_opAIN,
    &&L_opPWM
    };

#define _OP(op, fn)   L_##op: { fn(); continue; }
#define _OX(op, code) L_##op: { code; goto vm_step_next; }
    
    do {
        TRACE_WORD();                   /// * tracing stack and word name

        U8 op = BGET(PC);               ///> fetch next opcode
        goto *vt[op];                   ///> jump to computed label
        ///
        /// traditionally, this is the part done in Assembly
        ///
        _OX(opNOP,   {});
        _OP(opBYE,   _init);
        /// 
        /// @name Console IO
        /// @{
        _OX(opQRX,
            PUSH(ef_getchar());         ///> yield to user task until console input available
            if (top) PUSH(TRUE));
        _OP(opTXSTO, _txsto);
        /// @}
        /// @name Built-in ops
        /// @{
        _OX(opDOCON,
            ++PC;                       ///> skip opDOCON opcode
            PUSH(GET(PC)));             ///> push cell value onto stack
        _OX(opDOLIT,
            TRACE(" ", GET(IP));        ///> fetch literal from data
            PUSH(GET(IP));              ///> push onto data stack
            IP += CELLSZ);              ///> skip to next instruction
        _OX(opDOVAR, ++PC; PUSH(PC));
        /// @}
        /// @name Branching ops
        /// @{
        _OX(opENTER,
        	ef_yield();
            TRACE_COLON();
            RPUSH(IP);                  ///> keep return address
            IP = ++PC);                 ///> skip opcode opENTER, advance to next instruction
        _OX(opEXIT,
            TRACE_EXIT();
            IP = RPOP());               ///> pop return address
        _OP(opEXECU, _execu);
        _OX(opDONEXT,
        	if (RS(R) > 0) {        	///> check if loop counter > 0
        		RS(R)--;            	///>> decrement loop counter
        		IP = GET(IP);       	///>> branch back to FOR
        	}
        	else {                  	///> or,
        		IP += CELLSZ;       	///>> skip to next instruction
        		RPOP();             	///>> pop off return stack
            	ef_yield();             ///> give system task some cycles
        	});
        _OX(opQBRAN,
            if (top) IP += CELLSZ;      ///> next instruction, or
            else     IP = GET(IP);      ///> fetch branching target address
            POP());
        _OX(opBRAN,  IP = GET(IP));     ///> fetch branching target address
        /// @}
        /// @name Memory Storage ops
        /// @{
        _OX(opSTORE,
            SET(top, SS(S--));
            POP());
        _OX(opPSTOR,
            SET(top, GET(top)+SS(S--));
            POP());
        _OX(opAT,    top = (S16)GET(top));
        _OX(opCSTOR,
            BSET(top, SS(S--));
            POP());
        _OX(opCAT,   top = (S16)BGET(top));
        _OX(opRFROM, PUSH(RPOP()));
        _OX(opRAT,   PUSH(RS(R)));
        _OX(opTOR,
            RPUSH(top);
            POP());
        /// @{
        /// @name Stack ops
        /// @}
        _OX(opDROP,  POP());
        _OX(opDUP,   SS(++S)=top);
        _OX(opSWAP,
            S16 tmp = top;
            top     = SS(S);
            SS(S)   = tmp);
        _OX(opOVER,  PUSH(SS(S)));      ///> push w1
        _OX(opROT,
            S16 tmp = SS(S-1);
            SS(S-1) = SS(S);
            SS(S)   = top;
            top     = tmp);
        _OX(opPICK,  top = SS(S - (U8)top));
        /// @}
        /// @name ALU ops
        /// @{
        _OX(opAND,   top &= SS(S--));
        _OX(opOR,    top |= SS(S--));
        _OX(opXOR,   top ^= SS(S--));
        _OX(opINV,   top = -top - 1);
        _OX(opLSH,   top = SS(S--) << top);
        _OX(opRSH,   top = SS(S--) >> top);
        _OX(opADD,   top += SS(S--));
        _OX(opSUB,   top = SS(S--) - top);
        _OX(opMUL,   top *= SS(S--));
        _OX(opDIV,   top = (top) ? SS(S--) / top : (S--, 0));
        _OX(opMOD,   top = (top) ? SS(S--) % top : SS(S--));
        _OX(opNEG,   top = 0 - top);
        /// @}
        /// @name Logic ops
        /// @{
        _OX(opGT,    top = BOOL(SS(S--) > top));
        _OX(opEQ,    top = BOOL(SS(S--)==top));
        _OX(opLT,    top = BOOL(SS(S--) < top));
        _OX(opZGT,   top = BOOL(top > 0));
        _OX(opZEQ,   top = BOOL(top == 0));
        _OX(opZLT,   top = BOOL(top < 0));
        /// @}
        /// @name Misc. ops
        /// @{
        _OX(opONEP,  top++);
        _OX(opONEM,  top--);
        _OX(opQDUP,  if (top) SS(++S) = top);
        _OX(opDEPTH, PUSH(S));
        _OX(opULESS, top = BOOL((U16)(SS(S--)) < (U16)top));
        _OP(opUMMOD, _ummod);
        _OX(opUMSTAR,               /// (u1 u2 -- ud) unsigned multiply return double product
            U32 u = (U32)SS(S) * top;
            SS(S) = (U16)(u & 0xffff);
            top   = (U16)(u >> 16));
        _OX(opMSTAR,                /// (n1 n2 -- d) signed multiply, return double product
            S32 d = (S32)SS(S) * top;
            DTOP(d));
        /// @}
        /// @name Double precision ops
        /// @{
        _OX(opDNEG,                 /// (d -- -d) two's complemente of top double
            S32 d = ((S32)top<<16) | (SS(S) & 0xffff);
            DTOP(-d));
        _OX(opDADD,                 /// (d1 d2 -- d1+d2) add two double precision numbers
            S32 d0 = ((S32)top<<16)     | (SS(S)&0xffff);
            S32 d1 = ((S32)SS(S-1)<<16) | (SS(S-2)&0xffff);
            S -= 2; DTOP(d1 + d0));
        _OX(opDSUB,                 /// (d1 d2 -- d1-d2) subtract d2 from d1
            S32 d0 = ((S32)top<<16)     | (SS(S)&0xffff);
            S32 d1 = ((S32)SS(S-1)<<16) | (SS(S-2)&0xffff);
            S -= 2; DTOP(d1 - d0));
        /// @}
        /// @name Arduino specific ops
        /// @{
        _OP(opDELAY, _delay);
        _OX(opCLK,                  /// fetch system clock in double precision
            S += 2; DTOP(millis()));
        _OX(opPIN,
            pinMode(top, SS(S) ? OUTPUT : INPUT);
            POP(); POP());
        _OX(opMAP,
            U16 tmp = map(top, SS(S-3), SS(S-2), SS(S-1), SS(S));
            S -= 4;
            top = tmp);
        _OX(opIN,  PUSH(digitalRead(POP())));
        _OX(opOUT,
            digitalWrite(top, SS(S));
            POP(); POP());
        _OX(opAIN, PUSH(analogRead(POP())));
        _OX(opPWM,
            analogWrite(top, SS(S));
            POP(); POP());

    vm_step_next:
        PC = GET(IP);               ///> fetch next program counter (branch)
        IP += sizeof(IU);           ///> advance to next instruction
    } while (PC);

    return (int)PC;
}
