/**
 * @file
 * @brief eForth Assembler module
 *
 * Forth Macro Assembler
 * Note: TODO: adding Leo Brodie's words
 */
#include "eforth_asm.h"

#define WORDS_ROW_WIDTH 0x44        /** WORDS row width control */
///
///@name eForth Assembler module variables
///@{
IU PC;                              ///< assembler program counter
U8 R;                               ///< assembler return stack index
U8 *_byte;                          ///< assembler byte array (heap)
IU _link;                           ///< link to previous word
///@}
///@name Compiled Address for Branching
///@{
IU DOTQP;                           ///< addr of output ops, used by _dotq, _strq, _abortq
///@}
///
///> eForth Macro Assembler
///
int assemble(U8 *rom)
{
    _byte = rom;
    _link = R = 0;
    ///
    ///> ROM starting address
    ///
    PC = FORTH_BOOT_ADDR;
    IU BOOT  = _LABEL(0xfeed);     ///< reserved for COLD boot vector
    ///
    ///> Kernel dictionary (primitive words)
    ///
    IU NOP   = _PRIM("NOP",     NOP    );
    IU EXIT  = _PRIM("EXIT",    EXIT   );
    IU ENTER = _PRIM("ENTER",   ENTER  );  /// aka doLIST
    IU BYE   = _PRIM("BYE",     BYE    );
    IU QKEY  = _PRIM("?KEY",    QRX    );
    IU EMIT  = _PRIM("EMIT",    TXSTO  );
    IU DOLIT = _PRIM("DOLIT",   DOLIT  );
    IU DOVAR = _PRIM("DOVAR",   DOVAR  );
    IU QBRAN = _PRIM("QBRANCH", QBRAN  );
    IU BRAN  = _PRIM("BRANCH",  BRAN   );
    IU DONXT = _PRIM("DONEXT",  DONEXT );
    IU EXECU = _PRIM("EXECUTE", EXECU  );
    IU STORE = _PRIM("!",       STORE  );
    IU PSTOR = _PRIM("+!",      PSTOR  );
    IU AT    = _PRIM("@",       AT     );
    IU CSTOR = _PRIM("C!",      CSTOR  );
    IU CAT   = _PRIM("C@",      CAT    );
    IU RFROM = _PRIM("R>",      RFROM  );
    IU RAT   = _PRIM("R@",      RAT    );
    IU TOR   = _PRIM(">R",      TOR    );
    IU DROP  = _PRIM("DROP",    DROP   );
    IU DUP   = _PRIM("DUP",     DUP    );
    IU SWAP  = _PRIM("SWAP",    SWAP   );
    IU OVER  = _PRIM("OVER",    OVER   );
    IU ROT   = _PRIM("ROT",     ROT    );
    IU PICK  = _PRIM("PICK",    PICK   );
    IU AND   = _PRIM("AND",     AND    );
    IU OR    = _PRIM("OR",      OR     );
    IU XOR   = _PRIM("XOR",     XOR    );
    IU INV   = _PRIM("INVERT",  INV    );
    IU LSH   = _PRIM("LSHIFT",  LSH    );
    IU RSH   = _PRIM("RSHIFT",  RSH    );
    IU ADD   = _PRIM("+",       ADD    );
    IU SUB   = _PRIM("-",       SUB    );
    IU MUL   = _PRIM("*",       MUL    );
    IU DIV   = _PRIM("/",       DIV    );
    IU MOD   = _PRIM("MOD",     MOD    );
    IU NEG   = _PRIM("NEGATE",  NEG    );
    IU GT    = _PRIM(">",       GT     );
    IU EQ    = _PRIM("=",       EQ     );
    IU LT    = _PRIM("<",       LT     );
    IU ZGT   = _PRIM("0>",      ZGT    );
    IU ZEQ   = _PRIM("0=",      ZEQ    );
    IU ZLT   = _PRIM("0<",      ZLT    );
    IU ONEP  = _PRIM("1+",      ONEP   );
    IU ONEM  = _PRIM("1-",      ONEM   );
    IU QDUP  = _PRIM("?DUP",    QDUP   );
    IU DEPTH = _PRIM("DEPTH",   DEPTH  );
    IU RP    = _PRIM("RP",      RP     );
    IU BLANK = _PRIM("BL",      BL     );
    IU CELL  = _PRIM("CELL",    CELL   );
    IU ABS   = _PRIM("ABS",     ABS    );
    IU MAX   = _PRIM("MAX",     MAX    );
    IU MIN   = _PRIM("MIN",     MIN    );
    IU WITHI = _PRIM("WITHIN",  WITHIN );    ///> ( u ul uh -- f ) check 3rd item within [ul uh)
    
    IU TOUPP = _PRIM(">UPPER",  TOUPP  );
    IU COUNT = _PRIM("COUNT",   COUNT  );
    IU ULESS = _PRIM("U<",      ULESS  );
    
    IU UMMOD = _PRIM("UM/MOD",  UMMOD  );    ///> ( udl udh u -- ur uq ) unsigned double divided by a single
    IU UMSTA = _PRIM("UM*",     UMSTAR );    ///> ( u1 u2 -- ud ) unsigned double = multiply unsigned singles
    IU MSTAR = _PRIM("M*",      MSTAR  );    ///> ( n1 n2 -- d ) double = single * single
    IU UMPLU = _PRIM("UM+",     UMPLUS );    ///> ( n1 n2 -- sum c ) add two numbers and carry flag
    IU SSMOD = _PRIM("*/MOD",   SSMOD  );    ///> ( n1 n2 n -- r q ) multiply n1 n2 div/mod by a single
    IU SMOD  = _PRIM("/MOD",    SMOD   );    ///> ( n1 n2 -- r q ) single devide
    IU MSLAS = _PRIM("*/",      MSLAS  );    ///> ( n1 n2 n3 -- q ) multiply n1 n2 divide by n3 return quotient
    
    IU S2D   = _PRIM("S>D",     S2D    );    ///> ( n -- dl dh )
    IU D2S   = _PRIM("D>S",     D2S    );    ///> ( dl dh -- n )
    IU DNEG  = _PRIM("DNEGATE", DNEG   );
    IU DADD  = _PRIM("D+",      DADD   );
    IU DSUB  = _PRIM("D-",      DSUB   );
    IU DSTOR = _COLON("2!",     DUP, TOR, DOLIT, CELLSZ, ADD, STORE, RFROM, STORE, EXIT);
    IU DAT   = _COLON("2@",     DUP, TOR, AT, RFROM, DOLIT, CELLSZ, ADD, AT, EXIT);
    IU DDUP  = _COLON("2DUP",   OVER, OVER, EXIT);
    IU DDROP = _COLON("2DROP",  DROP, DROP, EXIT);
    _COLON("2SWAP",   ROT, TOR, ROT, RFROM, EXIT);
    _COLON("2OVER",   DOLIT, 3, PICK, DOLIT, 3, PICK, EXIT);
    ///
    /// extended words
    ///
    _COLON("CELL+", CELL, ADD, EXIT);
    _COLON("CELL-", CELL, SUB, EXIT);
    _COLON("CELLS", CELL, MUL, EXIT);
    _COLON("2+",    DOLIT, 2, ADD, EXIT);
    _COLON("2-",    DOLIT, 2, SUB, EXIT);
    _COLON("2*",    DOLIT, 1, LSH, EXIT);
    _COLON("2/",    DOLIT, 1, RSH, EXIT);
    _COLON("S0",    DOLIT, FORTH_STACK_ADDR, EXIT);   ///> base of data stack (fixed instead of user var)
    _PRIM("SP@",   SPAT);                    ///> address of stack pointer
    _PRIM("I",     RAT );
    ///  (TODO: add J)
    _PRIM("TRACE", TRC  );                   ///  ( f -- )     enable/disable debug tracing
    _PRIM("SAVE",  SAVE );                   ///  ( -- )       save user variables and dictionary to EEPROM
    _PRIM("LOAD",  LOAD );                   ///  ( -- )       restore user variables and dictionary from EERPROM
    _PRIM("CALL",  CALL );                   ///  ( n -- )     call a C-function vector
    ///
    /// Kernel constants
    ///
    IU ua    = FORTH_UVAR_ADDR;
    IU vTTIB = _CODE("'TIB",    VAL(ua,0));   ///> * 'TIB console input buffer pointer
    IU vBASE = _CODE("BASE",    VAL(ua,1));   ///> * BASE current radix for numeric ops
    IU vCP   = _CODE("CP",      VAL(ua,2));   ///> * CP,  top of dictionary, same as HERE
    IU vCNTX = _CODE("CONTEXT", VAL(ua,3));   ///> * CONTEXT name field of last word
    IU vLAST = _CODE("LAST",    VAL(ua,4));   ///> * LAST, same as CONTEXT
    IU vMODE = _CODE("'MODE",   VAL(ua,5));   ///> * 'MODE ('TEVAL - interpreter or compiler)
    IU vTABRT= _CODE("'ABORT",  VAL(ua,6));   ///> * ABORT exception rescue handler (QUIT)
    IU vHLD  = _CODE("HLD",     VAL(ua,7));   ///> * HLD  char pointer to output buffer
    IU vSPAN = _CODE("SPAN",    VAL(ua,8));   ///> * SPAN number of character accepted
    IU vIN   = _CODE(">IN",     VAL(ua,9));   ///> * >IN  interpreter pointer to next char
    IU vNTIB = _CODE("#TIB",    VAL(ua,10));  ///> * #TIB number of character received in TIB
    IU vTMP  = _CODE("tmp",     VAL(ua,11));  ///> * tmp storage (alternative to return stack)
    ///
    ///> Console Input and Common words
    ///
    IU KEY   = _COLON("KEY",   NOP); {
        _BEGIN(QKEY);
        _UNTIL(EXIT);
    }
    IU TCHAR = _COLON(">CHAR", DOLIT, 0x7f, AND, DUP, DOLIT, 0x7f, BLANK, WITHI); {
        _IF(DROP, DOLIT, 0x5f);
        _THEN(EXIT);
    }
    IU HERE  = _COLON("HERE",  vCP, AT, EXIT);                /// top of dictionary
    IU PAD   = _COLON("PAD",   DOLIT, FORTH_MAX_ADDR, EXIT);  /// use tail of TIB for output
    IU TIB   = _COLON("TIB",   vTTIB, AT, EXIT);
    IU CMOVE = _COLON("CMOVE", NOP); {
        _FOR(NOP);
        _AFT(OVER, CAT, OVER, CSTOR, TOR, ONEP, RFROM, ONEP);
        _THEN(NOP);
        _NEXT(DROP, DROP, EXIT);
    }
    _COLON("MOVE", CELL, DIV); {
        _FOR(NOP);
        _AFT(OVER, AT, OVER, STORE, TOR, CELL, ADD, RFROM, CELL, ADD);
        _THEN(NOP);
        _NEXT(DROP, DROP, EXIT);
    }
    _COLON("FILL", SWAP); {
        _FOR(SWAP);
        _AFT(DDUP, CSTOR, ONEP);
        _THEN(NOP);
        _NEXT(DROP, DROP, EXIT);
    }
    ///
    ///> Number Conversions and formatting
    ///
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
    IU EDIGS = _COLON("#>",      DROP, vHLD, AT, PAD, OVER, SUB, EXIT);
    IU STR   = _COLON("STR",     DUP, TOR, ABS, BDIGS, DIGS, RFROM, SIGN, EDIGS, EXIT);
    IU HEX_  = _COLON("HEX",     DOLIT, 16, vBASE, STORE, EXIT);
    IU DECIM = _COLON("DECIMAL", DOLIT, 10, vBASE, STORE, EXIT);
    IU DIGTQ = _COLON("DIGIT?",
         TOR, TOUPP, DOLIT, 0x30, SUB, DOLIT, 9, OVER, LT); {
        _IF(DOLIT, 7, SUB, DUP, DOLIT, 10, LT, OR);           /// handle hex number
        _THEN(DUP, RFROM, ULESS, EXIT);                       /// handle base > 10
    }
    /// TODO: add >NUMBER
    IU NUMBQ = _COLON("NUMBER?",
        vBASE, AT, TOR, DOLIT, 0, OVER, COUNT,
        OVER, CAT, DOLIT, 0x24, EQ); {                        /// leading with $ (i.e. 0x24)
        _IF(HEX_, SWAP, ONEP, SWAP, ONEM);
        _THEN(OVER, CAT, DOLIT, 0x2d, EQ,                     /// handle negative sign (i.e. 0x2d)
             TOR, SWAP, RAT, SUB, SWAP, RAT, ADD, QDUP);
        _IF(ONEM); {
            /// a FOR..WHILE..NEXT..ELSE..THEN construct =~ for {..break..}
            _FOR(DUP, TOR, CAT, vBASE, AT, DIGTQ);
            _WHILE(SWAP, vBASE, AT, MUL, ADD, RFROM, ONEP);   /// if digit, xBASE, else break to ELSE
            _NEXT(DROP, RAT); {                               /// whether negative number
                _IF(NEG);
                _THEN(SWAP);
            }
            _ELSE(RFROM, RFROM, DDROP, DDROP, DOLIT, 0);
            _THEN(DUP);
         }
         _THEN(RFROM, DDROP, RFROM, vBASE, STORE, EXIT);
    }
    ///
    ///> Console Output
    ///
    IU SPACE = _COLON("SPACE", BLANK, EMIT, EXIT);
    IU CHARS = _COLON("CHARS", SWAP, DOLIT, 0, MAX); {
        _FOR(NOP);
        _AFT(DUP, EMIT);
        _THEN(NOP);
        _NEXT(DROP, EXIT);
    }
    IU SPACS = _COLON("SPACES", BLANK, CHARS, EXIT);
    IU TYPE  = _COLON("TYPE", NOP); {
        _FOR(NOP);
        _AFT(COUNT, DOLIT, 0x7f, AND, DUP, DOLIT, 0x7f, BLANK, WITHI); {
            _IF(DROP, DOLIT, 0x5f);    /// out-of-range put '_' instead
            _THEN(EMIT);
        }
        _THEN(NOP);
        _NEXT(DROP, EXIT);
    }
    IU CR    = _COLON("CR",   DOLIT, 10, EMIT, EXIT);         /// LF i.e. \n actually
    IU DOSTR = _COLON("do$",  RFROM, RAT, RFROM, COUNT, ADD, TOR, SWAP, TOR, COUNT, EXIT);
    IU STRQP = _COLON("$\"|", DOSTR, EXIT);
       DOTQP = _COLON(".\"|", DOSTR, TYPE, EXIT);             /// Note: DOTQP export to _dotq
    IU DOTR  = _COLON(".R",   TOR, STR, RFROM, OVER, SUB, SPACS, TYPE, EXIT);  /// shown as string
    IU UDOTR = _COLON("U.R",  TOR, BDIGS, DIGS, EDIGS, RFROM, OVER, SUB, SPACS, TYPE, EXIT);
    IU UDOT  = _COLON("U.",   BDIGS, DIGS, EDIGS, SPACE, TYPE, EXIT);
    IU DOT   = _COLON(".",    vBASE, AT, DOLIT, 0xa, XOR); {
        _IF(UDOT, EXIT);                                      /// base 10
        _THEN(STR, SPACE, TYPE, EXIT);                        /// shown as string
    }
    IU QUEST = _COLON("?", AT, DOT, EXIT);
    /// TODO: add PAGE
    ///
    ///> Parsing
    ///
    IU PARSE0= _COLON("(parse)", vTMP, CSTOR, OVER, TOR, DUP); {   /// ( addr len delim -- ) delimiter kept tmp, addr in R
        _IF(ONEM, vTMP, CAT, BLANK, EQ); {                    /// if (len) { --len; if (delim==" ") {...} }
            _IF(NOP); {
                /// a FOR..WHILE..NEXT..THEN construct =~ for {..break..}
                _FOR(BLANK, OVER, CAT, SUB, ZLT, INV);        /// for (len)
                _WHILE(ONEP);                                 /// break to THEN if is char, or next char
                _NEXT(RFROM, DROP, DOLIT, 0, DUP, EXIT);      /// no break, (R>, DROP to rm loop counter)
                _THEN(RFROM);                                 /// populate A0, i.e. break comes here, rm counter
            }
            _THEN(OVER, SWAP); {                              /// advance until next space found
                /// a FOR..WHILE..NEXT..ELSE..THEN construct =~ DO..LEAVE..+LOOP
                _FOR(vTMP, CAT, OVER, CAT, SUB, vTMP, CAT, BLANK, EQ); {
                  _IF(ZLT);
                  _THEN(NOP);
                }
                _WHILE(ONEP);                                 /// if (char <= space) break to ELSE
                _NEXT(DUP, TOR);                              /// no break, if counter < limit loop back to FOR
                _ELSE(RFROM, DROP, DUP, ONEP, TOR);           /// R>, DROP to rm loop counter
               _THEN(OVER, SUB, RFROM, RFROM, SUB, EXIT);     /// put token length on stack
            }
        }
        _THEN(OVER, RFROM, SUB, EXIT);
    }
    IU PACKS = _COLON("PACK$", DUP, TOR, DDUP, CSTOR, ONEP, SWAP, CMOVE, RFROM, EXIT);     /// ( b u a -- a )
    IU PARSE = _COLON("PARSE",                                                             /// ( c -- b u )
                       TOR, TIB, vIN, AT, ADD, vNTIB, AT, vIN, AT, SUB, RFROM,
                       PARSE0, vIN, PSTOR,
                       EXIT);
    IU TOKEN = _COLON("TOKEN", BLANK, PARSE, DOLIT, 0x1f, MIN, HERE, CELL, ADD, PACKS, EXIT);  /// ( -- a; <string>) put token at HERE
    IU WORD  = _COLON("WORD",  PARSE, HERE, CELL, ADD, PACKS, EXIT);                           /// ( c -- a; <string)
    ///
    ///> Dictionary serach
    ///
    IU NAMET = _COLON("NAME>", COUNT, DOLIT, 0x1f, AND, ADD, EXIT);        /// ( nfa -- cfa )
    IU SAMEQ = _COLON("SAME?", NOP); {                                     /// ( a1 a2 n - a1 a2 f ) compare a1, a2 byte-by-byte
        _FOR(DDUP);
#if CASE_SENSITIVE
        _AFT(DUP, CAT, TOR, ONEP, SWAP,                                    /// *a1++
             DUP, CAT, TOR, ONEP, SWAP, RFROM, RFROM, SUB, QDUP); {        /// *a2++
#else
        _AFT(DUP, CAT, TOUPP, TOR, ONEP, SWAP,                             /// *a1++
             DUP, CAT, TOUPP, TOR, ONEP, SWAP, RFROM, RFROM, SUB, QDUP); { /// *a2++
#endif /// CASE_SENSITIVE
            _IF(RFROM, DROP, TOR, DDROP, RFROM, EXIT);          /// pop off loop counter and pointers
            _THEN(NOP);
        }
        _THEN(NOP);
        _NEXT(DDROP, DOLIT, FALSE, EXIT);                       /// SAME!
    }
    /// TODO: add COMPARE
    IU FIND = _COLON("FIND",                                    /// ( a va -- cfa nfa, a F ) keep length in tmp
        SWAP, DUP, CAT, vTMP, STORE, DUP, AT, TOR, CELL, ADD, SWAP); { /// fetch 1st cell
        _BEGIN(AT, DUP); {                                      /// 0000 = end of dic
#if CASE_SENSITIVE
            _IF(DUP, AT, DOLIT, 0x3fff, AND, RAT, XOR); {       /// compare 2-byte
#else
            _IF(DUP, AT, DOLIT, 0x3fff, AND,
                    DOLIT, 0x5f5f, AND, RAT,
                    DOLIT, 0x5f5f, AND, XOR); {                 /// compare 2-byte (uppercased)
#endif /// CASE_SENSITIVE
                _IF(CELL, ADD, DOLIT, TRUE);                    /// miss, try next word
                _ELSE(CELL, ADD, vTMP, AT, ONEM, DUP); {        /// -1, since 1st byte has been compared
                    _IF(SAMEQ);                                 /// compare strings if larger than 2 bytes
                    _THEN(NOP);
                }
                _THEN(NOP);
            }
            _ELSE(RFROM, DROP, SWAP, CELL, SUB, SWAP, EXIT);
            _THEN(NOP);
        }
        _WHILE(CELL, SUB, CELL, SUB);                                     /// get thread field to previous word
        _REPEAT(RFROM, DROP, SWAP, DROP, CELL, SUB, DUP, NAMET, SWAP, EXIT);  /// word found, get name field
    }
    IU NAMEQ = _COLON("NAME?", vCNTX, FIND, EXIT);
    ///
    ///> Terminal Input
    ///
    IU HATH  = _COLON("^H", TOR, OVER, RFROM, SWAP, OVER, XOR); {
        _IF(DOLIT, 8, EMIT, ONEM, BLANK, EMIT, DOLIT, 8, EMIT);
        _THEN(EXIT);
    }
    IU TAP   = _COLON("TAP", DUP, EMIT, OVER, CSTOR, ONEP, EXIT);                  /// echo and store new char to TIB
    IU KTAP  = _COLON("kTAP", DUP, DOLIT, 0xd, XOR, OVER, DOLIT, 0xa, XOR, AND); { /// check <CR><LF>
        _IF(DOLIT, 8, XOR); {                                                      /// check <TAB>
            _IF(BLANK, TAP);                                                       /// check BLANK
            _ELSE(HATH);
            _THEN(EXIT);
        }
        _THEN(DROP, SWAP, DROP, DUP, EXIT);
    }
    IU ACCEP = _COLON("ACCEPT", OVER, ADD, OVER); {             /// accquire token from console
        _BEGIN(DDUP, XOR);                                      /// loop through input stream
        _WHILE(KEY, DUP, BLANK, SUB, DOLIT, 0x5f, ULESS); {
            _IF(TAP);                                           /// store new char into TIB
            _ELSE(KTAP);                                        /// check if done
            _THEN(NOP);
        }
        _REPEAT(DROP, OVER, SUB, EXIT);                         /// keep token length in #TIB
    }
    IU EXPEC = _COLON("EXPECT", ACCEP, vSPAN, STORE, DROP, EXIT);
    IU QUERY = _COLON("QUERY", TIB, DOLIT, FORTH_TIB_SZ, ACCEP,
                      vNTIB, STORE, DROP, DOLIT, 0, vIN, STORE, EXIT);
    ///
    ///> Text Interpreter
    ///
    ///    QUERY/TIB/ACCEPT - start the interpreter loop <-------<------.
    ///    TOKEN/PARSE - get a space delimited word                      .
    ///    EXECUTE - attempt to look up that word in the dictionary       .
    ///      NAME?/find - was the word found?                              ^
    ///      |-Yes:                                                        |
    ///      |   $INTERPRET - are we in compile mode?                      |
    ///      |   |-Yes:                                                    ^
    ///      |   |  \-Is the Word an Immediate word?                       |
    ///      |   |   |-Yes:                                                |
    ///      |   |   |    \- EXECUTE Execute the word -------------------->.
    ///      |   |   |     |                                               |
    ///      |   |   |     \- if (NAME?) EXECU else NUMBQ ---------------->.
    ///      |   |    \-No:                                                |
    ///      |   |        \-Compile the word into the dictionary --------->.
    ///      |    \-No:                                                    |
    ///      |       \- EXECUTE Execute the word ----->------------------->.
    ///       \-No:                                                        ^
    ///          NUMBER? - Can the word be treated as a number?            |
    ///          |-Yes:                                                    |
    ///          | \-Are we in compile mode?                               |
    ///          |  |-Yes:                                                 |
    ///          |  |  |                                                   |
    ///          |  |   \-Compile a literal into the dictionary >--------->.
    ///          |  |                                                      |
    ///          |   \-No:                                                 |
    ///          |     |                                                   |
    ///          |      \-Push the number to the variable stack >--------->.
    ///           \-No:                                                   .
    ///              \-An Error has occurred, prIU out an error message ->
    ///
    IU ABORT = _COLON("ABORT", vTABRT, AT, QDUP); {  /// load ABORT vector
            _IF(EXECU);                              /// @EXECUTE
            _THEN(EXIT);
        }
    /// TODO: add ?STACK
    IU ERROR = _COLON("ERROR", SPACE, COUNT, TYPE, DOLIT, 0x3f, EMIT, CR, ABORT);
    IU INTER = _COLON("$INTERPRET", NAMEQ, QDUP); {  /// scan dictionary for word
        _IF(CAT, DOLIT, fCMPL, AND); {               /// check for compile only word
            _IF(DROP); {
                _DOTQ("compile only");               /// INTER0 of Dr Ting's
            }
            _ELSE(EXECU);
            _THEN(EXIT);
        }
        _THEN(NUMBQ);                                /// word name not found, check if it is a number
        _IF(EXIT);
        _ELSE(ERROR);
        _THEN(NOP);
    }
    IU iLBRAC= _IMMED("[", DOLIT, INTER, vMODE, STORE, EXIT);
    IU DOTOK = _COLON(".OK", CR, DOLIT, INTER, vMODE, AT, EQ); {
        _IF(DEPTH, DOLIT, 4, MIN); {                 /// dump stack and ok prompt
            _FOR(RAT, PICK, DOT);
            _NEXT(NOP);
            _DOTQ(" ok> ");
        }
        _THEN(EXIT);
    }
    IU EVAL  = _COLON("EVAL", NOP); {
        _BEGIN(TOKEN, DUP, CAT);                     /// fetch token length
        _WHILE(vMODE, AT, QDUP); {                   /// fetch operation mode ($INTERPRET or $COMPILE)
            _IF(EXECU);                              /// execute according to mode
            _THEN(NOP);
        }
        _REPEAT(DROP, DOTOK, EXIT);
    }
    IU QUIT = _COLON("QUIT", DOLIT, FORTH_TIB_ADDR, vTTIB, STORE, iLBRAC); {  /// clear TIB, interpreter mode
        _BEGIN(QUERY, EVAL);                         /// main query-eval loop
        _AGAIN(NOP);
    }
    ///
    ///> Colon Compiler
    ///
    IU COMMA = _COLON(",",  HERE, DUP, CELL, ADD, vCP, STORE, STORE, EXIT);   /// store a byte
    IU CCMMA = _COLON("C,", HERE, DUP, ONEP,  vCP, STORE, CSTOR, EXIT);       /// store a word
    IU ALLOT = _COLON("ALLOT",    vCP, PSTOR, EXIT);
    IU iLITR = _IMMED("LITERAL",  DOLIT, opDOLIT, CCMMA, COMMA, EXIT);        /// create a literal
    IU COMPI = _COLON("COMPILE",  RFROM, DUP, CAT, CCMMA, ONEP, TOR, EXIT);
    IU SCOMP = _COLON("$COMPILE", NAMEQ, QDUP); {    /// name found?
        _IF(CAT, DOLIT, fIMMD, AND); {               /// is immediate?
            _IF(EXECU);                              /// execute
            _ELSE(DUP, DUP, DOLIT, FORTH_ROM_SZ, LT, /// a primitive?
                  SWAP, ONEP, CAT, DOLIT, EXIT, EQ, AND); {  /// XX08 <= this might break
                _IF(CAT, CCMMA);                     /// append just the opcode
                _ELSE(DOLIT, fCOLON16, OR, COMMA);   /// append colon word address with flag
                _THEN(NOP);
            }
            _THEN(EXIT);
        }
        _THEN(NUMBQ);                                /// a number?
        _IF(iLITR, EXIT);                            /// append as a literal
        _THEN(ERROR);
    }
    IU UNIQU = _COLON("?UNIQUE", DUP, NAMEQ, QDUP); {
        _IF(COUNT, DOLIT, 0x1f, AND, SPACE, TYPE); {
            _DOTQ(" reDef");
        }
        _THEN(DROP, EXIT);
    }
    IU SNAME = _COLON("$,n", DUP, AT); {  /// add new name field which is already build by PACK$
        _IF(UNIQU,
            DUP, NAMET, vCP, STORE,    DUP, vLAST, STORE,
            CELL, SUB, vCNTX, AT, SWAP, STORE, EXIT);
        _THEN(ERROR);
    }
    IU TICK  = _COLON("'", TOKEN, NAMEQ); {
        _IF(EXIT);
        _THEN(ERROR);
    }
    IU RBRAC = _COLON("]", DOLIT, SCOMP, vMODE, STORE, EXIT);  /// switch into compiler-mode
    /// TODO: add [']
    _IMMED("[COMPILE]", TICK, COMMA, EXIT);                    /// add word address to dictionary
    _COLON(":", TOKEN, SNAME, RBRAC, EXIT);
    _IMMED(";", COMPI, opEXIT, iLBRAC, vLAST, AT, vCNTX, STORE, EXIT);
    ///
    ///> Debugging Tools
    ///
    IU TNAME = _COLON(">NAME", NOP); {                         /// ( pfa -- nfa )
        _BEGIN(ONEM, DUP, CAT, DOLIT, 0x7f, AND, DOLIT, 0x20, LT);
        _UNTIL(EXIT);
    }
    _COLON("DUMP", vBASE, AT, TOR, HEX_,                       /// save BASE, make HEX
            DOLIT, 0x1f, ADD, DOLIT, 0x10, DIV); {             /// get row count
        _FOR(NOP);
        _AFT(CR, DOLIT, 0x10, DDUP, OVER, DOLIT, 5, UDOTR); {  /// dump one row
            _FOR(DOLIT, 0x3a, EMIT);                           /// :
            _AFT(SPACE, DUP, CAT,
                DOLIT, 0x10, EXTRC, SWAP,                      /// low-nibble
                DOLIT, 0x10, EXTRC, EMIT, DROP, EMIT,          /// high-nibble
                ONEP, RAT, DOLIT, 8, EQ); {
                _IF(SPACE);
                _THEN(NOP);
            }
            _THEN(NOP);
            _NEXT(TOR, SPACE, SPACE, TYPE, RFROM);
        }
        _THEN(NOP);
        _NEXT(DROP, RFROM, vBASE, STORE, EXIT);                /// restore BASE
    }
    _COLON("WORDS", CR, vCNTX, DOLIT, 0, vTMP, STORE); {       /// tmp keeps width
        _BEGIN(AT, QDUP);
        _WHILE(DUP, COUNT, DOLIT, 0x1f, AND,                   /// get name length
             DUP, ONEP, ONEP, vTMP, PSTOR,                     /// add to tmp
             TYPE, SPACE, SPACE, CELL, SUB,                    /// get LFA
             vTMP, AT, DOLIT, WORDS_ROW_WIDTH, GT); {          /// check row width
            _IF(CR, DOLIT, 0, vTMP, STORE);
            _THEN(NOP);
        }
        _REPEAT(EXIT);
    }
    _COLON("FORGET", TOKEN, NAMEQ, QDUP); {
        _IF(CELL, SUB, DUP, vCP, STORE, AT, DUP, vCNTX, STORE, vLAST, STORE, DROP, EXIT);
        _THEN(ERROR);
    }
#if ENABLE_SEE
    /// Optional: Takes ~300 bytes ROM space
    ///> display address with colon delimiter ( a -- )
    IU DOTAD = _COLON(".ADDR", CR, DUP, DOT, DOLIT, 0x3a, EMIT, EXIT);
    ///> display opcode at given address ( a0 op -- a1 )
    IU DOTOP = _COLON(".OP", DUP, DOLIT, fCOLON8, AND); {      /// check primitive flag?
        /* poorman's CASE */
        _IF(DROP, DUP, AT, DOLIT, 0x7fff, AND, DUP,            /// colon word - show name
            SPACE, TNAME, COUNT, TYPE, DUP,
            DOLIT, DOTQP, EQ, SWAP, DOLIT, STRQP, EQ, OR); {
            _IF(SPACE, CELL, ADD, COUNT, DDUP, TYPE,           /// ."| or $"| (string)"
                DOLIT, 0x22, EMIT, ADD, EXIT);
            _ELSE(CELL, ADD, EXIT);
            _THEN(NOP);
        }
        _THEN(DUP, DOLIT, opDOLIT, EQ);                        /// a literal?
        _IF(DROP, ONEP, DUP, AT, DOT, CELL, ADD, EXIT);        /// show the number
        _THEN(DUP, DOLIT, opDOVAR, EQ);
        _IF(DROP, ONEP, ONEP, DUP, AT, DOT,                    /// (var)v
            DOLIT, 0x76, EMIT, ONEM, EXIT);
        _THEN(DUP, DOLIT, opBRAN, EQ);
        _IF(DROP, ONEP, DUP, AT, DOT,                          /// show jump target
            DOLIT, 0x6a, EMIT, CELL, ADD, DOTAD, EXIT);        /// (jump target)j
        _THEN(DUP, DOLIT, opQBRAN, EQ);                        /// or a qbran
        _IF(DROP, ONEP, DUP, AT, DOT,                          /// show jump target
            DOLIT, 0x3f, EMIT, CELL, ADD, DOTAD, EXIT);        /// (jump target)?
        _THEN(DUP, DOLIT, opDONEXT, EQ);                       /// a bran or donext?
        _IF(DROP, ONEP, DUP, AT, DOT,                          /// show jump target
            DOLIT, 0x6e, EMIT, CELL, ADD, DOTAD, EXIT);        /// (jump target)n
        _THEN(DUP, DOLIT, opDOES, EQ);                         /// DOES>
        _IF(DROP, ONEP, DUP, CAT, SWAP,
            DUP, ONEP, AT, DOT, DOLIT, 0x2a, EMIT,             /// (var)*
            ADD, ONEP, AT, DUP, DOT, DOLIT, 0x6a, EMIT,        /// (does> target)j
            DOTAD, EXIT);                                      /// skip to defining word
        _THEN(SPACE, vCNTX);                                   /// show other opcode#
        /// opcode -> name ( a op -- a+1 )
        _BEGIN(AT, DUP);                                       /// 0000 = end of dic
        _WHILE(DUP, NAMET,                                     /// primitive words?
               DUP, ONEP, CAT, DOLIT, opEXIT, EQ); {           /// * check second byte is EXIT
            _IF(CAT, TOR, OVER, RFROM, EQ); {                  /// same as our opcode?
                _IF(COUNT, TYPE, DROP, ONEP, EXIT);            /// print name, done!
                _THEN(DUP);
            }
            _THEN(DROP, CELL, SUB);                            /// link to prev word
        }
        _REPEAT(DOT, DOLIT, 0x3f, EMIT, DROP, ONEP, EXIT);     /// ?(unknown opcode)
    }
    _COLON("SEE", TICK, DOTAD); {                              /// word address
        _BEGIN(DUP, CAT, DUP, DOLIT, opEXIT, EQ, INV);         /// loop until EXIT
        _WHILE(DOTOP);                                         /// disasmble opcode
        _REPEAT(DDROP, SPACE, DOLIT, 0x3b, EMIT, EXIT);        /// semi colon
    }
#endif /// ENABLE_SEE
    ///
    ///> Control Structures
    ///
    ///> * BEGIN...AGAIN, BEGIN... f UNTIL, BEGIN...(once)...f WHILE...(loop)...REPEAT
    ///
    IU iAHEAD = _IMMED("AHEAD", COMPI, opBRAN, HERE, DOLIT, 0, COMMA, EXIT);
    IU iAGAIN = _IMMED("AGAIN", COMPI, opBRAN, COMMA, EXIT);
    _IMMED("BEGIN", HERE, EXIT);
    _IMMED("UNTIL", COMPI, opQBRAN, EXIT);
    ///
    ///> * f IF...THEN, f IF...ELSE...THEN
    ///
    IU iIF    = _IMMED("IF",   COMPI, opQBRAN, HERE, DOLIT, 0, COMMA, EXIT);
    IU iTHEN  = _IMMED("THEN", HERE, SWAP, STORE, EXIT);
    _IMMED("ELSE",  iAHEAD, SWAP, iTHEN, EXIT);
    _IMMED("WHILE", iIF, SWAP, EXIT);
    _IMMED("WHEN",  iIF, OVER, EXIT);
    _IMMED("REPEAT",iAGAIN, iTHEN, EXIT);
    ///
    ///> * n FOR...NEXT, n FOR...(first)... f AFT...(2nd,...)...THEN...(every)...NEXT
    ///
    /// TODO: add DO...LOOP, +LOOP, LEAVE
    ///
    _IMMED("FOR",   COMPI, opTOR, HERE, EXIT);
    _IMMED("AFT",   DROP, iAHEAD, HERE, SWAP, EXIT);
    _IMMED("NEXT",  COMPI, opDONEXT, COMMA, EXIT);
    ///
    ///> String Literals
    ///
    IU STRCQ  = _COLON("$,\"", DOLIT, 0x22, WORD,       /// find quote in TIB (0x22 is " in ASCII)
                    COUNT, ADD, vCP, STORE, EXIT);      /// advance dic pointer
    _IMMED("$\"",   DOLIT, STRQP | fCOLON16, HERE, STORE, STRCQ, EXIT);
    _IMMED(".\"",   DOLIT, DOTQP | fCOLON16, HERE, STORE, STRCQ, EXIT);
    ///
    ///> Defining Words - variable, constant, and comments
    ///
    IU CODE  = _COLON("CODE", TOKEN, SNAME, vLAST, AT, vCNTX, STORE, EXIT);
    IU CREAT = _COLON("CREATE", CODE, COMPI, opDOVAR, COMPI, opEXIT, EXIT);
    _COLON("DOES>",                                             ///> change runtime behavior to following code
           RFROM, HERE, vLAST, AT, NAMET, DUP, TOR, SUB, ONEM,  /// ( ra ca ) para on return stack, offset to defining word
           DOLIT, opDOES, RAT, CSTOR, RFROM, ONEP, CSTOR,
           COMPI, opBRAN, COMMA, COMPI, opEXIT, EXIT);          /// * the last opEXIT is not needed but nicer
    /// TODO: add POSTPONE
    _COLON("VARIABLE",  CREAT, DOLIT, 0, COMMA, EXIT);
    _COLON("CONSTANT",  CODE,                                   /// * CC: Dr. Ting hardcoded here
           DOLIT, opDOLIT, CCMMA, HERE, DOLIT, 4, ADD, COMMA,   /// * calculate addr of constant
           COMPI, opAT, COMPI, opEXIT, COMMA, EXIT);
    _COLON("2VARIABLE", CREAT, DOLIT, 0, DUP, COMMA, COMMA, EXIT);
    _COLON("2CONSTANT", CODE,
           DOLIT, opDOLIT, CCMMA, HERE, DOLIT, 4, ADD, COMMA,
           DAT, COMPI, opEXIT, SWAP, COMMA, COMMA, EXIT);
    ///
    ///> Comments
    ///
    _IMMED(".(", DOLIT, 0x29, PARSE, TYPE, EXIT);       /// print til hit ) i.e. 0x29
    _IMMED("\\", DOLIT, 0xa,  WORD,  DROP, EXIT);       /// skip til end of line
    _IMMED("(",  DOLIT, 0x29, PARSE, DDROP, EXIT);      /// skip til )
    ///
    ///> Lexicon Bits
    ///
    _COLON("COMPILE-ONLY", vLAST, AT, DUP,              /// enable COMPILE-ONLY flag
        CAT, DOLIT, fCMPL, OR, SWAP, CSTOR, EXIT);
    _COLON("IMMEDIATE",    vLAST, AT, DUP,              /// enable IMMEDIATE flag
        CAT, DOLIT, fIMMD, OR, SWAP, CSTOR, EXIT);
    ///
    ///> Arduino specific opcodes
    ///
    IU CLK = _PRIM("CLOCK", CLK  ); ///  ( -- ud ud ) get current clock (in ms)
    _PRIM("PINMODE",  PIN  );       ///  ( n p -- )   set pinMode(p, n=1:OUTPUT, n=0: INPUT)
    _PRIM("MAP",      MAP  );       ///  ( h l p -- ) set map range to pin
    _PRIM("IN",       IN   );       ///  ( p -- n )   digitalRead(p)
    _PRIM("OUT",      OUT  );       ///  ( n p -- )   digitialWrite(p, n=1 HIGH, n=0 LOW)
    _PRIM("AIN",      AIN  );       ///  ( p -- n )   read analog value from pin
    _PRIM("PWM",      PWM  );       ///  ( n p -- )   set duty cycle % (PWM) to pin
    _PRIM("TMISR",    TMISR);       ///  ( xt n -- )  on timer interrupt calls xt every n ms
    _PRIM("PCISR",    PCISR);       ///  ( xt p -- )  on pin change interrupt calls xt
    _PRIM("TIMER",    TMRE );       ///  ( f -- )     enable/disable timer interrupt
    _PRIM("PCINT",    PCIE );       ///  ( f -- )     enable/disable pin change interrupt
    _COLON("DELAY", S2D, CLK, DADD, vTMP, DSTOR); {
        _BEGIN(vTMP, DAT, CLK, DSUB, ZLT, SWAP, DROP);
        _UNTIL(EXIT);
    }
    ///
    ///> Cold Start address (End of dictionary)
    ///
    int last  = PC + CELLSZ;                /// name field of last word
    IU  COLD  = _COLON("COLD",
            DOLIT, last,  vCNTX, STORE,     /// reset vectors
            DOLIT, last,  vLAST, STORE,
            DOLIT, INTER, vMODE, STORE,
            DOLIT, QUIT,  vTABRT,STORE,
            CR, QUIT);                      /// enter the main query loop (QUIT)
    int here  = PC;                         /// current pointer
    ///
    ///> Boot Vector Setup
    ///
    SET(FORTH_BOOT_ADDR, COLD);

    return here;
}
///
/// create C array dump for ROM
///
void _dump_rom(U8* rom, int len)
{
    printf(
        "///\n"
        "/// @file eforth_rom.c\n"
        "/// @brief eForth ROM (loaded in Arduino Flash Memory)\n"
        "/// @attention 8K max ROM before changing FORTH_ROM_SZ in eforth_core.h \n"
        "///\n"
        "#include \"eforth_core.h\"");
    printf("\nconst U32 forth_rom_sz PROGMEM = 0x%x;", len);
    printf("\nconst U32 forth_rom[] PROGMEM = {\n");
    for (int p=0; p<len+0x20; p+=0x20) {
        U32 *x = (U32*)&rom[p];
        for (int i=0; i<0x8; i++, x++) {
            printf("0x%08x,", *x);
        }
        printf(" // %04x ", p);
        for (int i=0; i<0x20; i++) {
            U8 c = rom[p+i];
            printf("%c", c ? ((c!=0x5c && c>0x1f && c<0x7f) ? c : '_') : '.');
        }
        printf("\n");
    }
    printf("};\n");
}

#if !ARDUINO
static U8 _rom[FORTH_ROM_SZ] = {};            ///< fake rom to simulate run time
int main(int ac, char* av[]) {
    setvbuf(stdout, NULL, _IONBF, 0);         /// * autoflush (turn STDOUT buffering off)

    int sz = assemble(_rom);

    _dump_rom(_rom, sz);

    return 0;
}
#endif /// !ARDUINO
