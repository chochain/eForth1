/**
 * @file eforth_asm.h
 * @brief eForth assember module header
 *
 * Usage of VA_ARGS veriable argument for assembler parameter counting
 */
#ifndef __EFORTH_SRC_EFORTH_ASM_H
#define __EFORTH_SRC_EFORTH_ASM_H

namespace EfAsm {
//
// tracing/logging macros
//
#if ASM_TRACE
#define DEBUG(s,v)      printf(s, v)
#define SHOWOP(op)      printf("\n%04x: %s\t", aPC, op)
#else
#define DEBUG(s,v)
#define SHOWOP(op)
#endif // ASM_TRACE
#if ARDUINO
typedef const __FlashStringHelper FCHAR;
#else
#define F(s)                      (s)
typedef const char                FCHAR;
#endif // ARDUINO
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
#define _CODE(seg, ...)      _code(F(seg), _NARG(__VA_ARGS__), __VA_ARGS__)
#define _COLON(seg, ...)     _colon(F(seg), _NARG(__VA_ARGS__), __VA_ARGS__)
#define _IMMED(seg, ...)     _immed(F(seg), _NARG(__VA_ARGS__), __VA_ARGS__)
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
#define _DOTQ(seq)           _dotq(F(seq))
#define _STRQ(seq)           _strq(F(seq))
#define _ABORTQ(seq)         _abortq(F(seq))
///@}
///@name Memory Access and Stack Op
///@{
#define BSET(d, c)  (aByte[d]=(U8)(c))
#define BGET(d)     (aByte[d])
#define SET(d, v)   do { U16 a=(d); U16 x=(v); BSET(a, (x)&0xff); BSET((a)+1, (x)>>8); } while (0)
#define GET(d)      ({ U16 a=(d); (U16)BGET(a) + ((U16)BGET((a)+1)<<8); })
#define STORE(v)    do { SET(aPC, (v)); aPC+=CELLSZ; } while(0)
#define RPUSH(a)    SET(FORTH_ROM_SZ - (++aR)*CELLSZ, (a))
#define RPOP()      ((U16)GET(FORTH_ROM_SZ - (aR ? aR-- : aR)*CELLSZ))
#define VL(a, i)    (((U16)(a)+CELLSZ*(i))&0xff)
#define VH(a, i)    (((U16)(a)+CELLSZ*(i))>>8)
#define V32(a, i)   VL(a,i),VH(a,i)
///@}
///

};  // namespace EfAsm
#endif // __EFORTH_SRC_EFORTH_ASM_H
