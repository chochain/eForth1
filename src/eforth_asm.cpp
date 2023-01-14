/**
 * @file eforth_asm.cpp
 * @brief eForth Assembler module
 *
 * Forth Macro Assembler
 *
 */
#include "eforth_core.h"
#include "eforth_asm.h"

#define fCMPL          0x40         /**< compile only flag */
#define fIMMD          0x80         /**< immediate flag    */
#define DUMP_ROW_WIDTH 0x40
///
/// define opcode enums
/// Note: in sync with VM's vtable
///
#define OP(name)  op##name
enum {
    opNOP = 0,                      ///< opcodes start at 0
    OPCODES
};

namespace EfAsm {
///
///@name Compiled Address for Branching
///@{
IU BRAN, QBRAN, DONXT;
IU DOTQ, STRQ, ABORTQ;
IU TOR;
IU NOP = 0xffff;                    ///< NOP set to ffff to prevent access before initialized
///@}
///@name Return Stack for Branching Ops
///@{
U8 *aByte;                          ///< assember byte array (heap)
U8 aR;                              ///< assember return stack index
IU aPC;                             ///< assembler program counter
IU aLink;                           ///< link to previous word
///@}
///@name Memory Dumpers
///@{
void _dump(int b, int u) {      /// dump memory between previous word and this
    DEBUG("%s", "\n    :");
    for (int i=b; i<u; i+=sizeof(IU)) {
        if ((i+1)<u) DEBUG(" %04x", GET(i));
        else         DEBUG(" %02x", BGET(i));
    }
}
void _rdump()                   /// dump return stack
{
    DEBUG("%cR[", ' ');
    for (int i=1; i<=aR; i++) {
        DEBUG(" %04x", GET(FORTH_ROM_SZ - i*CELLSZ));
    }
    DEBUG("%c]", ' ');
}
///@}
///@name Assembler - String/Byte Movers
///@{
int _strlen(FCHAR *seq) {      /// string length (in Arduino Flash memory block)
    PGM_P p = reinterpret_cast<PGM_P>(seq);
    int i=0;
    for (; pgm_read_byte(p); i++, p++);
    return i;
}
#define CELLCPY(n) {                            \
    va_list argList;                            \
    va_start(argList, n);                       \
    for (; n; n--) {                            \
        IU j = (IU)va_arg(argList, int);        \
        if (j==NOP) continue;                   \
        STORE(j);                               \
        DEBUG(" %04x", j);                      \
    }                                           \
    va_end(argList);                            \
    _rdump();                                   \
}
#define STRCPY(op, seq) {                       \
    STORE(op);                                  \
    int len = _strlen(seq);                     \
    PGM_P p = reinterpret_cast<PGM_P>(seq);     \
    BSET(aPC++, len);                           \
    for (int i=0; i < len; i++) {               \
    BSET(aPC++, pgm_read_byte(p++));            \
    }                                           \
}
///@}
///@name Assembler - Word Creation Headers
///@{
void _header(int lex, FCHAR *seq) {           /// create a word header in dictionary
    if (aLink) {
        if (aPC >= FORTH_ROM_SZ) DEBUG("ROM %s", "max!");
        _dump(aLink - sizeof(IU), aPC);       /// * dump data from previous word to current word
    }
    STORE(aLink);                             /// * point to previous word
    aLink = aPC;                              /// * keep pointer to this word

    BSET(aPC++, lex);                         /// * length of word (with optional fIMMED or fCOMPO flags)
    int len = lex & 0x1f;                     /// * Forth allows word max length 31
    PGM_P p = reinterpret_cast<PGM_P>(seq);
    for (int i=0; i < len; i++) {             /// * memcpy word string
        BSET(aPC++, pgm_read_byte(p++));
    }
    DEBUG("\n%04x: ", aPC);
    DEBUG("%s", seq);
}
///
/// create an opcode stream for built-in word
///
int _code(FCHAR *seg, int len, ...) {
    _header(_strlen(seg), seg);
    int addr = aPC;                           /// * keep address of current word
    va_list argList;
    va_start(argList, len);
    for (; len; len--) {                      /// * copy bytecodes
        U8 b = (U8)va_arg(argList, int);
        BSET(aPC++, b);
        DEBUG(" %02x", b);
    }
    va_end(argList);
    return addr;                              /// address to be kept in local var
}
///
/// create a colon word
///
int _colon(FCHAR *seg, int len, ...) {
    _header(_strlen(seg), seg);
    DEBUG(" %s", ":06");
    int addr = aPC;
    BSET(aPC++, opENTER);
    CELLCPY(len);
    return addr;
}
///
/// create a immediate word
///
int _immed(FCHAR *seg, int len, ...) {
    _header(fIMMD | _strlen(seg), seg);
    DEBUG(" %s", "i06");
    int addr = aPC;
    BSET(aPC++, opENTER);
    CELLCPY(len);
    return addr;
}
///
/// create a label
///
int _label(int len, ...) {
    SHOWOP("LABEL");
    int addr = aPC;
    // label has no opcode here
    CELLCPY(len);
    return addr;
}
///@}
///
///@name Assembler - Branching Ops
///@{
void _begin(int len, ...) {         /// **BEGIN**-(once)-WHILE-(loop)-UNTIL/REPEAT
    SHOWOP("BEGIN");                /// **BEGIN**-AGAIN
    RPUSH(aPC);                     /// * keep current address for looping
    CELLCPY(len);
}
void _while(int len, ...) {         /// BEGIN-(once)--**WHILE**-(loop)-UNTIL/REPEAT
    SHOWOP("WHILE");
    STORE(QBRAN);
    STORE(0);                       /// * branching address
    int k = RPOP();
    RPUSH(aPC - CELLSZ);
    RPUSH(k);
    CELLCPY(len);
}
void _repeat(int len, ...) {        /// BEGIN-(once)-WHILE-(loop)- **REPEAT**
    SHOWOP("REPEAT");
    STORE(BRAN);
    STORE(RPOP());
    SET(RPOP(), aPC);
    CELLCPY(len);
}
void _until(int len, ...) {         /// BEGIN-(once)-WHILE-(loop)--**UNTIL**
    SHOWOP("UNTIL");
    STORE(QBRAN);                   /// * conditional branch
    STORE(RPOP());                  /// * loop begin address
    CELLCPY(len);
}
void _again(int len, ...) {        /// BEGIN--**AGAIN**
    SHOWOP("AGAIN");
    STORE(BRAN);                   /// * unconditional branch
    STORE(RPOP());                 /// * store return address
    CELLCPY(len);
}
void _for(int len, ...) {          /// **FOR**-(first)-AFT-(2nd,...)-THEN-(every)-NEXT
    SHOWOP("FOR");
    STORE(TOR);                    /// * put loop counter on return stack
    RPUSH(aPC);                    /// * keep 1st loop repeat address A0
    CELLCPY(len);
}
void _aft(int len, ...) {          /// FOR-(first)--**AFT**-(2nd,...)-THEN-(every)-NEXT
    SHOWOP("AFT");                 /// * code between FOR-AFT run only once
    STORE(BRAN);                   /// * unconditional branch
    STORE(0);                      /// * forward jump address (A1)NOP,
    RPOP();                        /// * pop-off A0 (FOR-AFT once only)
    RPUSH(aPC);                    /// * keep repeat address on return stack
    RPUSH(aPC - CELLSZ);           /// * keep A1 address on return stack for AFT-THEN
    CELLCPY(len);
}
//
// Note: _next() is multi-defined in vm
//
void _nxt(int len, ...) {          /// FOR-(first)-AFT-(2nd,...)-THEN-(every)--**NEXT**
    SHOWOP("NEXT");
    STORE(DONXT);                  /// * check loop counter (on return stack)
    STORE(RPOP());                 /// * add A0 (FOR-NEXT) or
    CELLCPY(len);                  /// * A1 to repeat loop (conditional branch by DONXT)
}
void _if(int len, ...) {           /// **IF**-THEN, **IF**-ELSE-THEN
    SHOWOP("IF");
    STORE(QBRAN);                  /// * conditional branch
    RPUSH(aPC);                    /// * keep A0 address on return stack for ELSE or THEN
    STORE(0);                      /// * reserve branching address (A0)
    CELLCPY(len);
}
void _else(int len, ...) {         /// IF--**ELSE**-THEN
    SHOWOP("ELSE");
    STORE(BRAN);                   /// * unconditional branch
    STORE(0);                      /// * reserve branching address (A1)
    SET(RPOP(), aPC);              /// * backfill A0 branching address
    RPUSH(aPC - CELLSZ);           /// * keep A1 address on return stack for THEN
    CELLCPY(len);
}
void _then(int len, ...) {         /// IF-ELSE--**THEN**
    SHOWOP("THEN");
    SET(RPOP(), aPC);              /// * backfill branching address (A0) or (A1)
    CELLCPY(len);
}
///@}
///
///@name Assembler - IO Functions
///@{
void _dotq(FCHAR *seq) {
    SHOWOP("DOTQ");
    DEBUG("%s", seq);
    STRCPY(DOTQ, seq);
}
void _strq(FCHAR *seq) {
    SHOWOP("STRQ");
    DEBUG("%s", seq);
    STRCPY(STRQ, seq);
}
void _abortq(FCHAR *seq) {
    SHOWOP("ABORTQ");
    DEBUG("%s", seq);
    STRCPY(ABORTQ, seq);
}
///@}
///
/// eForth Assembler
///
int assemble(U8 *cdata)
{
    aByte = cdata;
    aR    = aLink = 0;
    ///
    ///> Kernel constants
    ///
    aPC = FORTH_BOOT_ADDR;
    IU BOOT  = _LABEL(opENTER, 0);      // reserved for boot vectors

    IU ua    = FORTH_UVAR_ADDR;
    IU vTTIB = _CODE("'TIB",    opDOCON, V32(ua,0));   ///> * 'TIB console input buffer pointer
    IU vBASE = _CODE("BASE",    opDOCON, V32(ua,1));   ///> * BASE current radix for numeric ops
    IU vCP   = _CODE("CP",      opDOCON, V32(ua,2));   ///> * CP,  top of dictionary, same as HERE
    IU vCNTX = _CODE("CONTEXT", opDOCON, V32(ua,3));   ///> * CONTEXT name field of last word
    IU vLAST = _CODE("LAST",    opDOCON, V32(ua,4));   ///> * LAST, same as CONTEXT
    IU vMODE = _CODE("'MODE",   opDOCON, V32(ua,5));   ///> * 'MODE (interpreter or compiler)
    IU vTABRT= _CODE("'ABORT",  opDOCON, V32(ua,6));   ///> * ABORT exception rescue handler (QUIT)
    IU vHLD  = _CODE("HLD",     opDOCON, V32(ua,7));   ///> * HLD  char pointer to output buffer
    IU vSPAN = _CODE("SPAN",    opDOCON, V32(ua,8));   ///> * SPAN number of character accepted
    IU vIN   = _CODE(">IN",     opDOCON, V32(ua,9));   ///> * >IN  interpreter pointer to next char
    IU vNTIB = _CODE("#TIB",    opDOCON, V32(ua,10));  ///> * #TIB number of character received in TIB
    IU vTEMP = _CODE("tmp",     opDOCON, V32(ua,11));  ///> * tmp storage (alternative to return stack)
    ///
    ///> common constants and variable spec.
    ///
    IU BLANK = _CODE("BL",      opDOCON, 0x20,   0);   ///> * BL blank
    IU CELL  = _CODE("CELL",    opDOCON, CELLSZ, 0);   ///> * CELL cell size
    ///
    ///> Kernel dictionary (primitive words)
    ///
       NOP   = _CODE("NOP",     opNOP    );
    IU BYE   = _CODE("BYE",     opBYE    );
    IU QRX   = _CODE("?RX",     opQRX    );
    IU TXSTO = _CODE("TX!",     opTXSTO  );
    IU DOCON = _CODE("DOCON",   opDOCON  );
    IU DOLIT = _CODE("DOLIT",   opDOLIT  );
    IU DOVAR = _CODE("DOVAR",   opDOVAR  );
    IU DOLST = _CODE("DOLIST",  opENTER  );
    IU ENTER = _CODE("ENTER",   opENTER  );  //alias doLIST
    IU EXIT  = _CODE("EXIT",    opEXIT   );
    IU EXECU = _CODE("EXECUTE", opEXECU  );
       DONXT = _CODE("DONEXT",  opDONEXT );
       QBRAN = _CODE("QBRANCH", opQBRAN  );
       BRAN  = _CODE("BRANCH",  opBRAN   );
    IU STORE = _CODE("!",       opSTORE  );
    IU PSTOR = _CODE("+!",      opPSTOR  );
    IU AT    = _CODE("@",       opAT     );
    IU CSTOR = _CODE("C!",      opCSTOR  );
    IU CAT   = _CODE("C@",      opCAT    );
    IU RFROM = _CODE("R>",      opRFROM  );
    IU RAT   = _CODE("R@",      opRAT    );
       TOR   = _CODE(">R",      opTOR    );
    IU DROP  = _CODE("DROP",    opDROP   );
    IU DUP   = _CODE("DUP",     opDUP    );
    IU SWAP  = _CODE("SWAP",    opSWAP   );
    IU OVER  = _CODE("OVER",    opOVER   );
    IU ROT   = _CODE("ROT",     opROT    );
    IU PICK  = _CODE("PICK",    opPICK   );
    IU AND   = _CODE("AND",     opAND    );
    IU OR    = _CODE("OR",      opOR     );
    IU XOR   = _CODE("XOR",     opXOR    );
    IU INV   = _CODE("INVERT",  opINV    );
    IU LSH   = _CODE("LSHIFT",  opLSH    );
    IU RSH   = _CODE("RSHIFT",  opRSH    );
    IU ADD   = _CODE("+",       opADD    );
    IU SUB   = _CODE("-",       opSUB    );
    IU MUL   = _CODE("*",       opMUL    );
    IU DIV   = _CODE("/",       opDIV    );
    IU MOD   = _CODE("MOD",     opMOD    );
    IU NEG   = _CODE("NEGATE",  opNEG    );
    IU GT    = _CODE(">",       opGT     );
    IU EQ    = _CODE("=",       opEQ     );
    IU LT    = _CODE("<",       opLT     );
    IU ZGT   = _CODE("0>",      opZGT    );
    IU ZEQ   = _CODE("0=",      opZEQ    );
    IU ZLT   = _CODE("0<",      opZLT    );
    IU ONEP  = _CODE("1+",      opONEP   );
    IU ONEM  = _CODE("1-",      opONEM   );
    IU QDUP  = _CODE("?DUP",    opQDUP   );
    IU DEPTH = _CODE("DEPTH",   opDEPTH  );
    IU ULESS = _CODE("U<",      opULESS  );
    IU UMMOD = _CODE("UM/MOD",  opUMMOD  );
    IU UMSTA = _CODE("UM*",     opUMSTAR );
    IU MSTAR = _CODE("M*",      opMSTAR  );
    _CODE("DNEGATE", opDNEG   );
    _CODE("D+",      opDADD   );
    _CODE("D-",      opDSUB   );
    ///
    ///> Common High-Level Colon Words
    ///
    IU HERE  = _COLON("HERE",  vCP, AT, EXIT);                          // top of dictionary
    IU PAD   = _COLON("PAD",   HERE, DOLIT, FORTH_PAD_SZ, ADD, EXIT);   // use HERE for output buffer
    IU CELLP = _COLON("CELL+", CELL, ADD, EXIT);
    IU CELLM = _COLON("CELL-", CELL, SUB, EXIT);
    IU CELLS = _COLON("CELLS", CELL, MUL, EXIT);
    // Dr. Ting's alternate opcodes
    IU DDUP  = _COLON("2DUP",  OVER, OVER, EXIT);
    IU DDROP = _COLON("2DROP", DROP, DROP, EXIT);
    IU SSMOD = _COLON("*/MOD", TOR, MSTAR, RFROM, UMMOD, EXIT);
    _COLON("/MOD", DDUP, DIV, TOR, MOD, RFROM, EXIT);
    _COLON("*/",   SSMOD, SWAP, DROP, EXIT);
    _COLON("2!",   DUP, TOR, CELL, ADD, STORE, RFROM, STORE, EXIT);
    _COLON("2@",   DUP, TOR, AT, RFROM, CELL, ADD, AT, EXIT);
    IU WITHI = _COLON("WITHIN",OVER, SUB, TOR, SUB, RFROM, ULESS, EXIT);
    IU COUNT = _COLON("COUNT", DUP,  ONEP, SWAP, CAT, EXIT);
    IU ABS   = _COLON("ABS", DUP, ZLT); {
        _IF(NEG);
        _THEN(EXIT);
    }
    IU MAX   = _COLON("MAX", DDUP, LT); {
        _IF(SWAP);
        _THEN(DROP, EXIT);
    }
    IU MIN   = _COLON("MIN",  DDUP, GT); {
        _IF(SWAP);
        _THEN(DROP, EXIT);
    }
    IU CMOVE = _COLON("CMOVE", NOP); {
        _FOR(NOP);
        _AFT(OVER, CAT, OVER, CSTOR, TOR, ONEP, RFROM, ONEP);
        _THEN(NOP);
        _NEXT(DDROP, EXIT);
    }
    _COLON("MOVE", CELL, DIV); {
        _FOR(NOP);
        _AFT(OVER, AT, OVER, STORE, TOR, CELLP, RFROM, CELLP);
        _THEN(NOP);
        _NEXT(DDROP, EXIT);
    }
    _COLON("FILL", SWAP); {
        _FOR(SWAP);
        _AFT(DDUP, CSTOR, ONEP);
        _THEN(NOP);
        _NEXT(DDROP, EXIT);
    }
    ///
    ///> Number Conversions and formatting
    ///
    IU HEX_  = _COLON("HEX",     DOLIT, 16, vBASE, STORE, EXIT);
    IU DECIM = _COLON("DECIMAL", DOLIT, 10, vBASE, STORE, EXIT);
    IU DIGIT = _COLON("DIGIT",   DOLIT, 9, OVER, LT, DOLIT, 7, AND, ADD, DOLIT, 0x30, ADD, EXIT);
    IU EXTRC = _COLON("EXTRACT", DOLIT, 0, SWAP, UMMOD, SWAP, DIGIT, EXIT);
    IU BDIGS = _COLON("<#",      PAD, vHLD, STORE, EXIT);
    IU HOLD  = _COLON("HOLD",    vHLD, AT, ONEM, DUP, vHLD, STORE, CSTOR, EXIT);
    IU DIG   = _COLON("#",       vBASE, AT, EXTRC, HOLD, EXIT);
    IU DIGS  = _COLON("#S", NOP); {
        _BEGIN(DIG, DUP);
        _WHILE(NOP);
        _REPEAT(EXIT);
    }
    IU SIGN  = _COLON("SIGN", ZLT); {
        _IF(DOLIT, 0x2d, HOLD);
        _THEN(EXIT);
    }
    IU EDIGS = _COLON("#>",     DROP, vHLD, AT, PAD, OVER, SUB, EXIT);
    IU TOUPP = _COLON(">UPPER", DUP, DOLIT, 0x61, DOLIT, 0x7b, WITHI); { // [a-z] only?
        _IF(DOLIT, 0x5f, AND);
        _THEN(EXIT);
    }
    IU DIGTQ = _COLON("DIGIT?", TOR, TOUPP, DOLIT, 0x30, SUB, DOLIT, 9, OVER, LT); {
        _IF(DOLIT, 7, SUB, DUP, DOLIT, 10, LT, OR);           // handle hex number
        _THEN(DUP, RFROM, ULESS, EXIT);                       // handle base > 10
    }
    IU NUMBQ = _COLON("NUMBER?", vBASE, AT, TOR, DOLIT, 0, OVER, COUNT,
                      OVER, CAT, DOLIT, 0x24, EQ); {          // leading with $ (i.e. 0x24)
        _IF(HEX_, SWAP, ONEP, SWAP, ONEM);
        _THEN(OVER, CAT, DOLIT, 0x2d, EQ,                     // handle negative sign (i.e. 0x2d)
              TOR, SWAP, RAT, SUB, SWAP, RAT, ADD, QDUP);
        _IF(ONEM); {
            // a FOR..WHILE..NEXT..ELSE..THEN construct =~ for {..break..}
            _FOR(DUP, TOR, CAT, vBASE, AT, DIGTQ);
            _WHILE(SWAP, vBASE, AT, MUL, ADD, RFROM, ONEP);   // if digit, xBASE, else break to ELSE
            _NEXT(DROP, RAT); {                               // whether negative number
                _IF(NEG);
                _THEN(SWAP);
            }
            _ELSE(RFROM, RFROM, DDROP, DDROP, DOLIT, 0);
            _THEN(DUP);
         }
         _THEN(RFROM, DDROP, RFROM, vBASE, STORE, EXIT);
    }
    ///
    ///> Console I/O
    ///
    IU TIB   = _COLON("TIB",   vTTIB, AT, EXIT);
    IU QKEY  = _COLON("?KEY",  QRX, EXIT);
    IU KEY   = _COLON("KEY",   NOP); {
        _BEGIN(QKEY);
        _UNTIL(EXIT);
}
    IU EMIT  = _COLON("EMIT",  TXSTO, EXIT);
    IU HATH  = _COLON("^H", TOR, OVER, RFROM, SWAP, OVER, XOR); {
        _IF(DOLIT, 8, EMIT, ONEM, BLANK, EMIT, DOLIT, 8, EMIT);
        _THEN(EXIT);
    }
    IU SPACE = _COLON("SPACE", BLANK, EMIT, EXIT);
    IU CHARS = _COLON("CHARS", SWAP, DOLIT, 0, MAX); {
        _FOR(NOP);
        _AFT(DUP, EMIT);
        _THEN(NOP);
        _NEXT(DROP, EXIT);
    }
    IU TCHAR = _COLON(">CHAR", DOLIT, 0x7f, AND, DUP, DOLIT, 0x7f, BLANK, WITHI); {
    }
    IU SPACS = _COLON("SPACES", BLANK, CHARS, EXIT);
    IU TYPE  = _COLON("TYPE", NOP); {
        _FOR(NOP);
        _AFT(COUNT, DOLIT, 0x7f, AND, DUP, DOLIT, 0x7f, BLANK, WITHI); {
            _IF(DROP, DOLIT, 0x5f);    // out-of-range put '_' instead
            _THEN(EMIT);
        }
        _THEN(NOP);
        _NEXT(DROP, EXIT);
    }
    IU CR    = _COLON("CR",   DOLIT, 10, EMIT, EXIT);
    // IU CR    = _COLON("CR",   DOLIT, 10, DOLIT, 13, EMIT, EMIT, EXIT);   // LFCR
    IU DOSTR = _COLON("DOSTR",RFROM, RAT, RFROM, COUNT, ADD, TOR, SWAP, TOR, EXIT);
    IU STRQ  = _COLON("S\"|", DOSTR, EXIT);
       DOTQ  = _COLON(".\"|", DOSTR, COUNT, TYPE, EXIT);
    IU DOTR  = _COLON(".R",   TOR,
            DUP, TOR, ABS, BDIGS, DIGS, RFROM, SIGN, EDIGS,         // shown as string
            RFROM, OVER, SUB, SPACS, TYPE, EXIT);
    IU UDOTR = _COLON("U.R",  TOR, BDIGS, DIGS, EDIGS, RFROM, OVER, SUB, SPACS, TYPE, EXIT);
    IU UDOT  = _COLON("U.",   BDIGS, DIGS, EDIGS, SPACE, TYPE, EXIT);
    IU DOT   = _COLON(".",    vBASE, AT, DOLIT, 0xa, XOR); {
        _IF(UDOT, EXIT);                                            // base 10
        _THEN(DUP, TOR, ABS, BDIGS, DIGS, RFROM, SIGN, EDIGS,       // shown as string
              SPACE, TYPE, EXIT);
    }
    IU QUEST = _COLON("?", AT, DOT, EXIT);
    ///
    ///> Parser
    ///
    IU PARSE0= _COLON("(parse)", vTEMP, CSTOR, OVER, TOR, DUP); {  // delimiter kept in vTEMP
        _IF(ONEM, vTEMP, CAT, BLANK, EQ); {                   // check <SPC>
            _IF(NOP); {
                // a FOR..WHILE..NEXT..THEN construct =~ for {..break..}
                _FOR(BLANK, OVER, CAT, SUB, ZLT, INV);        //
                _WHILE(ONEP);                                 // break to THEN if is char, or next char
                _NEXT(RFROM, DROP, DOLIT, 0, DUP, EXIT);      // no break, (R>, DROP to rm loop counter)
                _THEN(RFROM);                                 // populate A0, i.e. break comes here, rm counter
            }
            _THEN(OVER, SWAP);                                // advance until next space found
            // a FOR..WHILE..NEXT..ELSE..THEN construct =~ DO..LEAVE..+LOOP
            _FOR(vTEMP, CAT, OVER, CAT, SUB, vTEMP, CAT, BLANK, EQ); {
                _IF(ZLT);
                _THEN(NOP);
            }
            _WHILE(ONEP);                                     // if (char <= space) break to ELSE
            _NEXT(DUP, TOR);                                  // no break, if counter < limit loop back to FOR
            _ELSE(RFROM, DROP, DUP, ONEP, TOR);               // R>, DROP to rm loop counter
            _THEN(OVER, SUB, RFROM, RFROM, SUB, EXIT);        // put token length on stack
        }
        _THEN(OVER, RFROM, SUB, EXIT);
    }
    IU PACKS = _COLON("PACK$", DUP, TOR, DDUP, CSTOR, ONEP, SWAP, CMOVE, RFROM, EXIT);
    IU PARSE = _COLON("PARSE",
                       TOR, TIB, vIN, AT, ADD, vNTIB, AT, vIN, AT, SUB, RFROM,
                       PARSE0, vIN, PSTOR,
                       EXIT);
    IU TOKEN = _COLON("TOKEN", BLANK, PARSE, DOLIT, 0x1f, MIN, HERE, CELLP, PACKS, EXIT);  // put token at HERE
    IU WORD  = _COLON("WORD",  PARSE, HERE, CELLP, PACKS, EXIT);
    IU NAMET = _COLON("NAME>", COUNT, DOLIT, 0x1f, AND, ADD, EXIT);
    IU SAMEQ = _COLON("SAME?", NOP); {  // (a1 a2 n - a1 a2 f) compare a1, a2 byte-by-byte
        _FOR(DDUP);
#if CASE_SENSITIVE
        _AFT(DUP, CAT, TOR, ONEP, SWAP,                                    // *a1++
             DUP, CAT, TOR, ONEP, SWAP, RFROM, RFROM, SUB, QDUP); {        // *a2++
#else
        _AFT(DUP, CAT, TOUPP, TOR, ONEP, SWAP,                             // *a1++
             DUP, CAT, TOUPP, TOR, ONEP, SWAP, RFROM, RFROM, SUB, QDUP); { // *a2++
#endif // CASE_SENSITIVE
            _IF(RFROM, DROP, TOR, DDROP, RFROM, EXIT);                     // pop off loop counter and pointers
            _THEN(NOP);
        }
        _THEN(NOP);
        _NEXT(DDROP, DOLIT, 0, EXIT);
    }
    IU FIND = _COLON("FIND", SWAP, DUP, CAT, vTEMP, STORE,      // keep length in temp
                     DUP, AT, TOR, CELLP, SWAP); {              // fetch 1st cell
        _BEGIN(AT, DUP); {                                      // 0000 = end of dic
#if CASE_SENSITIVE
            _IF(DUP, AT, DOLIT, 0xff3f, AND, RAT, XOR); {       // compare 2-byte
#else
            _IF(DUP, AT, DOLIT, 0xff3f, AND,
                    DOLIT, 0x5f5f, AND, RAT,
                    DOLIT, 0x5f5f, AND, XOR); {                 // compare 2-byte (uppercased)
#endif // CASE_SENSITIVE
                _IF(CELLP, DOLIT, 0xffff);                      // miss, try next word
                _ELSE(CELLP, vTEMP, AT, ONEM, DUP); {           // -1, since 1st byte has been compared
                    _IF(SAMEQ);                                 // compare strings if larger than 2 bytes
                    _THEN(NOP);
                }
                _THEN(NOP);
            }
            _ELSE(RFROM, DROP, SWAP, CELLM, SWAP, EXIT);
            _THEN(NOP);
        }
        _WHILE(CELLM, CELLM);                                             // get thread field to previous word
        _REPEAT(RFROM, DROP, SWAP, DROP, CELLM, DUP, NAMET, SWAP, EXIT);  // word found, get name field
    }
    IU NAMEQ = _COLON("NAME?", vCNTX, FIND, EXIT);
    ///
    ///> Interpreter Input String handler
    ///
    IU TAP   = _COLON("TAP", DUP, EMIT, OVER, CSTOR, ONEP, EXIT);                  // echo and store new char to TIB
    IU KTAP  = _COLON("kTAP", DUP, DOLIT, 0xd, XOR, OVER, DOLIT, 0xa, XOR, AND); { // check <CR><LF>
        _IF(DOLIT, 8, XOR); {                                                      // check <TAB>
            _IF(BLANK, TAP);                                                       // check BLANK
            _ELSE(HATH);
            _THEN(EXIT);
        }
        _THEN(DROP, SWAP, DROP, DUP, EXIT);
    }
    IU ACCEP = _COLON("ACCEPT", OVER, ADD, OVER); {             // accquire token from console
        _BEGIN(DDUP, XOR);                                      // loop through input stream
        _WHILE(KEY, DUP, BLANK, SUB, DOLIT, 0x5f, ULESS); {
            _IF(TAP);                                           // store new char into TIB
            _ELSE(KTAP);                                        // check if done
            _THEN(NOP);
        }
        _REPEAT(DROP, OVER, SUB, EXIT);                         // keep token length in #TIB
    }
    IU EXPEC = _COLON("EXPECT", ACCEP, vSPAN, STORE, DROP, EXIT);
    IU QUERY = _COLON("QUERY", TIB, DOLIT, FORTH_TIB_SZ, ACCEP,
                      vNTIB, STORE, DROP, DOLIT, 0, vIN, STORE, EXIT);
    ///
    ///> Text Interpreter
    ///
    ///    QUERY/TIB/ACCEPT - start the interpreter loop <-------<------.
    ///    TOKEN/PARSE - get a space delimited word                      \
    ///    EXECUTE - attempt to look up that word in the dictionary       \
    ///      NAME?/find - was the word found?                              ^
    ///      |-Yes:                                                        |
    ///      |   $INTERPRET - are we in compile mode?                      |
    ///      |   |-Yes:                                                    ^
    ///      |   |  \-Is the Word an Immediate word?                       |
    ///      |   |   |-Yes:                                                |
    ///      |   |   |    \- EXECUTE Execute the word -------------------->.
    ///      |   |    \-No:                                                |
    ///      |   |        \-Compile the word into the dictionary --------->.
    ///      |    \-No:                                                    |
    ///      |       \- EXECUTE Execute the word ----->------------------->.
    ///       \-No:                                                        ^
    ///          NUMBER? - Can the word be treated as a number?            |
    ///          |-Yes:                                                    |
    ///          | \-Are we in compile mode?                               |
    ///          |  |-Yes:                                                 |
    ///          |  |   \-Compile a literal into the dictionary >--------->.
    ///          |   \-No:                                                 |
    ///          |      \-Push the number to the variable stack >--------->.
    ///           \-No:                                                    /
    ///              \-An Error has occurred, prIU out an error message ->
    ///
    IU ABORT  = _COLON("ABORT", vTABRT, AT, QDUP); {
            _IF(EXECU);                              // @EXECUTE
            _THEN(EXIT);
        }
        ABORTQ = _COLON("ABORT\"", NOP); {
        _IF(DOSTR, COUNT, TYPE, ABORT);
        _THEN(DOSTR, DROP, EXIT);
    }
    IU ERROR = _COLON("ERROR", SPACE, COUNT, TYPE, DOLIT, 0x3f, EMIT, CR, ABORT);
    IU INTER = _COLON("$INTERPRET", NAMEQ, QDUP); {  // scan dictionary for word
        _IF(CAT, DOLIT, fCMPL, AND); {               // if it is compile only word
            _ABORTQ(" compile only");
            _LABEL(EXECU, EXIT);
        }
        _THEN(NUMBQ);                                // word name not found, check if it is a number
        _IF(EXIT);
        _ELSE(ERROR);
        _THEN(NOP);
    }
    IU iLBRAC= _IMMED("[", DOLIT, INTER, vMODE, STORE, EXIT);
    IU EVAL  = _COLON("EVAL", NOP); {
        _BEGIN(TOKEN, DUP, CAT);                     // fetch token length
        _WHILE(vMODE, AT, QDUP); {                   // execute if word exist
            _IF(EXECU);
            _THEN(NOP);
        }
        _REPEAT(DROP, CR, DOLIT, INTER, vMODE, AT, EQ); {
            _IF(DEPTH, DOLIT, 4, MIN); {             // dump stack and ok prompt
                _FOR(RAT, PICK, DOT);
                _NEXT(NOP);
                _DOTQ(" ok> ");
            }
            _THEN(EXIT);
        }
    }
     IU QUIT = _COLON("QUIT", DOLIT, FORTH_TIB_ADDR, vTTIB, STORE, iLBRAC); {  // clear TIB, interpreter mode
        _BEGIN(QUERY, EVAL);                      // main query-eval loop
        _AGAIN(NOP);
    }
    ///
    ///> Forth Compiler - utility functions
    ///
    IU COMMA = _COLON(",",       HERE, DUP, CELLP, vCP, STORE, STORE, EXIT);   // store a byte
    IU CCMMA = _COLON("C,",      HERE, DUP, ONEP,  vCP, STORE, CSTOR, EXIT);   // store a word
    IU UNIQU = _COLON("?UNIQUE", DUP, NAMEQ, QDUP); {
        _IF(COUNT, DOLIT, 0x1f, AND, SPACE, TYPE); {
            _DOTQ(" reDef");
        }
        _THEN(DROP, EXIT);
    }
    IU SNAME = _COLON("$,n", DUP, AT); {          // add new name field which is already build by PACK$
        _IF(UNIQU, DUP, NAMET, vCP, STORE, DUP, vLAST, STORE, CELLM, vCNTX, AT, SWAP, STORE, EXIT);
        _THEN(ERROR);
    }
    IU TICK  = _COLON("'", TOKEN, NAMEQ); {
        _IF(EXIT);
        _THEN(ERROR);
    }
    IU iLITR = _IMMED("LITERAL", DOLIT, DOLIT, COMMA, COMMA, EXIT);  // create a literal
    IU COMPI = _COLON("COMPILE",  RFROM, DUP, AT, COMMA, CELLP, TOR, EXIT);
    IU SCOMP = _COLON("$COMPILE", NAMEQ, QDUP); { // name found?
        _IF(AT, DOLIT, fIMMD, AND); {             // is immediate?
            _IF(EXECU);                           // execute
            _ELSE(COMMA);                         // or, add to dictionary
            _THEN(EXIT);
        }
        _THEN(NUMBQ);                             // a number?
        _IF(iLITR, EXIT);                         // add as literal
        _THEN(ERROR);
    }
    _COLON("COMPILE-ONLY", DOLIT, fCMPL, vLAST, AT, PSTOR, EXIT);   // enable COMPILE-ONLY flag
    _COLON("IMMEDIATE",    DOLIT, fIMMD, vLAST, AT, PSTOR, EXIT);   // enable IMMEDIATE flag
    _COLON("ALLOT",        vCP, PSTOR, EXIT);
    _IMMED("[COMPILE]",    TICK, COMMA, EXIT);                      // add word address to dictionary
    ///
    ///> Forth Compiler - define new word
    ///
    IU RBRAC = _COLON("]", DOLIT, SCOMP, vMODE, STORE, EXIT);       // switch into compiler-mode
    _COLON(":", TOKEN,
            SNAME,
            RBRAC, DOLIT, opENTER, CCMMA, EXIT);
    _IMMED(";", DOLIT, EXIT, COMMA, iLBRAC, vLAST, AT, vCNTX, STORE, EXIT);
    _COLON("FORGET", TOKEN, NAMEQ, QDUP); {
        _IF(CELLM, DUP, vCP, STORE, AT, DUP, vCNTX, STORE, vLAST, STORE, DROP, EXIT);
        _THEN(ERROR);
    }
    _IMMED(".(",      DOLIT, 0x29, PARSE, TYPE, EXIT);              // print til hit ) i.e. 0x29
    _IMMED("\\",      DOLIT, 0xa,  WORD,  DROP,  EXIT);             // skip til end of line
    _IMMED("(",       DOLIT, 0x29, PARSE, DDROP, EXIT);             // skip til )
    ///
    ///> Forth Compiler - variable, constant, and comments
    ///
    IU CODE  = _COLON("CODE", TOKEN,
            SNAME,
            vLAST, AT, vCNTX, STORE, EXIT);
    IU CREAT = _COLON("CREATE", CODE,  DOLIT, opDOVAR, CCMMA, EXIT);
    _COLON("VARIABLE",CREAT, DOLIT, 0, COMMA, EXIT);
    _COLON("CONSTANT",CODE,  DOLIT, opDOCON, CCMMA, COMMA, EXIT);
    ///
    ///> Forth Compiler - branching instructions
    ///
    ///> * BEGIN...AGAIN, BEGIN... f UNTIL, BEGIN...(once)...f WHILE...(loop)...REPEAT
    ///
    IU iAHEAD = _IMMED("AHEAD",   COMPI, BRAN,  HERE, DOLIT, 0, COMMA, EXIT);
    IU iAGAIN = _IMMED("AGAIN",   COMPI, BRAN,  COMMA, EXIT);
    _IMMED("BEGIN",   HERE, EXIT);
    _IMMED("UNTIL",   COMPI, QBRAN, COMMA, EXIT);
    ///
    ///> * f IF...THEN, f IF...ELSE...THEN
    ///
    IU iIF    = _IMMED("IF",      COMPI, QBRAN, HERE, DOLIT, 0, COMMA, EXIT);
    IU iTHEN  = _IMMED("THEN",    HERE, SWAP, STORE, EXIT);
    _IMMED("ELSE",    iAHEAD, SWAP, iTHEN, EXIT);
    _IMMED("WHILE",   iIF, SWAP, EXIT);
    _IMMED("WHEN",    iIF, OVER, EXIT);
    _IMMED("REPEAT",  iAGAIN, iTHEN, EXIT);
    ///
    ///> * n FOR...NEXT, n FOR...(first)... f AFT...(2nd,...)...THEN...(every)...NEXT
    ///
    _IMMED("FOR",     COMPI, TOR, HERE, EXIT);
    _IMMED("AFT",     DROP, iAHEAD, HERE, SWAP, EXIT);
    _IMMED("NEXT",    COMPI, DONXT, COMMA, EXIT);
    ///
    ///> Forth Compiler - String specification
    ///
    IU STRCQ  = _COLON("S,\"", DOLIT, 0x22, WORD,       // find quote in TIB (0x22 is " in ASCII)
                       COUNT, ADD, vCP, STORE, EXIT);   // advance dic pointer
    _IMMED("ABORT\"", DOLIT, ABORTQ, HERE, STORE, STRCQ, EXIT);
    _IMMED("S\"",     DOLIT, STRQ,   HERE, STORE, STRCQ, EXIT);
    _IMMED(".\"",     DOLIT, DOTQ,   HERE, STORE, STRCQ, EXIT);
    ///
    ///> Debugging Tools
    ///
    _COLON(">NAME", NOP); {
        _BEGIN(ONEM, DUP, CAT, DOLIT, 0x7f, AND, DOLIT, 0x20, LT);
        _UNTIL(EXIT);
    }
    _COLON("DUMP", vBASE, AT, TOR, HEX_, DOLIT, 0x1f, ADD, DOLIT, 0x10, DIV); {
        _FOR(NOP);
        _AFT(CR, DOLIT, 8, DDUP, OVER, DOLIT, 5, UDOTR); {
            _FOR(DOLIT, 0x3a, EMIT);                    // dump one row
            _AFT(DUP, AT, DOLIT, 5, UDOTR, CELLP);
            _THEN(NOP);
            _NEXT(TOR, SPACE, SPACE, CELLS, TYPE, RFROM);
        }
        _THEN(NOP);
        _NEXT(DROP, RFROM, vBASE, STORE, EXIT);         // restore BASE
    }
    _COLON("WORDS", CR, vCNTX, DOLIT, 0, vTEMP, STORE); {
        _BEGIN(AT, QDUP);
        _WHILE(DUP, COUNT, DOLIT, 0x1f, AND,            // .ID
        	DUP, DOLIT, 2, ADD, vTEMP, PSTOR,
        	TYPE, SPACE, SPACE, CELLM,
            vTEMP, AT, DOLIT, DUMP_ROW_WIDTH, GT); {    // check row width
            _IF(CR, DOLIT, 0, vTEMP, STORE);
            _THEN(NOP);
        }
        _REPEAT(EXIT);
    }
    ///
    ///> Arduino specific opcodes
    ///
    _CODE("DELAY",   opDELAY);
    _CODE("CLOCK",   opCLK  );
    _CODE("PINMODE", opPIN  );
    _CODE("MAP",     opMAP  );
    _CODE("IN",      opIN   );
    _CODE("OUT",     opOUT  );
    _CODE("AIN",     opAIN  );
    _CODE("PWM",     opPWM  );
    _CODE("TIMER",   opTMR  );
    _CODE("PCI",     opPCI  );
    _CODE("ENABLE_TIMER", opTMRE);
    _CODE("ENABLE_PCI",   opPCIE);
    ///
    ///> Cold Start address (End of dictionary)
    ///
    int last  = aPC + CELLSZ;               // name field of last word
    IU  COLD  = _COLON("COLD",
            DOLIT, last,  vCNTX, STORE,     // reset vectors
            DOLIT, last,  vLAST, STORE,
            DOLIT, INTER, vMODE, STORE,
            DOLIT, QUIT,  vTABRT,STORE,
            CR, QUIT);                      // enter the main query loop (QUIT)
    int here  = aPC;                        // current pointer
    ///
    ///> Boot Vector Setup
    ///
    SET(FORTH_BOOT_ADDR+1, COLD);

    return here;
}
}; // namespace EfAsm
///
/// eForth Assembler
///
using namespace EfAsm;
///
/// create C array dump for ROM
///
void _dump_rom(U8* cdata, int len)
{
    printf("//\n// cut and paste the following segment into Arduino C code\n//");
    printf("\nconst U32 forth_rom_sz PROGMEM = 0x%x;", len);
    printf("\nconst U32 forth_rom[] PROGMEM = {\n");
    for (int p=0; p<len+0x20; p+=0x20) {
        U32 *x = (U32*)&cdata[p];
        for (int i=0; i<0x8; i++, x++) {
            printf("0x%08x,", *x);
        }
        printf(" // %04x ", p);
        for (int i=0; i<0x20; i++) {
            U8 c = cdata[p+i];
            printf("%c", c ? ((c!=0x5c && c>0x1f && c<0x7f) ? c : '_') : '.');
        }
        printf("\n");
    }
    printf("};\n");
}

int ef_assemble(U8 *cdata) {
    int sz = assemble(cdata);

    _dump_rom(cdata, sz);

    return sz;
}

