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
///
///@name VM Registers
///@{
IU  IP;                           ///< instruction pointer, IU is 16-bit, opcode is 8-bit
IU  W;                            ///< work register, IU is 16-bit
DU  *DS;                          ///< data stack pointer, Dr. Ting's stack
DU  *RS;                          ///< return stack pointer, Dr. Ting's rack
DU  top;                          ///< ALU (i.e. cached top of stack value)
DU  rtop;                         ///< cached loop counter on return stack
IU  IR;                           ///< interrupt service routine
///@}
///@name Memory Management Unit
///@{
PGM_P _rom;                       ///< ROM, Forth word stored in Arduino Flash Memory
U8    *_ram;                      ///< RAM, memory block for user define dictionary
U8    *_pre;                      ///< Pre-built/embedded Forth code
CFP   _fp[CFUNC_MAX];             ///> store C function pointer
///@}
///@name IO Streaming interface
///@{
Stream *io;
///@}
///
///> Forth Virtual Machine primitive functions
///
///
/// virtual machine initializer
///
void _init() {
    intr_reset();                     /// * reset interrupt handlers

    IR = top = 0;                     ///> setup control variables
    DS = (DU*)RAM(FORTH_STACK_ADDR - CELLSZ);
    RS = (DU*)RAM(FORTH_STACK_TOP);

#if EXE_TRACE
    tCNT = 0; tTAB = 0;               ///> setup tracing variables
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

    IP = GET(0) & ~fCOLON;            ///> fetch cold boot vector
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
#define YIELD_PERIOD 50
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
    static char *p = (char*)_pre;
#if ARDUINO
    char c   = p ? pgm_read_byte(p) : 0;
    DU   rst = (DU)(c ? (p++, c) : io->read());        ///> fetch from IO stream
    if (rst > 0) PUSH(rst);
    PUSH(BOOL(rst >= 0));
#else
    char c   = p ? *p : 0;
    DU   rst = (DU)(c ? (p++, c) : getchar());
    PUSH(rst);               /// * Note: blocking, i.e. no interrupt support
    PUSH(TRUE);
#endif
}

void _txsto()                ///> (c -- ) send a char to console
{
#if 0 && EXE_TRACE
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

void _out(U16 p, U16 v)
{
#if ARDUINO
    switch (p & 0x300) {
    case 0x100:            // PORTD (0~7)
        DDRD  = DDRD | (p & 0xfc);  /// * mask out RX,TX
        PORTD = (U8)(v & p) | (PORTD & ~p);
        break;
    case 0x200:            // PORTB (8~13)
        DDRB  = DDRB | (p & 0xff);
        PORTB = (U8)(v & p) | (PORTB & ~p);
        break;
    case 0x300:            // PORTC (A0~A6)
        DDRC  = DDRC | (p & 0xff);
        PORTC = (U8)(v & p) | (PORTC & ~p);
        break;
    default: digitalWrite(p, v);
    }
#endif  // ARDUINO
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

void vm_init(PGM_P rom, U8 *ram, void *io_stream, const char *code) {
    io    = (Stream *)io_stream;
    _rom  = rom;
    _ram  = ram;
    _pre  = (U8*)code;

    _init();                    /// * resetting user variables
}
///
///> C interface implementation
///  TODO: build formal C callstack construct
///
void _ccall() {
    CFP fp  = _fp[top];               ///> fetch C function pointer
    POP();                            ///> pop off TOS
    fp();                             ///> call C function
}

void vm_cfunc(int n, CFP fp) {
    _fp[n] = fp;
    LOG_V(", fp[", n); LOG_H("]=", (uintptr_t)fp);
}

void vm_push(int v) {                 /// proxy to VM
    PUSH(v);
}

int vm_pop() {
    int t = (int)top;
    POP();
    return t;
}
///
/// eForth virtual machine outer interpreter (single-step) execution unit
/// @return
///   0 - exit
void vm_outer() {
#if COMPUTED_JUMP
    /// Note:
    ///   computed label jumps
    ///      + overall 15% faster than subroutine calls
    ///      + but uses extra 180 bytes of RAM (avr-gcc failed to put vt in PROGMEM)
    ///      + the 'continue' in _X() macro behaves as $NEXT
    ///
    #define OP(name)     &&L_##name
    #define _X(n, code)  L_##n: { DEBUG("%s",#n); code; continue; }
    #define DISPATCH(op) goto *vt[op];
    #define opENTER      2              /** hardcoded for ENTER */
    PROGMEM const void *vt[] = {        ///< computed label lookup table
        OP(NOP),                        ///< opcode 0
        OPCODES                         ///< convert opcodes to address of labels
    };
#else // !COMPUTED_JUMP
    #define OP(name)     op##name
    #define _X(n, code)  case op##n: { DEBUG("%s",#n); { code; } break; }
    #define DISPATCH(op) switch(op)
    enum {
        OP(NOP) = 0,                    ///< opcodes start at 0
        OPCODES
    };
#endif // COMPUTED_JUMP
    IP = GET(0) & ~fCOLON;              ///> fetch cold boot vector

    while (1) {
        YIELD();                        /// * yield to interrupt services
        U8 op = BGET(IP++);             /// * NEXT in Forth's context
        if (op & 0x80) {                /// * COLON word?
            W  = (U16)(op & 0x7f) << 8; /// * take high-byte of 16-bit address
            W |= BGET(IP);              /// * fetch low-byte of IP
            op = opENTER;
        }
        TRACE(op, IP, W, top, NDS());   /// * debug tracing

        DISPATCH(op) {
        ///
        /// the following part is in assembly for most of Forth implementations
        ///
        _X(NOP,   {});
        _X(EXIT,
            TAB();
            IP = rtop; RPOP();          ///> pop return address
            if (IP & IRET_FLAG) {       /// * IRETURN?
                IR = 0;                 /// * interrupt disabled
                IP &= ~IRET_FLAG;
            });
        _X(ENTER,
            RPUSH(++IP);                ///> keep return address
            DEBUG(">>%x", IP);
            IP = W);                    ///> jump to next instruction
#if ARDUINO
        _X(BYE,   _init());             /// * reset
#else // !ARDUINO
        _X(BYE,   return);              /// * quit
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
        _X(DOES,
           PUSH(IP+1);                  /// * +1 means skip the offset byte
           IP += BGET(IP));             /// * skip offset bytes, to does> code
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
        _X(DEPTH, DU d = NDS(); PUSH(d));
        _X(RP,
            DU r = ((U8*)RAM(FORTH_STACK_TOP) - (U8*)RS) >> 1;
            PUSH(r));
        _X(BL,    PUSH(0x20));
        _X(CELL,  PUSH(CELLSZ));
        _X(ABS,   top  = abs(top));
        _X(MAX,   DU s = *DS--; if (s > top) top = s);
        _X(MIN,   DU s = *DS--; if (s < top) top = s);
        _X(WITHIN,                        /// ( u ul uh -- f ) 3rd item is within [ul, uh)
            DU ul = *DS--;
            DU u  = *DS--;
            top = BOOL((U16)(u - ul) < (U16)(top - ul)));
        _X(TOUPP, if (top >= 0x61 && top <= 0x7b) top &= 0x5f);
        _X(COUNT, *++DS = top + 1; top = BGET(top));
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
            S32 d = (S32)*DS * *(DS - 1);
            *--DS = (DU)(d % top);
            top   = (DU)(d / top));
        _X(SMOD,                          /// ( n1 n2 -- r q )
            DU s = *DS;
            *DS = s % top;
            top = s / top);
        _X(MSLAS,
            S32 d = (S32)*DS-- * *DS--;   /// ( n1 n2 n3 -- q ) multiply n1 n2, divided by n3 return quotient
            top = (DU)(d / top));
        _X(S2D,   S32 d = (S32)top; DS++; DTOP(d));
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
        /// TODO: add J
        /*
        _X(DSTOR,
           SET(top + CELLSZ, *DS--);
           SET(top, *DS--);
           POP());
        _X(DAT,
           *(++DS) = (DU)GET(top);
           top     = (DU)GET(top + CELLSZ));
        */
        _X(SPAT,
            DU r = (U8*)DS - (U8*)RAM(FORTH_STACK_ADDR);
            PUSH(FORTH_STACK_ADDR + r));
//        _X(S0, PUSH(FORTH_STACK_ADDR));      /// fixed, instead of a user variable
#if EXE_TRACE
        _X(TRC,  tCNT = top; POP());
#else
        _X(TRC,  POP());
#endif // EXE_TRACE
        _X(SAVE,
            U16 sz = ef_save(_ram);
            LOG_V(" -> EEPROM ", sz); LOG(" bytes\n");
        );
        _X(LOAD,
            U16 sz = ef_load(_ram);
            LOG_V(" <- EEPROM ", sz); LOG(" bytes\n");
        );
        _X(CALL,
            _ccall());                       /// * call C function
        _X(CLK,
            U32 t = millis();
            *++DS = top; DS++;               /// * allocate 2-cells for clock ticks
            DTOP(t));
        /// @}
        /// @name Arduino specific ops
        /// @{
        _X(PIN,
            pinMode(top, *DS-- ? OUTPUT : INPUT);
            POP());
        _X(MAP,
            U16 tmp = map(top, *(DS-3), *(DS-2), *(DS-1), *DS);
            DS -= 4;
            top = tmp);
        _X(IN,    top = digitalRead(top));
        _X(OUT,   _out(top, *DS);   DS--; POP());
        _X(AIN,   top = analogRead(top));
        _X(PWM,   analogWrite(top, *DS);    DS--; POP());
        _X(TMISR, intr_add_tmisr(top, *DS, *(DS-1)); DS-=2; POP());
        _X(PCISR, intr_add_pcisr(top, *DS); DS--; POP());
        _X(TMRE,  intr_timer_enable(top);   POP());
        _X(PCIE,  intr_pci_enable(top);     POP());
        }
    }
}
