/**
 * @file eforth_vm.cpp
 * @brief eForth Virtual Machine module
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
///
/// store a double on data stack top
///
void DTOP(S32 d) {
    SS(S) = d&0xffff;
    top   = d>>16;
}
///
/// update program counter (ready to fetch), advance instruction pointer
///
void NEXT() { PC=GET(IP); IP+=sizeof(IU); }
///
///@name Tracing
///@{
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
        if (s==s0) { LOG_H(" ", SS(s+1)); } else { LOG_H("_", SS(s+1)); }
    }
    if (S==0) { LOG_H(" ", top); } else { LOG_H("_", top); }
    LOG("_");
    int len = BGET(pc++) & 0x1f;          // Forth allows 31 char max
    for (int i=0; i<len; i++, pc++) {
        LOG_C((char)BGET(pc));
    }
}
///@}
//
// Forth Virtual Machine primitive functions
//
/// virtual machine initializer
///
void _init() {
    R = S = PC = IP = top = 0;  ///> setup control variables
    tCNT = tTAB = 0;            ///> setup tracing variables
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
    SET(p+2, 0x10);            /// * set BASE to 10
    SET(p+4, FORTH_DIC_ADDR);  /// * top of dictionary
    
#if EXE_TRACE
    tCNT=1;                 ///> optionally enable tracing
#endif // EXE_TRACE
    ///
    /// display init prompt
    ///
    LOG("\n"); LOG(APP_NAME); LOG(" "); LOG(MAJOR_VERSION);
}
void _nop() { NEXT(); }     ///< ( -- ) nop, as macro terminator
///
///@name Console IO
///@{
void _qrx() {               ///  ( -- c t|f) read a char from terminal input device
    PUSH(ef_getchar());     ///> yield to user task until console input available
    if (top) PUSH(TRUE);
    NEXT();
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
///
///@name Variables/Constant/Literal Ops
///@{
void _dovar()               /// ( -- a) return address of a variable
{
    ++PC;                   ///> skip opDOVAR opcode
    PUSH(PC);               ///> push variable address onto stack
    NEXT();
}
void _docon()               /// ( -- n) push next token onto data stack as constant
{
    ++PC;                   ///> skip opDOCON opcode
    PUSH(GET(PC));          ///> push cell value onto stack
    NEXT();
}
void _dolit()               /// ( -- w) push next token as an integer literal
{
    TRACE(" ", GET(IP));    ///> fetch literal from data
    PUSH(GET(IP));          ///> push onto data stack
    IP += CELLSZ;           ///> skip to next instruction
    NEXT();
}
///@}
///@name Call/Branching Ops
///@{
void _enter()               /// ( -- ) push instruction pointer onto return stack and pop, aka DOLIST by Dr. Ting
{
    TRACE_COLON();
    RPUSH(IP);              ///> keep return address
    IP = ++PC;              ///> skip opcode opENTER, advance to next instruction
    NEXT();
}
void _exit()               /// ( -- ) terminate all token lists in colon words
{
    TRACE_EXIT();
    IP = RPOP();            ///> pop return address
    NEXT();
}
void _execu()               /// (a -- ) take execution address from data stack and execute the token
{
    PC = (IU)top;           ///> fetch program counter
    POP();
}
void _donext()              /// ( -- ) terminate a FOR-NEXT loop
{
    IU i = RS(R);           ///> loop counter
    if (i) {                ///> check if loop counter > 0
        RS(R) = (S16)(i-1); ///>> decrement loop counter
        IP = GET(IP);       ///>> branch back to FOR
    }
    else {                  ///> or, 
        IP += CELLSZ;       ///>> skip to next instruction
        RPOP();             ///>> pop off return stack
    }
    ef_yield();             ///> steal some cycles. Note: 17ms/cycle on Arduino UNO
    NEXT();
}
void _qbran()               /// (f -- ) test top as a flag on data stack
{
    if (top) IP += CELLSZ;  ///> next instruction, or
    else     IP = GET(IP);  ///> fetch branching target address
    POP();
    NEXT();
}
void _bran()                /// ( -- ) branch to address following
{
    IP = GET(IP);           ///> fetch branching target address
    NEXT();
}
///@}
///
///@name Storage Ops
///@{
void _store()               /// (n a -- ) store into memory location from top of stack
{
    SET(top, SS(S--));
    POP();
    NEXT();
}
void _at()                  /// (a -- n) fetch from memory address onto top of stack
{
    top = (S16)GET(top);
    NEXT();
}
void _cstor()               /// (c b -- ) store a byte into memory location
{
    BSET(top, SS(S--));
    POP();
    NEXT();
}
void _cat()                 /// (b -- n) fetch a byte from memory location
{
    top = (S16)BGET(top);
    NEXT();
}
void _pstor()               /// (n a -- ) add n to content at address a
{
    SET(top, GET(top)+SS(S--));
    POP();
    NEXT();
}
///@}
///
///@name Return Stack Ops
///@{
void _rfrom()               /// (-- w) pop from return stack onto data stack (Ting comments different ???)
{
    PUSH(RPOP());
    NEXT();
}
void _rat()                 /// (-- w) copy a number off the return stack and push onto data stack
{
    PUSH(RS(R));
    NEXT();
}
void _tor()                 /// (w --) pop from data stack and push onto return stack
{
    RPUSH(top);
    POP();
    NEXT();
}
///@}
///
///@name Data Stack Ops
///@{
void _depth()               /// (-- w) fetch current stack depth
{
    PUSH(S);
    NEXT();
}
void _drop()                /// (w -- ) drop top of stack item
{
    POP();
    NEXT();
}
void _dup()                 /// (w -- w w) duplicate to of stack
{
    SS(++S)=top;
    NEXT();
}
void _swap()                /// (w1 w2 -- w2 w1) swap top two items on the data stack
{
    S16 tmp  = top;
    top = SS(S);
    SS(S)=tmp;
    NEXT();
}
void _over()                /// (w1 w2 -- w1 w2 w1) copy second stack item to top
{
    PUSH(SS(S));         ///> push w1
    NEXT();
}
void _rot()                 /// (w1 w2 w3 -- w2 w3 w1) rotate 3rd item to top
{
    S16 tmp = SS(S-1);
    SS(S-1)=SS(S);
    SS(S)  =top;
    top = tmp;
    NEXT();
}
void _pick()                /// (... +n -- ...w) copy nth stack item to top
{
    top = SS(S - (U8)top);
    NEXT();
}
void _qdup()                /// (w -- w w | 0) dup top of stack if it is not zero
{
    if (top) SS(++S)=top;
    NEXT();
}
///@}
///
///@name Logic Ops
///@{
void _and()                 /// (w w -- w) bitwise AND
{
    top &= SS(S--);
    NEXT();
}
void _or()                  /// (w w -- w) bitwise OR
{
    top |= SS(S--);
    NEXT();
}
void _xor()                 /// (w w -- w) bitwise XOR
{
    top ^= SS(S--);
    NEXT();
}
void _great()               /// (n1 n2 -- t) true if n1>n2
{
    top = BOOL(SS(S--) > top);
    NEXT();
}
void _less()                /// (n1 n2 -- t) true if n1<n2
{
    top = BOOL(SS(S--) < top);
    NEXT();
}
void _equal()               /// (w w -- t) true if top two items are equal
{
    top = BOOL(SS(S--)==top);
    NEXT();
}
void _invert()             /// (w -- ~w) one's complement
{
    top = -top - 1;
    NEXT();
}
void _zless()               /// (n -- f) check whether top of stack is negative
{
    top = BOOL(top < 0);
    NEXT();
}
void _uless()               /// (u1 u2 -- t) unsigned compare top two items
{
    top = BOOL((U16)(SS(S--)) < (U16)top);
    NEXT();
}
void _lshift()              /// (w n -- w) left shift n bits
{
    top = SS(S--) << top;
    NEXT();
}
void _rshift()              /// (w n -- w) right shift n bits
{
    top = SS(S--) >> top;
    NEXT();
}
///@}
///
///@name Arithmetic Ops
///@{
void _abs()                 /// (n -- n) absolute value of n
{
    U16 m = top>>15;
    top = (top + m) ^ m;    ///> no branching
    NEXT();
}
void _mod()                 /// (n n -- r) signed divide, returns mod
{
    top = (top) ? SS(S--) % top : SS(S--);
    NEXT();
}
void _negate()             /// (n -- -n) two's complement
{
    top = 0 - top;
    NEXT();
}
void _onep()                /// (n -- n+1) add one to top
{
    top++;
    NEXT();
}
void _onem()               /// (n -- n-1) minus one to top
{
    top--;
    NEXT();
}
void _plus()                /// (w w -- sum) add top two items
{
    top += SS(S--);
    NEXT();
}
void _sub()                 /// (n1 n2 -- n1-n2) subtraction
{
    top = SS(S--) - top;
    NEXT();
}
void _star()                /// (n n -- n) signed multiply, return single product
{
    top *= SS(S--);
    NEXT();
}
void _slash()               /// (n n - q) signed divide, return quotient
{
    top = (top) ? SS(S--) / top : (S--, 0);
    NEXT();
}
void _uplus()               /// (w w -- w c) add two numbers, return the sum and carry flag
{
    SS(S) = SS(S)+top;
    top = (U16)SS(S) < (U16)top;
    NEXT();
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
void _umstar()              /// (u1 u2 -- ud) unsigned multiply return double product
{
    U32 u = (U32)SS(S) * top;
    SS(S) = (U16)(u & 0xffff);
    top   = (U16)(u >> 16);
    NEXT();
}
///@}
///
///@name HighLevel Ops
///@{
/* deprecated (use high-level FORTH)
void _msmod()               /// (d n -- r q) signed floored divide of double by single
{
    S32 d = (S32)top;
    S32 m = ((S32)SS(S)<<16) + SS(S-1);
    POP();
    SS(S) = (S16)(m % d);   // remainder
    top   = (S16)(m / d);   // quotient
    NEXT();
}
void _slmod()               /// (n1 n2 -- r q) signed devide, return mod and quotient
{
    if (top) {
        S16 tmp = SS(S) / top;
        SS(S) = SS(S) % top;
        top = tmp;
    }
    NEXT();
}
void _ssmod()               /// (n1 n2 n3 -- r q) n1*n2/n3, return mod and quotion
{
    S32 m = (S32)SS(S-1) * SS(S);
    S16 d = top;
    POP();
    SS(S) = (S16)(m % d);
    top   = (S16)(m / d);
    NEXT();
}
void _stasl()               /// (n1 n2 n3 -- q) n1*n2/n3 return quotient
{
    S32 m = (S32)SS(S-1) * SS(S);
    S16 d = top;
    POP();
    POP();
    top = (S16)(m / d);
    NEXT();
}
void _count()               /// (b -- b+1 +n) count byte of a string and add 1 to byte address
{
    SS(++S) = top + 1;
    top = (S16)BGET(top);
    NEXT();
}
void _max_()                /// (n1 n2 -- n) return greater of two top stack items
{
    if (top < SS(S)) POP();
    else (U8)S--;
    NEXT();
}
void _min_()                /// (n1 n2 -- n) return smaller of two top stack items
{
    if (top < SS(S)) S--;
    else POP();
    NEXT();
}
void _ddrop()               /// (w w --) drop top two items
void _ddup()                /// (w1 w2 -- w1 w2 w1 w2) duplicate top two items
void _dstor()               /// (d a -- ) store the double to address a
{
    SET(top+CELLSZ, SS(S--));
    SET(top,        SS(S--));
    POP();
    NEXT();
}
void _dat()                 /// (a -- d) fetch double from address a
{
    PUSH(GET(top));
    top = GET(top + CELLSZ);
    NEXT();
}
*/
///@}
///
///@name Double Precision Ops
///@{
void _mstar()               /// (n1 n2 -- d) signed multiply, return double product
{
    S32 d = (S32)SS(S) * top;
    DTOP(d);
    NEXT();
}
void _dnegate()             /// (d -- -d) two's complemente of top double
{
    S32 d = ((S32)top<<16) | (SS(S) & 0xffff);
    DTOP(-d);
    NEXT();
}
void _dplus()               /// (d1 d2 -- d1+d2) add two double precision numbers
{
    S32 d0 = ((S32)top<<16)     | (SS(S)&0xffff);
    S32 d1 = ((S32)SS(S-1)<<16) | (SS(S-2)&0xffff);
    S -= 2;
    DTOP(d1 + d0);
    NEXT();
}
void _dsub()                /// (d1 d2 -- d1-d2) subtract d2 from d1
{
    S32 d0 = ((S32)top<<16)     | (SS(S)&0xffff);
    S32 d1 = ((S32)SS(S-1)<<16) | (SS(S-2)&0xffff);
    S -= 2;
    DTOP(d1 - d0);
    NEXT();
}
///@}
///
///@name Arduino Specific
///@{
void _delay()               /// (n -- ) delay n milli-second
{
    U32 t  = millis() + top;///> calculate break time
    POP();
    while (millis()<t) {    ///> loop until break time reached
        ef_yield();         ///> or, run hardware tasks while waiting
    }
    NEXT();
}
void _clock()               /// ( -- d) current clock value, a double precision number
{
    U32 t = millis();
    PUSH(t & 0xffff);       ///> stored double on stack top
    PUSH(t >> 16);
    NEXT();
}
void _pinmode()             /// (pin mode --) pinMode(pin, mode)
{
    pinMode(top, SS(S) ? OUTPUT : INPUT);
    POP();
    POP();
    NEXT();
}
void _map()                 /// (f1 f2 t1 t2 n -- nx) map(n, f1, f2, t1, t2)
{
    U16 tmp = map(top, SS(S-3), SS(S-2), SS(S-1), SS(S));
    S -= 4;
    top = tmp;
    NEXT();
}
void _din()                 /// (pin -- n) read from Arduino digital pin
{
    PUSH(digitalRead(POP()));
    NEXT();
}
void _dout()                /// (pin n -- ) write to Arduino digital pin
{
    digitalWrite(top, SS(S));
    POP();
    POP();
    NEXT();
}
void _ain()                 /// (pin -- n) read from Arduino analog pin
{
    PUSH(analogRead(POP()));
    NEXT();
}
void _aout()                /// (pin n -- ) write PWM to Arduino analog pin
{
    analogWrite(top, SS(S));
    POP();
    POP();
    NEXT();
}
///@}
///
/// primitive function lookup table
/// Note: subroutine indirected threading
/// TODO: computed goto
///
void(*prim[FORTH_PRIMITIVES])() = {
    /* case 0 */ _nop,
    /* case 1 */ _init,
    /* case 2 */ _qrx,
    /* case 3 */ _txsto,
    /* case 4 */ _docon,
    /* case 5 */ _dolit,
    /* case 6 */ _enter,
    /* case 7 */ _exit,
    /* case 8 */ _execu,
    /* case 9 */ _donext,
    /* case 10 */ _qbran,
    /* case 11 */ _bran,
    /* case 12 */ _store,
    /* case 13 */ _at,
    /* case 14 */ _cstor,
    /* case 15 */ _cat,
    /* case 16  opRPAT  */ _onep,
    /* case 17  opRPSTO */ _onem,
    /* case 18 */ _rfrom,
    /* case 19 */ _rat,
    /* case 20 */ _tor,
    /* case 21 opSPAT  */ _delay,
    /* case 22 opSPSTO */ _clock,
    /* case 23 */ _drop,
    /* case 24 */ _dup,
    /* case 25 */ _swap,
    /* case 26 */ _over,
    /* case 27 */ _zless,
    /* case 28 */ _and,
    /* case 29 */ _or,
    /* case 30 */ _xor,
    /* case 31 */ _uplus,
    /* case 32 */ _depth,
    /* case 33 */ _qdup,
    /* case 34 */ _rot,
    /* case 35 */ _lshift,
    /* case 36 */ _rshift,
    /* case 37 */ _plus,
    /* case 38 */ _invert,
    /* case 39 */ _negate,
    /* case 40 opDNEGA */ _great,
    /* case 41 */ _sub,
    /* case 42 */ _abs,
    /* case 43 */ _equal,
    /* case 44 */ _uless,
    /* case 45 */ _less,
    /* case 46 */ _ummod,
    /* case 47 opMSMOD */ _pinmode,
    /* case 48 opSLMOD */ _map,
    /* case 49 */ _mod,
    /* case 50 */ _slash,
    /* case 51 */ _umstar,
    /* case 52 */ _star,
    /* case 53 */ _mstar,
    /* case 54 opSSMOD */ _din,
    /* case 55 opSTASL */ _dout,
    /* case 56 */ _pick,
    /* case 57 */ _pstor,
    /* case 58 opDSTOR */ _ain,
    /* case 59 opDAT   */ _aout,
    /* case 60 */ _dnegate,
    /* case 61 */ _dovar,
    /* case 62 */ _dplus,
    /* case 63 */ _dsub,
};

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
void vm_init(PGM_P rom, U8 *cdata, void *io_stream) {
    EfVM::io     = (Stream *)io_stream;
    EfVM::cRom   = rom;
    EfVM::cData  = cdata;
    EfVM::cStack = (S16*)&cdata[FORTH_STACK_ADDR - FORTH_RAM_ADDR];
    
    EfVM::_init();                   // resetting user variables
}
///
/// eForth virtual machine (single-step) execution unit
/// @return
///   0 - exit
///
int vm_step() {
    EfVM::TRACE_WORD();                    // tracing stack and word name
    EfVM::prim[EfVM::BGET(EfVM::PC)]();    // walk bytecode stream

    return (int)EfVM::PC;
}
