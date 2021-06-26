#include "eforth_core.h"

static Stream *io;
//
// Forth VM control registers
//
XA  PC, IP;                     // PC (program counter), IP (instruction pointer)
U8  R, S;                       // return stack index, data stack index
S16 top;                        // ALU (i.e. cached top of stack value)
//
// Forth VM core storage
//
PGM_P cRom;
U8    *cData;             		// linear byte array pointer
S16   *cStack;                  // pointer to stack/rack memory block
//
// data and return stack ops
//         S                   R
//         |                   |
// top -> [S0, S1, S2..., R1, R0]
//
// Dr. Ting uses 256 and U8 for wrap-around control
//
#define RAM_FLAG       0xf000
#define OFF_MASK       0x0fff
#define BOOL(f)        ((f) ? TRUE : FALSE)

U8   BGET(U16 d)       {
	return (U8)((d&RAM_FLAG) ? cData[d&OFF_MASK] : pgm_read_byte(cRom+d));
}
#define BSET(d, c)     (cData[(d)&OFF_MASK]=(U8)(c))
#define SET(d, v)      (*((S16*)&cData[(d)&OFF_MASK])=(v))
U16  GET(U16 d)        {
    return (d&RAM_FLAG)
        ? *((U16*)&cData[d&OFF_MASK])
        : (U16)pgm_read_byte(cRom+d) + ((U16)pgm_read_byte(cRom+d+1)<<8);
}
#define S_GET(s)       (cStack[s])
#define S_SET(s, v)    (cStack[s]=(S16)(v))
#define RS_TOP         (FORTH_STACK_SZ>>1)
#define R_GET(r)       ((XA)cStack[RS_TOP - (r)])
#define R_SET(r, v)    (cStack[RS_TOP - (r)]=(S16)(v))

void PUSH(S16 v)       { S_SET(++S, top); top = v;  }
#define	POP()          (top=S_GET(S ? S-- : S))
#define RPUSH(v)       (R_SET(++R, (v)))
#define RPOP()         (R_GET(R ? R-- : R))
void DTOP(S32 d) {
	S_SET(S, d&0xffff);
	top = d>>16;
}

void NEXT()            { PC=GET(IP); IP+=sizeof(XA); }
//
// tracing instrumentation
//
int tTAB, tCNT;		// trace indentation and depth counters

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
void TRACE_WORD()
{
	if (!tCNT) return;
	if (!PC || BGET(PC)==opEXIT) return;
	XA pc = PC-1;
	for (; (BGET(pc) & 0x7f)>0x20; pc--);  // retract pointer to word name (ASCII range: 0x21~0x7f)

	for (int s=(S>=3 ? S-3 : 0), s0=s; s<S; s++) {
        if (s==s0) { LOG_H(" ", S_GET(s+1)); } else { LOG_H("_", S_GET(s+1)); }
	}
    if (S==0) { LOG_H(" ", top); } else { LOG_H("_", top); }
    LOG("_");
	int len = BGET(pc++) & 0x1f;          // Forth allows 31 char max
	for (int i=0; i<len; i++, pc++) {
		LOG_C((char)BGET(pc));
	}
}
//
// Forth Virtual Machine (primitive functions)
//
void _nop() { NEXT(); }    // ( -- )
void _init() {
	R = S = PC = IP = top = 0;
	tCNT = tTAB = 0;
    
	XA pc = FORTH_UVAR_ADDR;
	SET(pc,   FORTH_TIB_ADDR);
	SET(pc+2, 0x10);
	SET(pc+4, FORTH_DIC_ADDR);
    
#if EXE_TRACE
    tCNT=1;
#endif // EXE_TRACE

    ef_prompt();
}
void _qrx()                 // ( -- c t|f) read a char from terminal input device
{
	PUSH(ef_getchar());     // yield to user task until console input available
	if (top) PUSH(TRUE);
    NEXT();
}
void _txsto()               // (c -- ) send a char to console
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
void _dovar()               // ( -- a) return address of a variable
{
    ++PC;                   // skip opDOVAR opcode
	PUSH(PC);               // push variable address onto stack
	NEXT();
}
void _docon()               // ( -- n) push next token onto data stack as constant
{
    ++PC;                   // skip opDOCON opcode
 	PUSH(GET(PC));          // push cell value onto stack
    NEXT();
}
void _dolit()               // ( -- w) push next token as an integer literal
{
	TRACE(" ", GET(IP));    // fetch literal from data
	PUSH(GET(IP));	        // push onto data stack
	IP += CELLSZ;			// skip to next instruction
    NEXT();
}
void _enter()               // ( -- ) push instruction pointer onto return stack and pop, aka DOLIST by Dr. Ting
{
	TRACE_COLON();
	RPUSH(IP);              // keep return address
	IP = ++PC;              // skip opcode opENTER, advance to next instruction
    NEXT();
}
void __exit()               // ( -- ) terminate all token lists in colon words
{
	TRACE_EXIT();
	IP = RPOP();            // pop return address
    NEXT();
}
void _execu()               // (a -- ) take execution address from data stack and execute the token
{
	PC = (XA)top;           // fetch program counter
	POP();
}
void _donext()              // ( -- ) terminate a FOR-NEXT loop
{
    XA i = R_GET(R);        // loop counter
	if (i) {			    // check if loop counter > 0
		R_SET(R, i-1);		// decrement loop counter
		IP = GET(IP);		// branch back to FOR
	}
	else {
		IP += CELLSZ;		// skip to next instruction
		RPOP();				// pop off return stack
	}
    NEXT();
}
void _qbran()               // (f -- ) test top as a flag on data stack
{
	if (top) IP += CELLSZ;	// next instruction
    else     IP = GET(IP);	// fetch branching target address
	POP();
    NEXT();
}
void _bran()                // ( -- ) branch to address following
{
	IP = GET(IP);			// fetch branching target address
    NEXT();
}
void _store()               // (n a -- ) store into memory location from top of stack
{
	SET(top, S_GET(S--));
	POP();
    NEXT();
}
void _at()                  // (a -- n) fetch from memory address onto top of stack
{
	top = (S16)GET(top);
    NEXT();
}
void _cstor()               // (c b -- ) store a byte into memory location
{
	BSET(top, S_GET(S--));
	POP();
    NEXT();
}
void _cat()                 // (b -- n) fetch a byte from memory location
{
	top = (S16)BGET(top);
    NEXT();
}
void _onep()
{
    top++;
    NEXT();
}
void _onem()
{
    top--;
    NEXT();
}
void _rfrom()               // (-- w) pop from return stack onto data stack (Ting comments different ???)
{
	PUSH(RPOP());
    NEXT();
}
void _rat()                 // (-- w) copy a number off the return stack and push onto data stack
{
	PUSH(R_GET(R));
    NEXT();
}
void _tor()                 // (w --) pop from data stack and push onto return stack
{
	RPUSH(top);
	POP();
    NEXT();
}
void _depth()
{
    PUSH(S);
    NEXT();
}
void _delay()
{
    U32 t = POP();
    ef_wait(t);
    NEXT();
}
void _clock()               // ( -- d) arduino millis() as double
{
	U32 t = millis();
	PUSH(t & 0xffff);
	PUSH(t >> 16);
    NEXT();
}
void _drop()                // (w -- ) drop top of stack item
{
	POP();
    NEXT();
}
void _dup()                 // (w -- w w) duplicate to of stack
{
	S_SET(++S, top);
    NEXT();
}
void _swap()                // (w1 w2 -- w2 w1) swap top two items on the data stack
{
	S16 tmp  = top;
	top = S_GET(S);
	S_SET(S, tmp);
    NEXT();
}
void _over()                // (w1 w2 -- w1 w2 w1) copy second stack item to top
{
	PUSH(S_GET(S));			// push w1
	NEXT();
}
void _zless()               // (n -- f) check whether top of stack is negative
{
	top = BOOL(top < 0);
    NEXT();
}
void _and()                 // (w w -- w) bitwise AND
{
	top &= S_GET(S--);
    NEXT();
}
void _or()                  // (w w -- w) bitwise OR
{
	top |= S_GET(S--);
    NEXT();
}
void _xor()                 // (w w -- w) bitwise XOR
{
	top ^= S_GET(S--);
	NEXT();
}
void _uplus()               // (w w -- w c) add two numbers, return the sum and carry flag
{
	S_SET(S, S_GET(S)+top);
	top = (U16)S_GET(S) < (U16)top;
    NEXT();
}
void _qdup()                // (w -- w w | 0) dup top of stack if it is not zero
{
	if (top) S_SET(++S, top);
    NEXT();
}
void _rot()                 // (w1 w2 w3 -- w2 w3 w1) rotate 3rd item to top
{
	S16 tmp = S_GET(S-1);
	S_SET(S-1, S_GET(S));
	S_SET(S, top);
	top = tmp;
    NEXT();
}
void _lshift()              // (w n -- w) left shift n bits
{
	top = S_GET(S--) << top;
    NEXT();
}
void _rshift()              // (w n -- w) right shift n bits
{
    top = S_GET(S--) >> top;
    NEXT();
}
/* deprecated
void _ddrop()               // (w w --) drop top two items
{
	POP();
	POP();
    NEXT();
}
void _ddup()                // (w1 w2 -- w1 w2 w1 w2) duplicate top two items
{
	PUSH(S_GET(S-1));
	PUSH(S_GET(S-1));
    NEXT();
}
*/
void _plus()                // (w w -- sum) add top two items
{
	top += S_GET(S--);
    NEXT();
}
void _invert()             // (w -- w) one's complement
{
	top = -top - 1;
    NEXT();
}
void _negate()             // (n -- -n) two's complement
{
	top = 0 - top;
    NEXT();
}
void _great()               // (n1 n2 -- t) true if n1>n2
{
	top = BOOL(S_GET(S--) > top);
    NEXT();
}
void _sub()                 // (n1 n2 -- n1-n2) subtraction
{
	top = S_GET(S--) - top;
    NEXT();
}
void _abs()                 // (n -- n) absolute value of n
{
    U16 m = top>>15;
    top = (top + m) ^ m;    // no branching
    NEXT();
}
void _less()                // (n1 n2 -- t) true if n1<n2
{
	top = BOOL(S_GET(S--) < top);
    NEXT();
}
void _equal()               // (w w -- t) true if top two items are equal
{
	top = BOOL(S_GET(S--)==top);
    NEXT();
}
void _uless()               // (u1 u2 -- t) unsigned compare top two items
{
	top = BOOL((U16)(S_GET(S--)) < (U16)top);
    NEXT();
}
void _ummod()               // (udl udh u -- ur uq) unsigned divide of a double by single
{
	U32 d = (U32)top;       // CC: auto variable uses C stack 
 	U32 m = ((U32)S_GET(S)<<16) + (U16)S_GET(S-1);
	POP();
 	S_SET(S, (S16)(m % d)); // remainder
 	top   = (S16)(m / d);   // quotient
    NEXT();
}
void _pinmode()             // (pin mode --) arduino pinMode(pin, mode)
{
    pinMode(S_GET(S), top ? INPUT : OUTPUT);
    POP();
    POP();
    NEXT();
}
void _map()                 // (f1 f2 t1 t2 n -- nx) arduino map(n, f1, f2, t1, t2)
{
    U16 tmp = map(top, S_GET(S-3), S_GET(S-2), S_GET(S-1), S_GET(S));
    S -= 4;
    top = tmp;
    NEXT();
}
/* deprecated
void _msmod()               // (d n -- r q) signed floored divide of double by single
{
	S32 d = (S32)top;
 	S32 m = ((S32)S_GET(S)<<16) + S_GET(S-1);
	POP();
	S_SET(S, (S16)(m % d)); // remainder
	top   = (S16)(m / d);   // quotient
    NEXT();
}
void _slmod()               // (n1 n2 -- r q) signed devide, return mod and quotient
{
	if (top) {
		S16 tmp = S_GET(S) / top;
		S_SET(S, S_GET(S) % top);
		top = tmp;
	}
    NEXT();
}
*/
void _mod()                 // (n n -- r) signed divide, returns mod
{
	top = (top) ? S_GET(S--) % top : S_GET(S--);
    NEXT();
}
void _slash()               // (n n - q) signed divide, return quotient
{
	top = (top) ? S_GET(S--) / top : (S_GET(S--), 0);
    NEXT();
}
void _umstar()              // (u1 u2 -- ud) unsigned multiply return double product
{
 	U32 u = (U32)S_GET(S) * top;
 	S_SET(S, (U16)(u & 0xffff));
 	top = (U16)(u >> 16);
    NEXT();
}
void _star()                // (n n -- n) signed multiply, return single product
{
	top *= S_GET(S--);
    NEXT();
}
void _mstar()               // (n1 n2 -- d) signed multiply, return double product
{
 	S32 d = (S32)S_GET(S) * top;
 	DTOP(d);
    NEXT();
}
void _din()                 // (pin -- n) read from arduino pin
{
    PUSH(digitalRead(POP()));
    NEXT();
}
void _dout()                // (pin n -- ) output to arduino pin
{
    digitalWrite(top, S_GET(S));
    POP();
    POP();
    NEXT();
}
/* deprecated
void _ssmod()               // (n1 n2 n3 -- r q) n1*n2/n3, return mod and quotion
{
	S32 m = (S32)S_GET(S-1) * S_GET(S);
	S16 d = top;
	POP();
	S_SET(S, (S16)(m % d));
	top   = (S16)(m / d);
    NEXT();
}
void _stasl()               // (n1 n2 n3 -- q) n1*n2/n3 return quotient
{
	S32 m = (S32)S_GET(S-1) * S_GET(S);
    S16 d = top;
	POP();
    POP();
	top = (S16)(m / d);
    NEXT();
}
*/
void _pick()                // (... +n -- ...w) copy nth stack item to top
{
	top = S_GET(S - (U8)top);
    NEXT();
}
void _pstor()               // (n a -- ) add n to content at address a
{
	SET(top, GET(top)+S_GET(S--));
    POP();
    NEXT();
}
void _ain()                 // (pin -- n) read from arduino analog pin
{
    PUSH(analogRead(POP()));
    NEXT();
}
void _aout()                // (pin n -- ) write PWM to arduino analog pin
{
    analogWrite(top, S_GET(S));
    POP();
    POP();
    NEXT();
}

/* deprecated
void _dstor()               // (d a -- ) store the double to address a
{
	SET(top+CELLSZ, S_GET(S--));
	SET(top,        S_GET(S--));
	POP();
    NEXT();
}
void _dat()                 // (a -- d) fetch double from address a
{
	PUSH(GET(top));
	top = GET(top + CELLSZ);
    NEXT();
}
void _count()               // (b -- b+1 +n) count byte of a string and add 1 to byte address
{
	S_SET(++S, top + 1);
	top = (S16)BGET(top);
    NEXT();
}
void _max_()                // (n1 n2 -- n) return greater of two top stack items
{
	if (top < S_GET(S)) POP();
	else (U8)S--;
    NEXT();
}
void _min_()                // (n1 n2 -- n) return smaller of two top stack items
{
	if (top < S_GET(S)) S--;
	else POP();
    NEXT();
}
*/

void _dnegate()             // (d -- -d) two's complemente of top double
{
	S32 d = ((S32)top<<16) | S_GET(S)&0xffff;
    DTOP(-d);
    NEXT();
}

void _dplus()
{
    S32 d0 = ((S32)top<<16)        | (S_GET(S)&0xffff);
    S32 d1 = ((S32)S_GET(S-1)<<16) | (S_GET(S-2)&0xffff);
    S -= 2;
    DTOP(d1 + d0);
    NEXT();
}

void _dsub()
{
    S32 d0 = ((S32)top<<16)        | (S_GET(S)&0xffff);
    S32 d1 = ((S32)S_GET(S-1)<<16) | (S_GET(S-2)&0xffff);
    S -= 2;
    DTOP(d1 - d0);
    NEXT();
}

void(*prim[FORTH_PRIMITIVES])() = {
	/* case 0 */ _nop,
	/* case 1 */ _init,
	/* case 2 */ _qrx,
	/* case 3 */ _txsto,
	/* case 4 */ _docon,
	/* case 5 */ _dolit,
	/* case 6 */ _enter,
	/* case 7 */ __exit,
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
//
// Forth internal (user) variables
//
//   'TIB    = FORTH_TIB_ADDR (pointer to input buffer)
//   BASE    = 0x10           (numerical base 0xa for decimal, 0x10 for hex)
//   CP      = here           (pointer to top of dictionary, first memory location to add new word)
//   CONTEXT = last           (pointer to name field of the most recently defined word in dictionary)
//   LAST    = last           (pointer to name field of last word in dictionary)
//   'EVAL   = INTER          ($COMPILE for compiler or $INTERPRET for interpreter)
//   ABORT   = QUIT           (pointer to error handler, QUIT is the main loop)
//   tmp     = 0              (scratch pad)
//
void vm_init(PGM_P rom, U8 *cdata, void *io_stream) {
    io    = (Stream *)io_stream;
    cRom  = rom;
	cData = cdata;
    cStack= (S16*)&cdata[FORTH_STACK_ADDR - FORTH_RAM_ADDR];
    
    _init();                   // resetting user variables
}
    
int vm_step() {
    TRACE_WORD();              // tracing stack and word name
    prim[BGET(PC)]();          // walk bytecode stream

    return (int)PC;
}

