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
 *         0x2000-0x23ff User Dictionary
 *         0x2400-0x241f User Variables
 *         0x2420-0x24ff Return/Data Stack
 *         0x2500-0x257f TIB (Terminal Input Buffer)
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
IU  PC;                           ///< PC (program counter, IU is 16-bit)
IU  IP;                           ///< IP (instruction pointer, IU is 16-bit)
U8  R;                            ///< return stack index (0-255)
U8  S;                            ///< data stack index (0-255)
U8  ISR;
DU  top;                          ///< ALU (i.e. cached top of stack value)
///@}
///
///@name Storage
///@{
PGM_P cRom;                       ///< ROM, Forth word stored in Arduino Flash Memory
U8    *cData;                     ///< RAM, memory block for user define dictionary
DU    *cStack;                    ///< pointer to stack/rack memory block
///@}
#define RAM_FLAG       0xe000     /**< RAM ranger      (0x2000~0xffff) */
#define OFF_MASK       0x07ff     /**< RAM offset mask (0x0000~0x07ff) */
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
#define SET(d, v)      (*((DU*)&cData[(d)&OFF_MASK])=(v))
#define SS(s)          (cStack[s])
#define RS(r)          (cStack[RS_TOP - (r)])
#define RS_TOP         (FORTH_STACK_SZ>>1)
///
/// push a value onto stack top
///
void PUSH(DU v)        { SS(++S) = top; top = v; }
void RPUSH(DU v)       { RS(++R) = v; }
#define POP()          (top = SS(S ? S-- : S))
#define RPOP()         (R ? RS(R--) : RS(R))
#define DTOP(d)        { SS(S) = (d) & 0xffff; top = (d)>>16; }
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
///
/// display eForth system information
///
void sys_info(U8 *cdata, int sz) {
	LOG_H("\nROM_SZ=x",   sz);
    LOG_H(", RAM_SZ=x",   FORTH_RAM_SZ);
    LOG_V(", Addr=",      (U16)sizeof(IU)*8);
    LOG_V("-bit, CELL=",  CELLSZ);
    LOG("-byte\nMemory MAP:");
#if ARDUINO
    U16 h = (U16)&cdata[FORTH_RAM_SZ];
    U16 s = (U16)&s;
    LOG_H(" heap=x", h);
    LOG_V("--> ", s - h);
    LOG_H(" <--auto=x", s);
#endif // ARDUINO
    LOG_H("\n  ROM  :x0000+", FORTH_ROM_SZ);
    LOG_H("\n  DIC  :x", FORTH_DIC_ADDR);   LOG_H("+", FORTH_DIC_SZ);
    LOG_H("\n  UVAR :x", FORTH_UVAR_ADDR);  LOG_H("+", FORTH_UVAR_SZ);
    LOG_H("\n  STACK:x", FORTH_STACK_ADDR); LOG_H("+", FORTH_STACK_SZ);
    LOG_H("\n  TIB  :x", FORTH_TIB_ADDR);   LOG_H("+", FORTH_TIB_SZ);
}
//
// Forth Virtual Machine primitive functions
//
/// 
/// virtual machine initializer
///
void _init() {
    intr_reset();                     /// * reset interrupt handlers

    ISR = R = S = PC = IP = top = 0;  ///> setup control variables
#if EXE_TRACE
    tCNT = 1; tTAB = 0;               ///> setup tracing variables
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
    IU p = FORTH_UVAR_ADDR;         ///> setup Forth user variables
    SET(p,   FORTH_TIB_ADDR);       /// * set 'TIB pointer
    SET(p+2, 10);                   /// * set BASE to 10
    SET(p+4, FORTH_DIC_ADDR);       /// * top of dictionary
    ///
    /// display init prompt
    ///
    LOG("\n\n"); LOG(APP_NAME); LOG(" "); LOG(MAJOR_VERSION);
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
    SS(S) = (DU)(m % d);    ///> remainder
    top   = (DU)(m / d);    ///> quotient
    NEXT();
}
///
///> serve interrupt routines
///
#define YIELD_PERIOD    10
void _nest(U16 xt) {
	RPUSH(0);                   ///> ISR exit token
	IP = xt;                    ///> point to service function
	NEXT();
	vm_outer();
}
void _yield()
{
	static U8 n = 0;
	if (++n < 10) return;       /// * give more cycles to VM
	n = 0;
	U16 hx = intr_hits();
	if (!hx) return;

	U8  S0  = S,  R0  = R;      /// * save context
	U16 PC0 = PC, IP0 = IP;

	ISR = 1;
	intr_service(_nest);
	ISR = 0;

	S  = S0;  R  = R0;          /// * restore context
	PC = PC0; IP = IP0;
}
void _delay()                   /// (n -- ) delay n milli-second
{
    U32 t  = millis() + top;    ///> calculate break time
    POP();
    while (millis()<t) {        ///> loop until break time reached
        _yield();               ///> or, run hardware tasks while waiting
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
    cStack = (DU*)&cdata[FORTH_STACK_ADDR - FORTH_RAM_ADDR];
    
    _init();                   /// * resetting user variables
}
///
/// eForth virtual machine outer interpreter (single-step) execution unit
/// @return
///   0 - exit
/// Note:
///   vm_outer - computed label jumps (25% faster than subroutine calls)
///
#define OP(name)    &&L_##name /** redefined for label address */
#define _X(n, code) L_##n: { code; goto vm_next; }
#define _Y(n, fn)   L_##n: { fn(); continue; }

int vm_outer() {
    static void* vt[] = {               ///< computed label lookup table
        &&L_NOP,                        ///< opcode 0
        OPCODES                         ///< convert opcodes to address of labels
    };
    do {
        TRACE_WORD();                   /// * tracing stack and word name

        U8 op = BGET(PC);               ///> fetch next opcode
        goto *vt[op];                   ///> jump to computed label
        ///
        /// the following part is in assembly for most of Forth implementations
        ///
        _X(NOP,   {});
        _Y(BYE,   _init);
        /// 
        /// @name Console IO
        /// @{
        _X(QRX,
            PUSH(ef_getchar());         ///> yield to user task until console input available
            if (top) PUSH(TRUE));
        _Y(TXSTO, _txsto);
        /// @}
        /// @name Built-in ops
        /// @{
        _X(DOCON,
            ++PC;                       ///> skip opDOCON opcode
            PUSH(GET(PC)));             ///> push cell value onto stack
        _X(DOLIT,
            TRACE(" ", GET(IP));        ///> fetch literal from data
            PUSH(GET(IP));              ///> push onto data stack
            IP += CELLSZ);              ///> skip to next instruction
        _X(DOVAR, ++PC; PUSH(PC));
        /// @}
        /// @name Branching ops
        /// @{
        _X(ENTER,
            _yield();
            TRACE_COLON();
            RPUSH(IP);                  ///> keep return address
            IP = ++PC);                 ///> skip opcode opENTER, advance to next instruction
        _X(EXIT,
            TRACE_EXIT();
            IP = RPOP());               ///> pop return address
        _Y(EXECU, _execu);
        _X(DONEXT,
            if (RS(R) > 0) {            ///> check if loop counter > 0
                RS(R)--;                ///>> decrement loop counter
                IP = GET(IP);           ///>> branch back to FOR
            }
            else {                      ///> or,
                IP += CELLSZ;           ///>> skip to next instruction
                RPOP();                 ///>> pop off return stack
                _yield();               ///> give system task some cycles
            });
        _X(QBRAN,
            if (top) IP += CELLSZ;      ///> next instruction, or
            else     IP = GET(IP);      ///> fetch branching target address
            POP());
        _X(BRAN,  IP = GET(IP));        ///> fetch branching target address
        /// @}
        /// @name Memory Storage ops
        /// @{
        _X(STORE,
            SET(top, SS(S--));
            POP());
        _X(PSTOR,
            SET(top, GET(top)+SS(S--));
            POP());
        _X(AT,    top = (DU)GET(top));
        _X(CSTOR,
            BSET(top, SS(S--));
            POP());
        _X(CAT,   top = (DU)BGET(top));
        _X(RFROM, PUSH(RPOP()));
        _X(RAT,   PUSH(RS(R)));
        _X(TOR,
            RPUSH(top);
            POP());
        /// @{
        /// @name Stack ops
        /// @}
        _X(DROP,  POP());
        _X(DUP,   SS(++S)=top);
        _X(SWAP,
            DU tmp = top;
            top    = SS(S);
            SS(S)  = tmp);
        _X(OVER,  PUSH(SS(S)));      ///> push w1
        _X(ROT,
            DU tmp = SS(S-1);
            SS(S-1)= SS(S);
            SS(S)  = top;
            top    = tmp);
        _X(PICK,  top = SS(S - (U8)top));
        /// @}
        /// @name ALU ops
        /// @{
        _X(AND,   top &= SS(S--));
        _X(OR,    top |= SS(S--));
        _X(XOR,   top ^= SS(S--));
        _X(INV,   top = -top - 1);
        _X(LSH,   top = SS(S--) << top);
        _X(RSH,   top = SS(S--) >> top);
        _X(ADD,   top += SS(S--));
        _X(SUB,   top = SS(S--) - top);
        _X(MUL,   top *= SS(S--));
        _X(DIV,   top = (top) ? SS(S--) / top : (S--, 0));
        _X(MOD,   top = (top) ? SS(S--) % top : SS(S--));
        _X(NEG,   top = 0 - top);
        /// @}
        /// @name Logic ops
        /// @{
        _X(GT,    top = BOOL(SS(S--) > top));
        _X(EQ,    top = BOOL(SS(S--)==top));
        _X(LT,    top = BOOL(SS(S--) < top));
        _X(ZGT,   top = BOOL(top > 0));
        _X(ZEQ,   top = BOOL(top == 0));
        _X(ZLT,   top = BOOL(top < 0));
        /// @}
        /// @name Misc. ops
        /// @{
        _X(ONEP,  top++);
        _X(ONEM,  top--);
        _X(QDUP,  if (top) SS(++S) = top);
        _X(DEPTH, PUSH(S));
        _X(ULESS, top = BOOL((U16)(SS(S--)) < (U16)top));
        _Y(UMMOD, _ummod);
        _X(UMSTAR,               /// (u1 u2 -- ud) unsigned multiply return double product
            U32 u = (U32)SS(S) * top;
            DTOP(u));
        _X(MSTAR,                /// (n1 n2 -- d) signed multiply, return double product
            S32 d = (S32)SS(S) * top;
            DTOP(d));
        /// @}
        /// @name Double precision ops
        /// @{
        _X(DNEG,                 /// (d -- -d) two's complemente of top double
            S32 d = ((S32)top<<16) | (SS(S) & 0xffff);
            DTOP(-d));
        _X(DADD,                 /// (d1 d2 -- d1+d2) add two double precision numbers
            S32 d0 = ((S32)top<<16)     | (SS(S)&0xffff);
            S32 d1 = ((S32)SS(S-1)<<16) | (SS(S-2)&0xffff);
            S -= 2; DTOP(d1 + d0));
        _X(DSUB,                 /// (d1 d2 -- d1-d2) subtract d2 from d1
            S32 d0 = ((S32)top<<16)     | (SS(S)&0xffff);
            S32 d1 = ((S32)SS(S-1)<<16) | (SS(S-2)&0xffff);
            S -= 2; DTOP(d1 - d0));
        /// @}
        /// @name Arduino specific ops
        /// @{
        _Y(DELAY, _delay);
        _X(CLK,                  /// fetch system clock in double precision
            S += 2; DTOP(millis()));
        _X(PIN,
            pinMode(top, SS(S) ? OUTPUT : INPUT);
            POP(); POP());
        _X(MAP,
            U16 tmp = map(top, SS(S-3), SS(S-2), SS(S-1), SS(S));
            S -= 4;
            top = tmp);
        _X(IN,  PUSH(digitalRead(POP())));
        _X(OUT,
            digitalWrite(top, SS(S));
            POP(); POP());
        _X(AIN, PUSH(analogRead(POP())));
        _X(PWM,
            analogWrite(top, SS(S));
            POP(); POP());
        _X(TMR, IU xt = POP(); intr_add_timer(POP(), xt));
        _X(PCI, IU xt = POP(); intr_add_pci(POP(), xt));
        _X(TMRE, intr_enable_timer(POP()));
        _X(PCIE, intr_enable_pci(POP()));

    vm_next:
        PC = (ISR && IP==0) ? 0 : GET(IP);  ///> fetch next program counter (branch)
        IP += sizeof(IU);                   ///> advance to next instruction
    } while (PC);

    return (int)PC;
}
