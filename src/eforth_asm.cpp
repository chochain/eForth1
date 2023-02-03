/**
 * @file
 * @brief eForth Assembler module
 *
 * Forth Macro Assembler
 * Note: TODO: adding Leo Brodie's words
 */
#include "eforth_asm.h"

#define WORDS_ROW_WIDTH 0x44        /** WORDS row width control */

namespace EfAsm {
///
///@name eForth Assembler module variables
///@{
U8 *_byte;                          ///< assembler byte array (heap)
IU _link;                           ///< link to previous word
U8 R;                               ///< assembler return stack index
IU PC;                              ///< assembler program counter
///@}
///@name Compiled Address for Branching
///@{
IU DOTQP, STRQP, ABORQP;            ///< addr of output ops, used by _dotq, _strq, _abortq
///@}
///
///> eForth Macro Assembler
///
int assemble(U8 *cdata)
{
    _byte = cdata;
    _link = R = 0;
    ///
    ///> Cold boot vector
    ///
    PC = FORTH_BOOT_ADDR;
    IU BOOT  = _LABEL(0xfeed);           /// reserved for COLD boot vector
    ///
    ///> Kernel dictionary (primitive words)
    ///
    IU NOP   = _XCODE("NOP",     NOP    );
    IU BYE   = _XCODE("BYE",     BYE    );
    IU QRX   = _XCODE("?RX",     QRX    );
    IU TXSTO = _XCODE("TX!",     TXSTO  );
    IU DOLIT = _XCODE("DOLIT",   DOLIT  );
    IU DOVAR = _XCODE("DOVAR",   DOVAR  );
    IU ENTER = _XCODE("ENTER",   ENTER  );  //alias doLIST
    IU EXIT  = _XCODE("EXIT",    EXIT   );
    IU QBRAN = _XCODE("QBRANCH", QBRAN  );
    IU BRAN  = _XCODE("BRANCH",  BRAN   );
    IU DONXT = _XCODE("DONEXT",  DONEXT );
    IU EXECU = _XCODE("EXECUTE", EXECU  );
    IU STORE = _XCODE("!",       STORE  );
    IU PSTOR = _XCODE("+!",      PSTOR  );
    IU AT    = _XCODE("@",       AT     );
    IU CSTOR = _XCODE("C!",      CSTOR  );
    IU CAT   = _XCODE("C@",      CAT    );
    IU RFROM = _XCODE("R>",      RFROM  );
    IU RAT   = _XCODE("R@",      RAT    );
    IU TOR   = _XCODE(">R",      TOR    );
    IU DROP  = _XCODE("DROP",    DROP   );
    IU DUP   = _XCODE("DUP",     DUP    );
    IU SWAP  = _XCODE("SWAP",    SWAP   );
    IU OVER  = _XCODE("OVER",    OVER   );
    IU ROT   = _XCODE("ROT",     ROT    );
    IU PICK  = _XCODE("PICK",    PICK   );
    IU AND   = _XCODE("AND",     AND    );
    IU OR    = _XCODE("OR",      OR     );
    IU XOR   = _XCODE("XOR",     XOR    );
    IU INV   = _XCODE("INVERT",  INV    );
    IU LSH   = _XCODE("LSHIFT",  LSH    );
    IU RSH   = _XCODE("RSHIFT",  RSH    );
    IU ADD   = _XCODE("+",       ADD    );
    IU SUB   = _XCODE("-",       SUB    );
    IU MUL   = _XCODE("*",       MUL    );
    IU DIV   = _XCODE("/",       DIV    );
    IU MOD   = _XCODE("MOD",     MOD    );
    IU NEG   = _XCODE("NEGATE",  NEG    );
    IU GT    = _XCODE(">",       GT     );
    IU EQ    = _XCODE("=",       EQ     );
    IU LT    = _XCODE("<",       LT     );
    IU ZGT   = _XCODE("0>",      ZGT    );
    IU ZEQ   = _XCODE("0=",      ZEQ    );
    IU ZLT   = _XCODE("0<",      ZLT    );
    IU ONEP  = _XCODE("1+",      ONEP   );
    IU ONEM  = _XCODE("1-",      ONEM   );
    IU QDUP  = _XCODE("?DUP",    QDUP   );
    IU DEPTH = _XCODE("DEPTH",   DEPTH  );
    IU ULESS = _XCODE("U<",      ULESS  );
    /// TODO: add UM+
    IU UMMOD = _XCODE("UM/MOD",  UMMOD  );
    IU UMSTA = _XCODE("UM*",     UMSTAR );
    IU MSTAR = _XCODE("M*",      MSTAR  );
    IU DNEG  = _XCODE("DNEGATE", DNEG   );
    IU DADD  = _XCODE("D+",      DADD   );
    IU DSUB  = _XCODE("D-",      DSUB   );
    IU RP    = _XCODE("RP",      RP     );
    ///
    /// Kernel constants
    ///
    IU ua    = FORTH_UVAR_ADDR;
    IU vTTIB = _CODE("'TIB",    CST(ua,0));   ///> * 'TIB console input buffer pointer
    IU vBASE = _CODE("BASE",    CST(ua,1));   ///> * BASE current radix for numeric ops
    IU vCP   = _CODE("CP",      CST(ua,2));   ///> * CP,  top of dictionary, same as HERE
    IU vCNTX = _CODE("CONTEXT", CST(ua,3));   ///> * CONTEXT name field of last word
    IU vLAST = _CODE("LAST",    CST(ua,4));   ///> * LAST, same as CONTEXT
    IU vMODE = _CODE("'MODE",   CST(ua,5));   ///> * 'MODE ('TEVAL - interpreter or compiler)
    IU vTABRT= _CODE("'ABORT",  CST(ua,6));   ///> * ABORT exception rescue handler (QUIT)
    IU vHLD  = _CODE("HLD",     CST(ua,7));   ///> * HLD  char pointer to output buffer
    IU vSPAN = _CODE("SPAN",    CST(ua,8));   ///> * SPAN number of character accepted
    IU vIN   = _CODE(">IN",     CST(ua,9));   ///> * >IN  interpreter pointer to next char
    IU vNTIB = _CODE("#TIB",    CST(ua,10));  ///> * #TIB number of character received in TIB
    IU vTMP  = _CODE("tmp",     CST(ua,11));  ///> * tmp storage (alternative to return stack)
    ///
    ///> common constants and variable spec.
    ///
    IU BLANK = _CODE("BL",      CST(0x20,  0));  ///> * BL blank
    IU CELL  = _CODE("CELL",    CST(CELLSZ,0));  ///> * CELL cell size
    ///
    ///> Common High-Level Colon Words
    ///
    IU CELLP = _COLON("CELL+", CELL, ADD, EXIT);
    IU CELLM = _COLON("CELL-", CELL, SUB, EXIT);
    IU CELLS = _COLON("CELLS", CELL, MUL, EXIT);
    // Dr. Ting's alternate opcodes
    IU DDUP  = _COLON("2DUP",  OVER, OVER, EXIT);
    IU DDROP = _COLON("2DROP", DROP, DROP, EXIT);
    /// TODO: add 2SWAP, 2OVER, 2+, 2-, 2*, 2/
    IU D2S   = _COLON("D>S",   ZLT, OVER, ZLT, XOR); {
        _IF(NEG);
        _THEN(EXIT);
    }
    IU S2D   = _COLON("S>D",   DUP, ZLT); {
        _IF(DOLIT, 0xffff);
        _ELSE(DOLIT, 0);
        _THEN(EXIT);
    }
    /// TODO: add I, J
    IU SSMOD = _COLON("*/MOD", TOR, MSTAR, RFROM, UMMOD, EXIT);
    IU SMOD  = _COLON("/MOD", DDUP, DIV, TOR, MOD, RFROM, EXIT);     // Leo B. has it
    IU MSLAS = _COLON("*/",   SSMOD, SWAP, DROP, EXIT);
    IU DSTOR = _COLON("2!",   DUP, TOR, CELL, ADD, STORE, RFROM, STORE, EXIT);
    IU DAT   = _COLON("2@",   DUP, TOR, AT, RFROM, CELL, ADD, AT, EXIT);
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
    ///
    ///> Console Input and Common words
    ///
    IU QKEY  = _COLON("?KEY",  QRX, EXIT);
    IU KEY   = _COLON("KEY",   NOP); {
        _BEGIN(QKEY);
        _UNTIL(EXIT);
    }
    IU WITHI = _COLON("WITHIN",OVER, SUB, TOR, SUB, RFROM, ULESS, EXIT);
    IU TCHAR = _COLON(">CHAR", DOLIT, 0x7f, AND, DUP, DOLIT, 0x7f, BLANK, WITHI); {
        _IF(DROP, DOLIT, 0x5f);
        _THEN(EXIT);
    }
    IU HERE  = _COLON("HERE",  vCP, AT, EXIT);                // top of dictionary
    IU PAD   = _COLON("PAD",   DOLIT, FORTH_MAX_ADDR, EXIT);  // use tail of TIB for output
    IU TIB   = _COLON("TIB",   vTTIB, AT, EXIT);
    /// TODO: add SP@, SP0
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
    IU HEX_  = _COLON("HEX",     DOLIT, 16, vBASE, STORE, EXIT);
    IU DECIM = _COLON("DECIMAL", DOLIT, 10, vBASE, STORE, EXIT);
    IU TOUPP = _COLON(">UPPER", DUP, DOLIT, 0x61, DOLIT, 0x7b, WITHI); { // [a-z] only?
        _IF(DOLIT, 0x5f, AND);
        _THEN(EXIT);
    }
    IU DIGTQ = _COLON("DIGIT?", TOR, TOUPP, DOLIT, 0x30, SUB, DOLIT, 9, OVER, LT); {
        _IF(DOLIT, 7, SUB, DUP, DOLIT, 10, LT, OR);           // handle hex number
        _THEN(DUP, RFROM, ULESS, EXIT);                       // handle base > 10
    }
    /// TODO: add >NUMBER
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
    ///> Console Output
    ///
    IU EMIT  = _COLON("EMIT",  TXSTO, EXIT);
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
            _IF(DROP, DOLIT, 0x5f);    // out-of-range put '_' instead
            _THEN(EMIT);
        }
        _THEN(NOP);
        _NEXT(DROP, EXIT);
    }
    IU CR    = _COLON("CR",   DOLIT, 10, EMIT, EXIT);
    // IU CR    = _COLON("CR",   DOLIT, 10, DOLIT, 13, EMIT, EMIT, EXIT);   // LFCR
    IU DOSTR = _COLON("do$",  RFROM, RAT, RFROM, COUNT, ADD, TOR, SWAP, TOR, EXIT);
       STRQP = _COLON("$\"|", DOSTR, EXIT);
       DOTQP = _COLON(".\"|", DOSTR, COUNT, TYPE, EXIT);
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
    /// TODO: add PAGE
    ///
    ///> Parsing
    ///
    IU PARSE0= _COLON("(parse)", vTMP, CSTOR, OVER, TOR, DUP); {   // ( addr len delim -- ) delimiter kept tmp, addr in R
        _IF(ONEM, vTMP, CAT, BLANK, EQ); {                    // if (len) { --len; if (delim==" ") {...} }
            _IF(NOP); {
                // a FOR..WHILE..NEXT..THEN construct =~ for {..break..}
                _FOR(BLANK, OVER, CAT, SUB, ZLT, INV);        // for (len)
                _WHILE(ONEP);                                 // break to THEN if is char, or next char
                _NEXT(RFROM, DROP, DOLIT, 0, DUP, EXIT);      // no break, (R>, DROP to rm loop counter)
                _THEN(RFROM);                                 // populate A0, i.e. break comes here, rm counter
            }
            _THEN(OVER, SWAP); {                              // advance until next space found
                // a FOR..WHILE..NEXT..ELSE..THEN construct =~ DO..LEAVE..+LOOP
                _FOR(vTMP, CAT, OVER, CAT, SUB, vTMP, CAT, BLANK, EQ); {
                  _IF(ZLT);
                  _THEN(NOP);
                }
                _WHILE(ONEP);                                  // if (char <= space) break to ELSE
                _NEXT(DUP, TOR);                               // no break, if counter < limit loop back to FOR
                _ELSE(RFROM, DROP, DUP, ONEP, TOR);            // R>, DROP to rm loop counter
               _THEN(OVER, SUB, RFROM, RFROM, SUB, EXIT);      // put token length on stack
            }
        }
        _THEN(OVER, RFROM, SUB, EXIT);
    }
    IU PACKS = _COLON("PACK$", DUP, TOR, DDUP, CSTOR, ONEP, SWAP, CMOVE, RFROM, EXIT);     // ( b u a -- a )
    IU PARSE = _COLON("PARSE",                                                             // ( c -- b u )
                       TOR, TIB, vIN, AT, ADD, vNTIB, AT, vIN, AT, SUB, RFROM,
                       PARSE0, vIN, PSTOR,
                       EXIT);
    IU TOKEN = _COLON("TOKEN", BLANK, PARSE, DOLIT, 0x1f, MIN, HERE, CELLP, PACKS, EXIT);  // ( -- a; <string>) put token at HERE
    IU WORD  = _COLON("WORD",  PARSE, HERE, CELLP, PACKS, EXIT);                           // ( c -- a; <string)
    ///
    ///> Dictionary serach
    ///
    IU NAMET = _COLON("NAME>", COUNT, DOLIT, 0x1f, AND, ADD, EXIT);        // ( nfa -- cfa )
    IU SAMEQ = _COLON("SAME?", NOP); {                                     // ( a1 a2 n - a1 a2 f ) compare a1, a2 byte-by-byte
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
        _NEXT(DDROP, DOLIT, FALSE, EXIT);                       // SAME!
    }
    /// TODO: add COMPARE
    IU FIND = _COLON("FIND", SWAP, DUP, CAT, vTMP, STORE,       // ( a va -- cfa nfa, a F ) keep length in tmp
                     DUP, AT, TOR, CELLP, SWAP); {              // fetch 1st cell
        _BEGIN(AT, DUP); {                                      // 0000 = end of dic
#if CASE_SENSITIVE
            _IF(DUP, AT, DOLIT, 0x3fff, AND, RAT, XOR); {       // compare 2-byte
#else
            _IF(DUP, AT, DOLIT, 0x3fff, AND,
                    DOLIT, 0x5f5f, AND, RAT,
                    DOLIT, 0x5f5f, AND, XOR); {                 // compare 2-byte (uppercased)
#endif // CASE_SENSITIVE
                _IF(CELLP, DOLIT, TRUE);                        // miss, try next word
                _ELSE(CELLP, vTMP, AT, ONEM, DUP); {            // -1, since 1st byte has been compared
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
    ///> Terminal Input
    ///
    IU HATH  = _COLON("^H", TOR, OVER, RFROM, SWAP, OVER, XOR); {
        _IF(DOLIT, 8, EMIT, ONEM, BLANK, EMIT, DOLIT, 8, EMIT);
        _THEN(EXIT);
    }
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
    IU ABORT  = _COLON("ABORT", vTABRT, AT, QDUP); {
            _IF(EXECU);                              // @EXECUTE
            _THEN(EXIT);
        }
    ABORQP = _COLON("abort\"", NOP); {
        _IF(DOSTR, COUNT, TYPE, ABORT);
        _THEN(DOSTR, DROP, EXIT);
    }
    /// TODO: add ?STACK
    IU ERROR = _COLON("ERROR", SPACE, COUNT, TYPE, DOLIT, 0x3f, EMIT, CR, ABORT);
    IU INTER = _COLON("$INTERPRET", NAMEQ, QDUP); {  // scan dictionary for word
        _IF(CAT, DOLIT, fCMPL, AND); {               // check for compile only word
            _IF(NOP); {
                _ABORTQ(" compile only");
            }
            _ELSE(EXECU);                            // INTER0 of Dr Ting's
            _THEN(EXIT);
        }
        _THEN(NUMBQ);                                // word name not found, check if it is a number
        _IF(EXIT);
        _ELSE(ERROR);
        _THEN(NOP);
    }
    IU iLBRAC= _IMMED("[", DOLIT, INTER, vMODE, STORE, EXIT);
    IU DOTOK = _COLON(".OK", CR, DOLIT, INTER, vMODE, AT, EQ); {
        _IF(DEPTH, DOLIT, 4, MIN); {                 // dump stack and ok prompt
            _FOR(RAT, PICK, DOT);
            _NEXT(NOP);
            _DOTQ(" ok> ");
        }
        _THEN(EXIT);
    }
    IU EVAL  = _COLON("EVAL", NOP); {
        _BEGIN(TOKEN, DUP, CAT);                     // fetch token length
        _WHILE(vMODE, AT, QDUP); {                   // fetch operation mode ($INTERPRET or $COMPILE)
            _IF(EXECU);                              // execute according to mode
            _THEN(NOP);
        }
        _REPEAT(DROP, DOTOK, EXIT);
    }
    IU QUIT = _COLON("QUIT", DOLIT, FORTH_TIB_ADDR, vTTIB, STORE, iLBRAC); {  // clear TIB, interpreter mode
        _BEGIN(QUERY, EVAL);                         // main query-eval loop
        _AGAIN(NOP);
    }
    ///
    ///> Colon Compiler
    ///
    IU COMMA = _COLON(",",  HERE, DUP, CELLP, vCP, STORE, STORE, EXIT);   // store a byte
    IU CCMMA = _COLON("C,", HERE, DUP, ONEP,  vCP, STORE, CSTOR, EXIT);   // store a word
    IU iLITR = _IMMED("LITERAL",  DOLIT, DOLIT, CCMMA, COMMA, EXIT);      // create a literal
    IU ALLOT = _COLON("ALLOT",    vCP, PSTOR, EXIT);
    IU COMPI = _COLON("COMPILE",  RFROM, DUP, CAT, CCMMA, ONEP, TOR, EXIT);
    IU SCOMP = _COLON("$COMPILE", NAMEQ, QDUP); {      // name found?
        _IF(CAT, DOLIT, fIMMD, AND); {                 // is immediate?
            _IF(EXECU);                                // execute
            _ELSE(DUP, DUP, DOLIT, FORTH_ROM_SZ, LT,   // a primitive?
            	  SWAP, ONEP, CAT, DOLIT, EXIT, EQ, AND); {
                _IF(CAT, CCMMA);                       // append just the opcode
                _ELSE(DOLIT, 0x8000, OR, COMMA);       // append colon word address with flag
                _THEN(NOP);
            }
            _THEN(EXIT);
        }
        _THEN(NUMBQ);                                  // a number?
        _IF(iLITR, EXIT);                              // append as a literal
        _THEN(ERROR);
    }
    IU UNIQU = _COLON("?UNIQUE", DUP, NAMEQ, QDUP); {
        _IF(COUNT, DOLIT, 0x1f, AND, SPACE, TYPE); {
            _DOTQ(" reDef");
        }
        _THEN(DROP, EXIT);
    }
    IU SNAME = _COLON("$,n", DUP, AT); {          // add new name field which is already build by PACK$
        _IF(UNIQU,
            DUP, NAMET, vCP, STORE,    DUP, vLAST, STORE,
            CELLM, vCNTX, AT, SWAP, STORE, EXIT);
        _THEN(ERROR);
    }
    IU TICK  = _COLON("'", TOKEN, NAMEQ); {
        _IF(EXIT);
        _THEN(ERROR);
    }
    IU RBRAC = _COLON("]", DOLIT, SCOMP, vMODE, STORE, EXIT);       // switch into compiler-mode
    /// TODO: add [']
    _IMMED("[COMPILE]",    TICK, COMMA, EXIT);                      // add word address to dictionary
    _COLON(":", TOKEN, SNAME, RBRAC, EXIT);
    _IMMED(";", COMPI, EXIT, iLBRAC, vLAST, AT, vCNTX, STORE, EXIT);
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
    _COLON("WORDS", CR, vCNTX, DOLIT, 0, vTMP, STORE); {
        _BEGIN(AT, QDUP);
        _WHILE(DUP, COUNT, DOLIT, 0x1f, AND,            // .ID
            DUP, DOLIT, 2, ADD, vTMP, PSTOR,
            TYPE, SPACE, SPACE, CELLM,
            vTMP, AT, DOLIT, WORDS_ROW_WIDTH, GT); {    // check row width
            _IF(CR, DOLIT, 0, vTMP, STORE);
            _THEN(NOP);
        }
        _REPEAT(EXIT);
    }
    _COLON("FORGET", TOKEN, NAMEQ, QDUP); {
        _IF(CELLM, DUP, vCP, STORE, AT, DUP, vCNTX, STORE, vLAST, STORE, DROP, EXIT);
        _THEN(ERROR);
    }
    ///
    ///> Control Structures
    ///
    ///> * BEGIN...AGAIN, BEGIN... f UNTIL, BEGIN...(once)...f WHILE...(loop)...REPEAT
    ///
    IU iAHEAD = _IMMED("AHEAD", COMPI, BRAN, HERE, DOLIT, 0, COMMA, EXIT);
    IU iAGAIN = _IMMED("AGAIN", COMPI, BRAN, COMMA, EXIT);
    _IMMED("BEGIN",   HERE, EXIT);
    _IMMED("UNTIL",   COMPI, QBRAN, EXIT);
    ///
    ///> * f IF...THEN, f IF...ELSE...THEN
    ///
    IU iIF    = _IMMED("IF",   COMPI, QBRAN, HERE, DOLIT, 0, COMMA, EXIT);
    IU iTHEN  = _IMMED("THEN", HERE, SWAP, STORE, EXIT);
    _IMMED("ELSE",    iAHEAD, SWAP, iTHEN, EXIT);
    _IMMED("WHILE",   iIF, SWAP, EXIT);
    _IMMED("WHEN",    iIF, OVER, EXIT);
    _IMMED("REPEAT",  iAGAIN, iTHEN, EXIT);
    ///
    ///> * n FOR...NEXT, n FOR...(first)... f AFT...(2nd,...)...THEN...(every)...NEXT
    ///
    /// TODO: add DO...LOOP, +LOOP, LEAVE
    ///
    _IMMED("FOR",     COMPI, TOR, HERE, EXIT);
    _IMMED("AFT",     DROP, iAHEAD, HERE, SWAP, EXIT);
    _IMMED("NEXT",    COMPI, DONXT, COMMA, EXIT);
    ///
    ///> String Literals
    ///
    IU STRCQ  = _COLON("$,\"", DOLIT, 0x22, WORD,       // find quote in TIB (0x22 is " in ASCII)
                       COUNT, ADD, vCP, STORE, EXIT);   // advance dic pointer
    _IMMED("ABORT\"", DOLIT, ABORQP, HERE, STORE, STRCQ, EXIT);
    _IMMED("$\"",     DOLIT, STRQP,  HERE, STORE, STRCQ, EXIT);
    _IMMED(".\"",     DOLIT, DOTQP,  HERE, STORE, STRCQ, EXIT);
    ///
    ///> Defining Words - variable, constant, and comments
    ///
    IU CODE  = _COLON("CODE", TOKEN, SNAME, vLAST, AT, vCNTX, STORE, EXIT);
    IU CREAT = _COLON("CREATE", CODE, COMPI, DOVAR, COMPI, EXIT, EXIT);       /// * opDOVAR padded with opEXIT
    /// TODO: add DOES>, POSTPONE
    _COLON("VARIABLE",CREAT, DOLIT, 0, COMMA, EXIT);
    _COLON("CONSTANT",CODE,  DOLIT, DOLIT, CCMMA, DOLIT, EXIT, CCMMA, COMMA, EXIT);
    /// TODO: 2CONSTANT, 2VARIABLE
    ///
    ///> Comments
    ///
    _IMMED(".(", DOLIT, 0x29, PARSE, TYPE, EXIT);  // print til hit ) i.e. 0x29
    _IMMED("\\", DOLIT, 0xa,  WORD,  DROP, EXIT);  // skip til end of line
    _IMMED("(",  DOLIT, 0x29, PARSE, DDROP, EXIT); // skip til )
    ///
    ///> Lexicon Bits
    ///
    _COLON("COMPILE-ONLY", DOLIT, fCMPL, vLAST, AT, PSTOR, EXIT);   // enable COMPILE-ONLY flag
    _COLON("IMMEDIATE",    DOLIT, fIMMD, vLAST, AT, PSTOR, EXIT);   // enable IMMEDIATE flag
    ///
    ///> Arduino specific opcodes
    ///
    IU CLK     = _XCODE("CLOCK",    CLK  ); ///  ( -- ud ud ) get current clock (in ms)
    IU PINMODE = _XCODE("PINMODE",  PIN  ); ///  ( n p -- )   set pinMode(p, n=1:OUTPUT, n=0: INPUT)
    IU MAP     = _XCODE("MAP",      MAP  ); ///  ( h l p -- ) set map range to pin
    IU IN      = _XCODE("IN",       IN   ); ///  ( p -- n )   digitalRead(p)
    IU OUT     = _XCODE("OUT",      OUT  ); ///  ( n p -- )   digitialWrite(p, n=1 HIGH, n=0 LOW)
    IU AIN     = _XCODE("AIN",      AIN  ); ///  ( p -- n )   read analog value from pin
    IU PWM     = _XCODE("PWM",      PWM  ); ///  ( n p -- )   set duty cycle % (PWM) to pin
    IU TMISR   = _XCODE("TMISR",    TMISR); ///  ( xt n -- )  on timer interrupt calls xt every n ms
    IU PCISR   = _XCODE("PCISR",    PCISR); ///  ( xt p -- )  on pin change interrupt calls xt
    IU TIMER   = _XCODE("TIMER",    TMRE ); ///  ( f -- )     enable/disable timer interrupt
    IU PCINT   = _XCODE("PCINT",    PCIE ); ///  ( f -- )     enable/disable pin change interrupt
    IU TRACE   = _XCODE("TRACE",    TRC  ); ///  ( f -- )     enable/disable debug tracing
    IU SAVE    = _XCODE("SAVE",     SAVE ); ///  ( -- )       save user variables and dictionary to EEPROM
    IU LOAD    = _XCODE("LOAD",     LOAD ); ///  ( -- )       restore user variables and dictionary from EERPROM
    _COLON("DELAY", S2D, CLK, DADD, vTMP, DSTOR); {
        _BEGIN(vTMP, DAT, CLK, DSUB, ZLT, SWAP, DROP);
        _UNTIL(EXIT);
    }
    ///
    ///> Cold Start address (End of dictionary)
    ///
    int last  = PC + CELLSZ;                // name field of last word
    IU  COLD  = _COLON("COLD",
            DOLIT, last,  vCNTX, STORE,     // reset vectors
            DOLIT, last,  vLAST, STORE,
            DOLIT, INTER, vMODE, STORE,
            DOLIT, QUIT,  vTABRT,STORE,
            CR, QUIT);                      // enter the main query loop (QUIT)
    int here  = PC;                         // current pointer
    ///
    ///> Boot Vector Setup
    ///
    SET(FORTH_BOOT_ADDR, COLD);

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
