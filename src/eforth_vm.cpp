/**
 * @file
 * @brief eForth Virtual Machine module
 *
 * Revision Note: Threading model evolution
 *   202212 EfVMSub module - vtable subroutine calling
 *   202301 indirect threading - computed label jumping
 *   202302 direct threading - opcode => code(); continue = $NEXT
 *   202405 add computed_goto; DS,RS,top,rtop=>S,R,T,I
 */
#include "eforth_vm.h"

namespace EfVM {
///
///@name VM Registers
///@{
DU    *S;              ///< data stack pointer,   'stack' in Dr. Ting's
DU    *R;              ///< return stack pointer, 'rack'  in Dr. Ting's
DU    T;               ///< TOS, cached top of stack value
DU    I;               ///< RTOS, cached loop counter on return stack
///@}
///@name IO Streaming interface
///@{
Stream *io;            ///< IO Stream, tied to Serial usually
///@}
///@name Memory Management Unit
///@{
PGM_P _rom;            ///< ROM, Forth built-in words stored in Flash
U8    *_ram;           ///< RAM, for user defined words
U8    *_pre;           ///< Forth code pre-defined/embedded in .ino
CFP   _api[CFUNC_MAX]; ///< C API function pointer
///@}
///
///> Forth Virtual Machine primitive functions
///
void _init() {                        ///> VM initializer
    intr_reset();                     /// * reset interrupt handlers

    T = 0;                            ///> setup control variables
    S = (DU*)RAM(FORTH_STACK_ADDR - CELLSZ);
    R = (DU*)RAM(FORTH_STACK_TOP);

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
    ///
    /// display init prompt
    ///
    LOG("\n\n"); LOG(APP_NAME);
}
///
///> console IO functions
///
inline void _qrx()           ///> ( -- c ) fetch a char from console
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

inline void _txsto()         ///> (c -- ) send a char to console
{
#if EXE_TRACE
    if (tCNT > 1) {
        switch (T) {
        case 0xa: tCNT ? LOG("<LF>") : LOG("\n");  break;
        case 0xd: LOG("<CR>");    break;
        case 0x8: LOG("<TAB>");   break;
        default: LOG("<"); LOG_C((char)T); LOG(">");
        }
    }
    else LOG_C((char)T);
#else  // !EXE_TRACE
    LOG_C((char)T);
#endif // EXE_TRACE
    POP();
}

inline void _out(U16 p, U16 v)   ///> Arduino port setting
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
    
inline void _ummod()        ///> (udl udh u -- ur uq) unsigned double divided by a single
{
    U32 d = (U32)T;         ///> CC: auto variable uses C stack
    U32 m = ((U32)*S<<16) + (U16)*(S-1);
    POP();
    *S   = (DU)(m % d);     ///> remainder
    T    = (DU)(m / d);     ///> quotient
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
    CFP fp  = _api[T];          ///> fetch C function pointer
    POP();                      ///> pop off TOS
    fp();                       ///> call C function
}

void vm_cfunc(int n, CFP fp) {
    _api[n] = fp;
    LOG_V(", API", n); LOG_H("=x", (uintptr_t)fp);
}

void vm_push(int v) {           /// proxy to VM
    PUSH(v);
}

int vm_pop() {
    int t = (int)T;
    POP();
    return t;
}
///
/// eForth virtual machine outer interpreter (single-step) execution unit
/// @return
///   0 - exit
///
#if COMPUTED_GOTO
/// Note:
///   computed goto
///      + overall ~5% faster, -100ms/100K, than token switch jump
///      + but uses extra 180 bytes of RAM (avr-gcc failed to put vt in PROGMEM)
///      + the 'continue' in _X() macro behaves as $NEXT
///
  #define OP(name)     &&L_##name
  #define VTABLE       const void *vt[] = { OP(NOP), OPCODES }
  #define DISPATCH(op) goto *vt[op];
  #define _X(n, code)  L_##n: { DEBUG("%s",#n); code; continue; }
  #define opENTER      2                  /** hardcoded for ENTER */
#else // !COMPUTED_GOTO
  #define OP(name)     op##name
  #define VTABLE       enum { OP(NOP) = 0, OPCODES }
  #define DISPATCH(op) switch(op)
  #define _X(n, code)  case op##n: { DEBUG("%s",#n); { code; } break; }
#endif // COMPUTED_GOTO

void vm_outer() {
    VTABLE;
    IU ir = 0;                          ///< interrupt flag
    IU ip = GET(FORTH_BOOT_ADDR);       ///< ip = cold boot vector
    while (1) {                         ///> Forth inner loop
        ///
        ///> serve interrupt routines
        ///
        if (!ir) {                      /// * still servcing interrupt
            ir = intr_service();        /// * get interrupt vectors
            if (ir) {                   /// * serve interrupt
                RPUSH(ip | IRET_FLAG);  /// * push IRET address
                ip = ir;                /// * skip opENTER
            }
        }
        ///
        ///> fetch primitive opcode or colon word address
        ///
        U8 op = BGET(ip++);             /// * fetch next opcode
        if (op & fCOLON8) {             /// * COLON word?
            RPUSH(ip + 1);              /// * save return address
            DEBUG(">>%x", ip + 1);
            ip = ((U16)(op & 0x7f)<<8)  /// * take high-byte of 16-bit address
                 | BGET(ip);            /// * and low-byte from *IP
            op = opENTER;               /// * doLIST a colon word
        }
        TRACE(op, ip, T, DEPTH());      /// * debug tracing
        
        DISPATCH(op) {
        ///
        /// the following part is in assembly for most of Forth implementations
        ///
        _X(NOP,   {});
        _X(EXIT,
            TAB();
            ip = I; RPOP();             ///> pop return address
            if (ip & IRET_FLAG) {       /// * IRETURN?
                ir = 0;                 /// * interrupt clear
                ip &= ~IRET_FLAG;
            });
        _X(ENTER, {});                  ///> handled above
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
            PUSH(GET(ip));              ///> push literal onto data stack
            ip += CELLSZ);
        _X(DOVAR, PUSH(ip+1));          ///> push literal addr to data stack
                                        /// * +1 means skip EXIT byte (08)
        /// @}
        /// @name Branching ops
        /// @{
        _X(EXECU,                       ///> ( xt -- ) execute xt
            DEBUG(">>%x", ip);
            RPUSH(ip);
            ip = (IU)T;                 /// * fetch program counter
            POP());
        _X(DOES,
           PUSH(ip+1);                  /// * +1 means skip the offset byte
           ip += BGET(ip));             /// * skip offset bytes, to does> code
        _X(DONEXT,
            TAB();
            if (I-- > 0) {              ///> check if loop counter > 0
                ip = GET(ip);           ///>> branch back to FOR
            }
            else {                      ///> or,
                ip += CELLSZ;           ///>> skip to next instruction
                RPOP();                 ///>> pop off return stack
            });
        _X(QBRAN,
            TAB();
            if (T) ip += CELLSZ;        ///> next instruction, or
            else   ip = GET(ip);        ///> fetch branching target address
            POP());
        _X(BRAN,                        ///> fetch branching target address
            TAB();
            ip = GET(ip));
        /// @}
        /// @name Memory Storage ops
        /// @{
        _X(STORE,
            SET(T, *S--);
            POP());
        _X(PSTOR,
            SET(T, (DU)GET(T) + *S--);
            POP());
        _X(AT,    T = (DU)GET(T));
        _X(CSTOR,
            BSET(T, *S--);
            POP());
        _X(CAT,   T = (DU)BGET(T));
        _X(RFROM, PUSH(I); RPOP());
        _X(RAT,   PUSH(I));
        _X(TOR,
            RPUSH(T);
            POP());
        /// @{
        /// @name Stack ops
        /// @}
        _X(DROP,  POP());
        _X(DUP,   *++S = T);
        _X(SWAP,
            DU tmp = T;
            T      = *S;
            *S     = tmp);
        _X(OVER,  DU v = *S; PUSH(v));      /// * note PUSH macro changes S
        _X(ROT,
            DU tmp = *(S-1);
            *(S-1) = *S;
            *S     = T;
            T      = tmp);
        _X(PICK,  T = *(S - T));
        /// @}
        /// @name ALU ops
        /// @{
        _X(AND,   T &= *S--);
        _X(OR,    T |= *S--);
        _X(XOR,   T ^= *S--);
        _X(INV,   T ^= -1);
        _X(LSH,   T =  *S-- << T);
        _X(RSH,   T =  *S-- >> T);
        _X(ADD,   T += *S--);
        _X(SUB,   T =  *S-- - T);
        _X(MUL,   T *= *S--);
        _X(DIV,   T = T ? *S-- / T : (S--, 0));
        _X(MOD,   T = T ? *S-- % T : *S--);
        _X(NEG,   T = -T);
        /// @}
        /// @name Logic ops
        /// @{
        _X(GT,    T = BOOL(*S-- > T));
        _X(EQ,    T = BOOL(*S-- ==T));
        _X(LT,    T = BOOL(*S-- < T));
        _X(ZGT,   T = BOOL(T > 0));
        _X(ZEQ,   T = BOOL(T == 0));
        _X(ZLT,   T = BOOL(T < 0));
        /// @}
        /// @name Misc. ops
        /// @{
        _X(ONEP,  T++);
        _X(ONEM,  T--);
        _X(QDUP,  if (T) *++S = T);
        _X(DEPTH, DU d = DEPTH(); PUSH(d));
        _X(RP,
            DU r = ((U8*)RAM(FORTH_STACK_TOP) - (U8*)R) >> 1;
            PUSH(r));
        _X(BL,    PUSH(0x20));
        _X(CELL,  PUSH(CELLSZ));
        _X(ABS,   T = abs(T));
        _X(MAX,   DU s = *S--; if (s > T) T = s);
        _X(MIN,   DU s = *S--; if (s < T) T = s);
        _X(WITHIN,                        /// ( u ul uh -- f ) 3rd item is within [ul, uh)
            DU ul = *S--;
            DU u  = *S--;
            T = BOOL((U16)(u - ul) < (U16)(T - ul)));
        _X(TOUPP, if (T >= 0x61 && T <= 0x7b) T &= 0x5f);
        _X(COUNT, *++S = T + 1; T = BGET(T));
        _X(ULESS, T = BOOL((U16)*S-- < (U16)T));
        _X(UMMOD, _ummod());              /// (udl udh u -- ur uq) unsigned divide of a double by single
        _X(UMSTAR,                        /// (u1 u2 -- ud) unsigned multiply return double product
            U32 u = (U32)*S * T;
            DTOP(u));
        _X(MSTAR,                         /// (n1 n2 -- d) signed multiply, return double product
            S32 d = (S32)*S * T;
            DTOP(d));
        _X(UMPLUS,                        /// ( n1 n2 -- sum c ) return sum of two numbers and carry flag
            U32 u = (U32)*S + T;
            DTOP(u));
        _X(SSMOD,                         /// ( dl dh n -- r q ) double div/mod by a single
            S32 d = (S32)*S * *(S - 1);
            *--S  = (DU)(d % T);
            T     = (DU)(d / T));
        _X(SMOD,                          /// ( n1 n2 -- r q )
            DU s = *S;
            *S = s % T;
            T  = s / T);
        _X(MSLAS,
            S32 d = (S32)*S-- * *S--;     /// ( n1 n2 n3 -- q ) multiply n1 n2, divided by n3 return quotient
            T = (DU)(d / T));
        _X(S2D,   S32 d = (S32)T; S++; DTOP(d));
        _X(D2S,
           DU s = *S--;
           T = (T < 0) ? -abs(s) : abs(s));
        /// @}
        /// @name Double precision ops
        /// @{
        _X(DNEG,                          /// (d -- -d) two's complemente of T double
            S32 d = S2D(T, *S);
            DTOP(-d));
        _X(DADD,                          /// (d1 d2 -- d1+d2) add two double precision numbers
            S32 d0 = S2D(T, *S);
            S32 d1 = S2D(*(S-1), *(S-2));
            S -= 2; DTOP(d1 + d0));
        _X(DSUB,                          /// (d1 d2 -- d1-d2) subtract d2 from d1
            S32 d0 = S2D(T, *S);
            S32 d1 = S2D(*(S-1), *(S-2));
            S -= 2; DTOP(d1 - d0));
        /// TODO: add J
        _X(SPAT,
            DU r = (U8*)S - (U8*)RAM(FORTH_STACK_ADDR);
            PUSH(FORTH_STACK_ADDR + r));
#if EXE_TRACE
        _X(TRC,  tCNT = T; POP());
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
            _ccall());                     /// * call C function
        _X(CLK,
            U32 t = millis();
            *++S  = T; S++;                /// * allocate 2-cells for clock ticks
            DTOP(t));
        /// @}
        /// @name Arduino specific ops
        /// @{
        _X(PIN,
            pinMode(T, *S-- ? OUTPUT : INPUT);
            POP());
        _X(MAP,
            U16 tmp = map(T, *(S-3), *(S-2), *(S-1), *S);
            S -= 4;
            T = tmp);
        _X(IN,    T = digitalRead(T));
        _X(OUT,   _out(T, *S);   S--; POP());
        _X(AIN,   T = analogRead(T));
        _X(PWM,   analogWrite(T, *S);    S--; POP());
        _X(TMISR, intr_add_tmisr(T, *S, *(S-1)); S-=2; POP());
        _X(PCISR, intr_add_pcisr(T, *S); S--; POP());
        _X(TMRE,  intr_timer_enable(T);   POP());
        _X(PCIE,  intr_pci_enable(T);     POP());
        }
    }
}
