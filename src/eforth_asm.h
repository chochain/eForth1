/**
 * @file
 * @brief eForth assembler module header
 *
 * Usage of VA_ARGS variable argument for assembler parameter counting
 */
#ifndef __EFORTH_ASM_H
#define __EFORTH_ASM_H
#include "eforth_config.h"

#define ASM_TRACE       0          /** create assembler trace */
#define CASE_SENSITIVE  0          /** enable case sensitive  */
#define ENABLE_SEE      1          /** add SEE, +280 bytes    */
///
/// Name field
/// +----------+-------------+
/// | len byte | name string |
/// +----------+-------------+
/// Forth name len max 31
/// the following flags are used to flag word attributes
///
#define fCMPL           0x40       /**< compile only flag */
#define fIMMD           0x80       /**< immediate flag    */
///
/// define opcode enums
/// Note: in sync with VM's vtable
///
#define OP(name)        op##name
enum {
    opNOP = 0,                      ///< opcodes start at 0
    OPCODES
};
//
// tracing/logging macros
//
#if ASM_TRACE
#define DEBUG(s,v)      printf(s, v)
#define SHOWOP(op)      printf("\n%04x: _%s\t", PC, op)
#else
#define DEBUG(s,v)
#define SHOWOP(op)
#endif // ASM_TRACE
///
/// variable length parameter handler macros
///
#define _ARG_N(                                            \
          _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, \
         _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
         _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
         _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
         _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
         _51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
         _61, _62, _63, N, ...) N
#define _NUM_N()                                           \
         62, 61, 60,                                       \
         59, 58, 57, 56, 55, 54, 53, 52, 51, 50,           \
         49, 48, 47, 46, 45, 44, 43, 42, 41, 40,           \
         39, 38, 37, 36, 35, 34, 33, 32, 31, 30,           \
         29, 28, 27, 26, 25, 24, 23, 22, 21, 20,           \
         19, 18, 17, 16, 15, 14, 13, 12, 11, 10,           \
          9,  8,  7,  6,  5,  4,  3,  2,  1,  0
#define _NARG0(...)          _ARG_N(__VA_ARGS__)
#define _NARG(...)           _NARG0(_, ##__VA_ARGS__, _NUM_N())

///
///@name Vargs Header (calculate number of parameters by compiler)
///@{
#define _CODE(seg, ...)      _code(seg, _NARG(__VA_ARGS__), __VA_ARGS__)
#define _PRIM(seg, x)        (_CODE(seg, op##x, opEXIT), op##x)
#define _COLON(seg, ...)     _colon(seg, _NARG(__VA_ARGS__), __VA_ARGS__)
#define _IMMED(seg, ...)     _immed(seg, _NARG(__VA_ARGS__), __VA_ARGS__)
#define _LABEL(...)          _label(_NARG(__VA_ARGS__), __VA_ARGS__)
///@}
///@name Vargs Branching
///@{
#define _BEGIN(...)          _begin(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _AGAIN(...)          _again(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _UNTIL(...)          _until(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _WHILE(...)          _while(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _REPEAT(...)         _repeat(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _IF(...)             _if(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _ELSE(...)           _else(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _THEN(...)           _then(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _FOR(...)            _for(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _NEXT(...)           _nxt(_NARG(__VA_ARGS__), __VA_ARGS__)
#define _AFT(...)            _aft(_NARG(__VA_ARGS__), __VA_ARGS__)
///@}
///@name Vargs IO
///@{
#define _DOTQ(str)           _dotq(str)
///@}
///@name Memory Access and Stack Op
///@{
#define BSET(d, c)  (_byte[d]=(U8)(c))
#define BGET(d)     (_byte[d])
#define SET(d, v)   do { U16 a=(d); U16 x=(v); BSET(a,(x)>>8); BSET((a)+1,(x)&0xff); } while (0)
#define GET(d)      ({ U16 a=(d); ((U16)BGET(a)<<8) | BGET((a)+1); })
#define STORE(v)    do { SET(PC,(v)); PC+=CELLSZ; } while(0)
#define RPUSH(a)    SET(FORTH_ROM_SZ - (++R)*CELLSZ, (a))             /** tail section of memory block */
#define RPOP()      ((U16)GET(FORTH_ROM_SZ - (R ? R-- : R)*CELLSZ))
#define VL(a, i)    (((U16)(a)+CELLSZ*(i))&0xff)
#define VH(a, i)    (((U16)(a)+CELLSZ*(i))>>8)
#define VAL(a, i)   opDOLIT,VH(a,i),VL(a,i),opEXIT
///@}
///@defgroup Memory copy
///@{
#define CELLCPY(n) {                            \
    va_list argList;                            \
    va_start(argList, n);                       \
    int lit=0;                                  \
    for (; n; n--) {                            \
        IU j = (IU)va_arg(argList, int);        \
        if (lit) {      /** literal */          \
            STORE(j);                           \
            lit = 0;                            \
            DEBUG(" %04x", j);                  \
            continue;                           \
        }                                       \
        if (j < 0x80) {  /** max 128 opcode */  \
            lit = (j==opDOLIT);                 \
            if (j != opNOP) {                   \
               BSET(PC++, j);                   \
               DEBUG(" %02x", j);               \
            }                                   \
        }                                       \
        else {          /** colon words */      \
          STORE(j | fCOLON16);                  \
          DEBUG(" %04x", j);                    \
        }                                       \
    }                                           \
    va_end(argList);                            \
    _rdump();                                   \
}

#define MEMCPY(len, seq) {                      \
    memcpy(&_byte[PC], seq, len);               \
    PC += len;                                  \
}

#define OPSTR(ip, seq) {                        \
    SET(PC, ip | fCOLON16);                     \
    PC += CELLSZ;                               \
    int len = strlen(seq);                      \
    BSET(PC++, len);                            \
    MEMCPY(len, seq);                           \
}
///@}
///@defgroup Assembler Module variables
///@{
extern IU PC;          ///< assembler program counter
extern U8 R;           ///< assembler return stack index
extern IU _link;       ///< link to previous word
extern U8 *_byte;      ///< assembler byte array (heap)
extern IU DOTQP;       ///< addr of output function _dotq
///
///@defgroup Pseudo macros (to handle va_list)
///@brief - keeping functions in the header is considered a bad practice!
///       - however, since eforth_asm.cpp is the only file include this header
///       - by keeping them here making the assembler code cleaner.
///@{
///
///> Memory Dumper helpers
///
void _dump(int b, int u) {               ///> dump memory between previous word and this
    DEBUG("%s", "\n    :");
    for (int i=b; i<u; i+=sizeof(IU)) {
        if ((i+1)<u) DEBUG(" %04x", GET(i));
        else         DEBUG(" %02x", BGET(i));
    }
}
void _rdump()                            ///> dump return stack
{
    DEBUG("%cR[", ' ');
    for (int i=1; i<=R; i++) {
        DEBUG(" %04x", GET(FORTH_ROM_SZ - i*CELLSZ));
    }
    DEBUG("%c]", ' ');
}
///
///> create a word hearder
///
void _header(int lex, const char *seq) { /// create a word header in dictionary
    if (_link) {
        if (PC >= FORTH_ROM_SZ) DEBUG("ROM %s", "max!");
        _dump(_link - sizeof(IU), PC);   /// * dump data from previous word to current word
    }
    STORE(_link);                        /// * point to previous word
    _link = PC;                          /// * keep pointer to this word

    BSET(PC++, lex);                     /// * length of word (with optional fIMMED or fCOMPO flags)
    int len = lex & 0x1f;                /// * Forth allows word max length 31
    
    MEMCPY(len, seq);                    /// * memcpy word string
    DEBUG("\n%04x: ", PC);
    DEBUG("%s", seq);
}
///
///> create an opcode stream for built-in word
///
int _code(const char *seg, int len, ...) {
    _header(strlen(seg), seg);
    int addr = PC;                       ///< keep address of current word
    va_list argList;
    va_start(argList, len);
    for (; len; len--) {                 /// * copy bytecodes
        U8 b = (U8)va_arg(argList, int);
        BSET(PC++, b);
        DEBUG(" %02x", b);
    }
    va_end(argList);
    return addr;                         /// address to be kept in local var
}
///
///> create a colon word
///
int _colon(const char *seg, int len, ...) {
    _header(strlen(seg), seg);
    DEBUG(" %s", ":_COLON");
    int addr = PC;
    CELLCPY(len);
    return addr;
}
///
/// create a immediate word
///
int _immed(const char *seg, int len, ...) {
    _header(fIMMD | strlen(seg), seg);
    SHOWOP("IMMD");
    int addr = PC;
    CELLCPY(len);
    return addr;
}
///
/// create a label
///
int _label(int len, ...) {
    SHOWOP("LABEL");
    int addr = PC;
    // label has no opcode here
    CELLCPY(len);
    return addr;
}
///
///> Branching Ops
///
void _begin(int len, ...) {        /// **BEGIN**-(once)-WHILE-(loop)-UNTIL/REPEAT
    SHOWOP("BEGIN");               /// **BEGIN**-AGAIN
    RPUSH(PC);                     /// * keep current address for looping
    CELLCPY(len);
}
void _while(int len, ...) {        /// BEGIN-(once)--**WHILE**-(loop)-UNTIL/REPEAT
    SHOWOP("WHILE");
    BSET(PC++, opQBRAN);
    STORE(0);                      /// * branching address
    int k = RPOP();
    RPUSH(PC - CELLSZ);
    RPUSH(k);
    CELLCPY(len);
}
void _repeat(int len, ...) {       /// BEGIN-(once)-WHILE-(loop)- **REPEAT**
    SHOWOP("REPEAT");
    BSET(PC++, opBRAN);
    STORE(RPOP());
    SET(RPOP(), PC);
    CELLCPY(len);
}
void _until(int len, ...) {        /// BEGIN-(once)-WHILE-(loop)--**UNTIL**
    SHOWOP("UNTIL");
    BSET(PC++, opQBRAN);           /// * conditional branch
    STORE(RPOP());                 /// * loop begin address
    CELLCPY(len);
}
void _again(int len, ...) {        /// BEGIN--**AGAIN**
    SHOWOP("AGAIN");
    BSET(PC++, opBRAN);            /// * unconditional branch
    STORE(RPOP());                 /// * store return address
    CELLCPY(len);
}
void _for(int len, ...) {          /// **FOR**-(first)-AFT-(2nd,...)-THEN-(every)-NEXT
    SHOWOP("FOR");
    BSET(PC++, opTOR);             /// * put loop counter on return stack
    RPUSH(PC);                     /// * keep 1st loop repeat address A0
    CELLCPY(len);
}
void _aft(int len, ...) {          /// FOR-(first)--**AFT**-(2nd,...)-THEN-(every)-NEXT
    SHOWOP("AFT");                 /// * code between FOR-AFT run only once
    BSET(PC++, opBRAN);            /// * unconditional branch
    STORE(0);                      /// * forward jump address (A1)NOP,
    RPOP();                        /// * pop-off A0 (FOR-AFT once only)
    RPUSH(PC);                     /// * keep repeat address on return stack
    RPUSH(PC - CELLSZ);            /// * keep A1 address on return stack for AFT-THEN
    CELLCPY(len);
}
///
///> Note: _next() is multi-defined in vm
//
void _nxt(int len, ...) {          /// FOR-(first)-AFT-(2nd,...)-THEN-(every)--**NEXT**
    SHOWOP("NEXT");
    BSET(PC++, opDONEXT);          /// * check loop counter (on return stack)
    STORE(RPOP());                 /// * add A0 (FOR-NEXT) or
    CELLCPY(len);                  /// * A1 to repeat loop (conditional branch by DONXT)
}
void _if(int len, ...) {           /// **IF**-THEN, **IF**-ELSE-THEN
    SHOWOP("IF");
    BSET(PC++, opQBRAN);           /// * conditional branch
    RPUSH(PC);                     /// * keep A0 address on return stack for ELSE or THEN
    STORE(0);                      /// * reserve branching address (A0)
    CELLCPY(len);
}
void _else(int len, ...) {         /// IF--**ELSE**-THEN
    SHOWOP("ELSE");
    BSET(PC++, opBRAN);            /// * unconditional branch
    STORE(0);                      /// * reserve branching address (A1)
    SET(RPOP(), PC);               /// * backfill A0 branching address
    RPUSH(PC - CELLSZ);            /// * keep A1 address on return stack for THEN
    CELLCPY(len);
}
void _then(int len, ...) {         /// IF-ELSE--**THEN**
    SHOWOP("THEN");
    SET(RPOP(), PC);               /// * backfill branching address (A0) or (A1)
    CELLCPY(len);
}
///
///> IO Functions
///
void _dotq(const char *seq) {
    SHOWOP("DOTQ");
    DEBUG("%s", seq);
    OPSTR(DOTQP, seq);
}
///@}
#endif // __EFORTH_ASM_H
