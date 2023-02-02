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
 *     0x2000-0x24ff RAM (2K dynamic memory)
 *         0x2000-0x201f User Variables
 *         0x2020-0x23ff User Dictionary
 *         0x2400-0x247f Data/Return Stacks
 *         0x2480-0x24ff TIB (Terminal Input Buffer)
 *         0x2500        heap
 * @endcode
 *
 * ####Data and Return Stack
 *
 * @code
 *            S                   R
 *            |                   |
 *    top -> [S0, S1, S2,..., R1, R0] <- rtop
 * @endcode
 * Note: Dr. Ting uses U8 (0~255) for wrap-around control
 */
#include "eforth_core.h"

namespace EfVM {

static Stream *io;
///
///@name Control
///@{
IU  PC;                           ///< program counter, IU is 16-bit
IU  IP;                           ///< instruction pointer, IU is 16-bit, opcode is 8-bit
DU  top;                          ///< ALU (i.e. cached top of stack value)
DU  rtop;                         ///< cached loop counter on return stack
IU  IR;                           ///< interrupt service routine
///@}
///
///@name Storage
///@{
PGM_P _rom;                       ///< ROM, Forth word stored in Arduino Flash Memory
U8    *_data;                     ///< RAM, memory block for user define dictionary
DU    *DS;                        ///< data stack pointer, Dr. Ting's stack
DU    *RS;                        ///< return stack pointer, Dr. Ting's rack
///@}
#define RAM_FLAG       0xe000     /**< RAM ranger      (0x2000~0x7fff) */
#define OFF_MASK       0x07ff     /**< RAM offset mask (0x0000~0x07ff) */
#define IRET_FLAG      0x8000     /**< interrupt return flag           */
#define BOOL(f)        ((f) ? TRUE : FALSE)
#define RAM(i)         &_data[(i) - FORTH_RAM_ADDR]
///
/// byte (8-bit) fetch from either RAM or ROM depends on filtered range
///
U8 BGET(U16 d) {
    return (U8)((d&RAM_FLAG) ? _data[d&OFF_MASK] : pgm_read_byte(_rom+d));
}
///
/// word (16-bit) fetch from either RAM or ROM depends on filtered range
///
U16 GET(U16 d) {
    return (d&RAM_FLAG)
        ? ((U16)_data[d&OFF_MASK]<<8) | _data[(d+1)&OFF_MASK]
        : ((U16)pgm_read_byte(_rom+d)<<8) | pgm_read_byte(_rom+d+1);
}
#define BSET(d, c)     (_data[(d)&OFF_MASK]=(U8)(c))
void SET(U16 d, U16 v) {
	_data[(d)&OFF_MASK]   = v>>8;
	_data[(d+1)&OFF_MASK] = v&0xff;
}
#define S2D(h, l)      (((S32)(h)<<16) | ((l)&0xffff))
///
/// push a value onto stack top
///
void PUSH(DU v)        { *++DS = top;  top  = v; }
void RPUSH(DU v)       { *--RS = rtop; rtop = v; }
#define POP()          (top  = *DS--)
#define RPOP()         (rtop = *RS++)
#define DTOP(d)        { *DS = (d) & 0xffff; top = (d)>>16; }
///
/// update program counter (ready to fetch), advance instruction pointer
///
void NEXT() { PC=GET(IP); IP+=sizeof(IU); }

DU _depth() {
    return (DU)((U8*)DS - RAM(FORTH_STACK_ADDR)) >> 1;
}
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
#define opDOCON 4
#define opDOLIT 5
#define opENTER 7
#define opEXIT  8
#define opEXEC  9

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
    DU s = _depth() - 1;                     ///< stack depth (minus top)
    while (s-- > 0) {
        DU *vp = ((DU*)DS - s);
        DU v   = *vp;
        LOG_H("_", v);
    }
    LOG_H("_", top);
    LOG("_");
    /// special opcode handlers for DOLIT, ENTER, EXIT
    switch (op) {
    case opDOCON:
    case opDOLIT: LOG_H("$", GET(IP)); LOG(" "); break;
    case opEXIT:
    	LOG(";");
    	--tTAB;
    	break;
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
#define opENTER 7
#define DEBUG(s,v)
#define TAB()         /* skip */
#define TRACE(op)     /* skip */
#endif // EXE_TRACE
///@}
//
// Forth Virtual Machine primitive functions
//
///
/// virtual machine initializer
///
void _init() {
    intr_reset();                     /// * reset interrupt handlers

    PC = IP = IR = top = 0;           ///> setup control variables
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
    IU p = FORTH_UVAR_ADDR;           ///> setup Forth user variables
    SET(p,   FORTH_TIB_ADDR);         /// * set 'TIB pointer
    SET(p+2, 10);                     /// * set BASE to 10
    SET(p+4, FORTH_DIC_ADDR);         /// * top of dictionary
    ///
    /// display init prompt
    ///
    LOG("\n\n"); LOG(APP_NAME); LOG(" "); LOG(MAJOR_VERSION);
}

///
///> serve interrupt routines
///
void _yield()                ///> yield to interrupt service
{
    IR = intr_service();              /// * check interrupts
    if (IR) {                         /// * service interrupt?
        RPUSH(IP | IRET_FLAG);        /// * flag return address as IRET
        IP = IR + 1;                  /// * skip opENTER
    }
}
int _yield_cnt = 0;          ///< interrupt service throttle counter
#define YIELD_PERIOD    100
#define YIELD()                               \
    if (!IR && ++_yield_cnt > YIELD_PERIOD) { \
        _yield_cnt = 0;                       \
        _yield();                             \
    }
///
///> console IO functions
///
void _qrx()                  ///> ( -- c ) fetch a char from console
{
#if ARDUINO
    int rst = io->read();             ///> fetch from IO stream
    if (rst > 0) PUSH((DU)rst);
    PUSH(BOOL(rst >= 0));
#else
    PUSH((DU)getchar());              /// * Note: blocking, i.e. no interrupt support
    PUSH(TRUE);
#endif
}

void _txsto()                ///> (c -- ) send a char to console
{
#if EXE_TRACE
    if (tCNT) {
        switch (top) {
        case 0xa: tCNT ? LOG("<LF>") : LOG("\n");  break;
        case 0xd: LOG("<CR>");    break;
        case 0x8: LOG("<TAB>");   break;
        default: LOG("<"); LOG_C((char)top); LOG(">");
        }
    }
    else LOG_C((char)top);
#else  // !EXE_TRACE
    LOG_C((char)top);
#endif // EXE_TRACE
    POP();
}
///@}
void _ummod()               /// (udl udh u -- ur uq) unsigned divide of a double by single
{
    U32 d = (U32)top;       ///> CC: auto variable uses C stack 
    U32 m = ((U32)*DS<<16) + (U16)*(DS-1);
    POP();
    *DS   = (DU)(m % d);    ///> remainder
    top   = (DU)(m / d);    ///> quotient
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

void vm_init(PGM_P rom, U8 *data, void *io_stream) {
    io     = (Stream *)io_stream;
    _rom   = rom;
    _data  = data;
    DS     = (DU*)RAM(FORTH_STACK_ADDR);
    RS     = (DU*)RAM(FORTH_STACK_TOP);

    _init();                    /// * resetting user variables
}
///
/// eForth virtual machine outer interpreter (single-step) execution unit
/// @return
///   0 - exit
/// Note:
///   vm_outer - computed label jumps (25% faster than subroutine calls)
///
#define OP(name)    &&L_##name /** redefined for label address */
#define _X(n, code) L_##n: { DEBUG("%s",#n); code; continue; }

void vm_outer() {
    static void* vt[] PROGMEM = {       ///< computed label lookup table
        &&L_NOP,                        ///< opcode 0
        OPCODES                         ///< convert opcodes to address of labels
    };
    IP = GET(0) & ~0x8000;              ///> fetch cold boot vector

    while (1) {
        //YIELD();                      /// * serve interrupt if any
        U8  op = BGET(IP++);
        if (op & 0x80) {
        	PC = (U16)(op & 0x7f) << 8; /// * take upper 8-bit of address
        	op = opENTER;
        }
        TRACE(op);

        goto *vt[op];                   ///> jump to computed label
        ///
        /// the following part is in assembly for most of Forth implementations
        ///
        _X(NOP,   {});
#if ARDUINO
        _X(BYE,   _init());            /// * reset
#else // !ARDUINO
        _X(BYE,   break);              /// quit
#endif // ARDUINO
        ///
        /// @name Console IO
        /// @{
        _X(QRX,   _qrx());              ///> fetch char from input console
        _X(TXSTO, _txsto());            ///> send char to output console
        /// @}
        /// @name Built-in ops
        /// @{
        _X(DOCON,
            PUSH(GET(IP));              ///> push onto data stack
            IP += CELLSZ);              ///> skip to next instruction
        _X(DOLIT,
            PUSH(GET(IP));              ///> push onto data stack
            IP += CELLSZ);              ///> skip to next instruction
        _X(DOVAR, PUSH(++PC));
        /// @}
        /// @name Branching ops
        /// @{
        _X(ENTER,
            PC |= BGET(IP++);           /// * fetch low-byte of PC
            DEBUG(">>%x", IP);
            RPUSH(IP);                  ///> keep return address
            IP = PC);                   ///> jump to next instruction
        _X(EXIT,
        	TAB();
            IP = rtop;
            RPOP();                     ///> pop return address
            if (IP & IRET_FLAG) {       /// * IRETURN?
                IR = 0;                 /// * interrupt disabled
                IP &= ~IRET_FLAG;
            });
        _X(EXECU,                       ///> ( xt -- ) execute xt
            DEBUG(">>%x", IP);
            RPUSH(IP);
        	IP = (IU)top;               /// * fetch program counter
        	POP());
        _X(DONEXT,
            if (rtop-- > 0) {           ///> check if loop counter > 0
                IP = GET(IP);           ///>> branch back to FOR
            }
            else {                      ///> or,
                IP += CELLSZ;           ///>> skip to next instruction
                RPOP();                 ///>> pop off return stack
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
            SET(top, *DS--);
            POP());
        _X(PSTOR,
            SET(top, GET(top) + *DS--);
            POP());
        _X(AT,    top = (DU)GET(top));
        _X(CSTOR,
            BSET(top, *DS--);
            POP());
        _X(CAT,   top = (DU)BGET(top));
        _X(RFROM, PUSH(rtop); RPOP());
        _X(RAT,   PUSH(rtop));
        _X(TOR,
            RPUSH(top);
            POP());
        /// @{
        /// @name Stack ops
        /// @}
        _X(DROP,  POP());
        _X(DUP,   *++DS = top);
        _X(SWAP,
            DU tmp = top;
            top    = *DS;
            *DS    = tmp);
        _X(OVER,  DU v = *DS; PUSH(v));      /// * note PUSH macro changes DS
        _X(ROT,
            DU tmp = *(DS-1);
            *(DS-1)= *DS;
            *DS    = top;
            top    = tmp);
        _X(PICK,  top = *(DS - top));
        /// @}
        /// @name ALU ops
        /// @{
        _X(AND,   top &= *DS--);
        _X(OR,    top |= *DS--);
        _X(XOR,   top ^= *DS--);
        _X(INV,   top = -top - 1);
        _X(LSH,   top =  *DS-- << top);
        _X(RSH,   top =  *DS-- >> top);
        _X(ADD,   top += *DS--);
        _X(SUB,   top =  *DS-- - top);
        _X(MUL,   top *= *DS--);
        _X(DIV,   top = top ? *DS-- / top : (DS--, 0));
        _X(MOD,   top = top ? *DS-- % top : *DS--);
        _X(NEG,   top = -top);
        /// @}
        /// @name Logic ops
        /// @{
        _X(GT,    top = BOOL(*DS-- > top));
        _X(EQ,    top = BOOL(*DS-- ==top));
        _X(LT,    top = BOOL(*DS-- < top));
        _X(ZGT,   top = BOOL(top > 0));
        _X(ZEQ,   top = BOOL(top == 0));
        _X(ZLT,   top = BOOL(top < 0));
        /// @}
        /// @name Misc. ops
        /// @{
        _X(ONEP,  top++);
        _X(ONEM,  top--);
        _X(QDUP,  if (top) *++DS = top);
        _X(DEPTH, DU d = _depth(); PUSH(d));
        _X(ULESS, top = BOOL((U16)*DS-- < (U16)top));
        _X(UMMOD, _ummod());              /// (udl udh u -- ur uq) unsigned divide of a double by single
        _X(UMSTAR,                        /// (u1 u2 -- ud) unsigned multiply return double product
            U32 u = (U32)*DS * top;
            DTOP(u));
        _X(MSTAR,                         /// (n1 n2 -- d) signed multiply, return double product
            S32 d = (S32)*DS * top;
            DTOP(d));
        /// @}
        /// @name Double precision ops
        /// @{
        _X(DNEG,                          /// (d -- -d) two's complemente of top double
            S32 d = S2D(top, *DS);
            DTOP(-d));
        _X(DADD,                          /// (d1 d2 -- d1+d2) add two double precision numbers
            S32 d0 = S2D(top, *DS);
            S32 d1 = S2D(*(DS-1), *(DS-2));
            DS -= 2; DTOP(d1 + d0));
        _X(DSUB,                          /// (d1 d2 -- d1-d2) subtract d2 from d1
            S32 d0 = S2D(top, *DS);
            S32 d1 = S2D(*(DS-1), *(DS-2));
            DS -= 2; DTOP(d1 - d0));
        /// @}
        /// @name Arduino specific ops
        /// @{
        _X(CLK,
            U32 t = millis();
            *++DS = top; DS++;            /// * allocate 2-cells for clock ticks
            DTOP(t));
        _X(PIN,
            pinMode(top, *DS ? OUTPUT : INPUT);
            POP(); POP());
        _X(MAP,
            U16 tmp = map(top, *(DS-3), *(DS-2), *(DS-1), *DS);
            DS -= 4;
            top = tmp);
        _X(IN,    top = digitalRead(top));
        _X(OUT,   digitalWrite(top, *DS);   POP(); POP());
        _X(AIN,   top = analogRead(top));
        _X(PWM,   analogWrite(top, *DS);    POP(); POP());
        _X(TMISR, intr_add_timer(top, *DS); POP(); POP());
        _X(PCISR, intr_add_pci(top, *DS);   POP(); POP());
        _X(TMRE,  intr_timer_enable(top);   POP());
        _X(PCIE,  intr_pci_enable(top);     POP());
        _X(RP,
            DU r = ((U8*)RAM(FORTH_STACK_TOP) - (U8*)RS) >> 1;
            PUSH(r));
#if EXE_TRACE
        _X(TRC,  tCNT = top; POP());
#else
        _X(TRC,  POP());
#endif // EXE_TRACE
        _X(SAVE,
            U16 sz = ef_save(_data);
            LOG_V(" -> EEPROM ", sz); LOG(" bytes\n");
        );
        _X(LOAD,
            U16 sz = ef_load(_data);
            LOG_V(" <- EEPROM ", sz); LOG(" bytes\n");
        );
    }
}
