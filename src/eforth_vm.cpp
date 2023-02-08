/**
 * @file
 * @brief eForth Virtual Machine module
 *
 * Revision Note: Threading model evolution
 *   202212 EfVMSub module - vtable subroutine calling
 *   202301 indirect threading - computed label jumping
 *   202302 direct threading - opcode => code(); continue = $NEXT
 */
#include "eforth_vm.h"

namespace EfVM {

static Stream *io;
///
///@name VM Registers
///@{
IU  IP;                           ///< instruction pointer, IU is 16-bit, opcode is 8-bit
IU  PC;                           ///< program counter, IU is 16-bit
DU  *DS;                          ///< data stack pointer, Dr. Ting's stack
DU  *RS;                          ///< return stack pointer, Dr. Ting's rack
DU  top;                          ///< ALU (i.e. cached top of stack value)
DU  rtop;                         ///< cached loop counter on return stack
IU  IR;                           ///< interrupt service routine
///@}
///@name Memory Management Unit
///@{
PGM_P _rom;                       ///< ROM, Forth word stored in Arduino Flash Memory
U8    *_data;                     ///< RAM, memory block for user define dictionary
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
    DS = (DU*)RAM(FORTH_STACK_ADDR - CELLSZ);
    RS = (DU*)RAM(FORTH_STACK_TOP);

#if EXE_TRACE
    tCNT = 1; tTAB = 0;               ///> setup tracing variables
#endif  // EXE_TRACE

    /// FORTH_UVAR_ADDR;
    ///   'TIB console input buffer pointer
    ///   BASE current radix for numeric ops
    ///   CP,  top of dictionary, same as HERE
    ///   CONTEXT name field of last word
    ///   LAST, same as CONTEXT
    ///   'MODE eval mode (interpreter or compiler)
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
    IR = intr_service();          /// * check interrupts
    if (IR) {                     /// * service interrupt?
        RPUSH(IP | IRET_FLAG);    /// * flag return address as IRET
        IP = IR;                  /// * skip opENTER
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
void _ummod()               /// (udl udh u -- ur uq) unsigned double divided by a single
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

    _init();                    /// * resetting user variables
}
///
/// eForth virtual machine outer interpreter (single-step) execution unit
/// @return
///   0 - exit
/// Note:
///   vm_outer - computed label jumps (25% faster than subroutine calls)
///   continue in _X() macro behaves as $NEXT
///
#define OP(name)    &&L_##name          /** redefined for label address     */
#define _X(n, code) L_##n: { DEBUG("%s",#n); code; continue; }

void vm_outer() {
    const void* vt[] PROGMEM = {        ///< computed label lookup table
        &&L_NOP,                        ///< opcode 0
        OPCODES                         ///< convert opcodes to address of labels
    };
    IP = GET(0) & ~fCOLON;              ///> fetch cold boot vector

    while (1) {
        YIELD();                        /// * serve interrupt if any
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
        _X(DOLIT,
        	PUSH(GET(IP));              ///> push literal onto data stack
            IP += CELLSZ);
        _X(DOVAR, PUSH(IP+1));          ///> push literal addr to data stack
                                        /// * +1 means skip EXIT byte (08)
        /// @}
        /// @name Branching ops
        /// @{
        _X(EXECU,                       ///> ( xt -- ) execute xt
            DEBUG(">>%x", IP);
            RPUSH(IP);
        	IP = (IU)top;               /// * fetch program counter
        	POP());
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
        _X(DONEXT,
            TAB();
            if (rtop-- > 0) {           ///> check if loop counter > 0
                IP = GET(IP);           ///>> branch back to FOR
            }
            else {                      ///> or,
                IP += CELLSZ;           ///>> skip to next instruction
                RPOP();                 ///>> pop off return stack
            });
        _X(QBRAN,
            TAB();
            if (top) IP += CELLSZ;      ///> next instruction, or
            else     IP = GET(IP);      ///> fetch branching target address
            POP());
        _X(BRAN,                        ///> fetch branching target address
            TAB();
            IP = GET(IP));
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
        _X(INV,   top ^= -1);
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
        _X(DEPTH, DU d = DEPTH(); PUSH(d));
        _X(RP,
            DU r = ((U8*)RAM(FORTH_STACK_TOP) - (U8*)RS) >> 1;
            PUSH(r));
        _X(BL,    PUSH(0x20));
        _X(CELL,  PUSH(CELLSZ));
        _X(CELLP, top += CELLSZ);
        _X(CELLM, top -= CELLSZ);
        _X(CELLS, top *= CELLSZ);
        _X(ABS,   top  = abs(top));
        _X(MAX,   DU s = *DS--; if (s > top) top = s);
        _X(MIN,   DU s = *DS--; if (s < top) top = s);
 /*
>         _X(WITHIN,                        /// ( u ul uh -- f ) 3rd item is between first 2 on stack
>            DU ul = *DS--;
>            DU u  = *DS--;
>            top = BOOL(u > ul && u < top));
>         _X(TOUPP, if (top >= 0x61 && top <= 0x7b) top &= 0x5f);
>         _X(COUNT, PUSH(IP); PUSH(BGET(IP+1)));
> */
        _X(ULESS, top = BOOL((U16)*DS-- < (U16)top));
        _X(UMMOD, _ummod());              /// (udl udh u -- ur uq) unsigned divide of a double by single
        _X(UMSTAR,                        /// (u1 u2 -- ud) unsigned multiply return double product
            U32 u = (U32)*DS * top;
            DTOP(u));
        _X(MSTAR,                         /// (n1 n2 -- d) signed multiply, return double product
            S32 d = (S32)*DS * top;
            DTOP(d));
        _X(UMPLUS,                        /// ( n1 n2 -- sum c ) return sum of two numbers and carry flag
            U32 u = (U32)*DS + top;
            DTOP(u));
        _X(SSMOD,                         /// ( dl dh n -- r q ) double div/mod by a single
            S32 d = (S32)*DS-- * top;
            *DS  = (DU)(d % top);
            top  = (DU)(d / top));
        _X(SMOD,                          /// ( n1 n2 -- r q )
            DU s = *DS;
            *DS = s % top;
            top = s / top);
        _X(MSLAS,
            S32 d = (S32)*DS-- * *DS--;   /// ( n1 n2 n3 -- q ) multiply n1 n2, divided by n3 return quotient
            top = (DU)(d / top));
        _X(S2D,   S32 d = (S32)top; DTOP(d));
        _X(D2S,
           DU s = *DS--;
           top = (top < 0) ? -abs(s) : abs(s));
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
        _X(DDUP,  DU v = *DS; PUSH(v); v = *DS; PUSH(v));
        _X(DDROP, POP(); POP());
        /// TODO: add 2SWAP, 2OVER, 2+, 2-, 2*, 2/
        /// TODO: add I, J
        _X(DSTOR,
           SET(top + CELLSZ, *DS--);
           SET(top, *DS--);
           POP());
        _X(DAT,
           *(++DS) = (DU)GET(top);
           top     = (DU)GET(top + CELLSZ));
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
