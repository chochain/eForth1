### eForth1 Source Code

    ~/eforth1.ino - example eForth1 wrapper
    ~/src         - Arduino eForth1 library package
        + eForth1.h         - interface for Arduino .ino
        + eforth_config.h   - universal types and sizing
        + eforth_asm.h      - C-based macro assembler
        + eforth_asm.c      - Forth meta-compiler to build eforth_rom.c
        + eforth_rom.c      - eForth1 built-in words from eforth_asm.c
        + eforth_core.h     - definition for interrupt handlers and system interface
        + eforth_core.cpp   - interrupt handler, system function implementation
        + eforth_vm.h       - definition for eForth1 Virtual Machine
        + eforth_vm.cpp     - eForth1 VM implementation
        + eforth1.cpp       - eForth1 library main entry point

### eForth1 Virtual Machine
#### Memory Map
Arduino IDE load the compiled eForth1 onto UNO/Nano flash memory. Unlike 328eForth, it does not overwrite the bootloader. The following memory map is logical instead of the physical address of ATmega328p processor. Built-in words are somewhere inside the 16K flash memory functions as a ROM, and the user defined words and other data parts stay in static RAM. They are stitched togather by eForth1 virtually as one continuous memory map.

  | Address       | Size (B) | Store | Desc                   | Save to EEPROM |
  |:--------------|:---------|:------|:-----------------------|----------------|
  | 0x0000-0x1fff | 8K       | Flash | built-in words         |                |
  | 0x2000-0x201f | 32       | SRAM  | user variables         | Yes            |
  | 0x2020-0x23ff | 1K - 32  | SRAM  | user colon words       | Yes            |
  | 0x2400-0x247f | 128      | SRAM  | data and return stacks |                |
  | 0x2480-0x24ff | 128      | SRAM  | input buffer           |                |

Note: currently, built-in words (defined in eforth_asm.c) occupied only 3.8K. Many more can be added.
        
#### Dictionary - Indirect Threaded (built-in token, colon subroutine address)

    +-------------+      +-------------+      +-------------+
    | Dictionary  |      | Entry 1     |      | Entry 2     |
    +-------------+      +-------------+      +-------------+
    | Entry 1     | ---> | Link Field  | ---> | Link Field  | ---> ...
    | (latest)    |      | Name Field  |      | Name Field  |
    |             |      | Code Field  |      | Code Field  |
    |             |      | Parameter   |      | Parameter   |
    |             |      | Field       |      | Field       |
    +-------------+      +-------------+      +-------------+

#### User Variables

    | User Variable | Address | Init Value | Function                                |
    |:--------------|:--------|:-----------|:----------------------------------------|
    | 'TIB          | 0x2000  | 0x2080     | Pointer to Terminal Input Buffer        |
    | BASE          | 0x2002  | 0x10       | Numeric Radix                           |
    | CP            | 0x2004  | 0x2020     | Top of dictionary                       |
    | CONTEXT       | 0x2006  | 0xEB3      | Pointer to name field of last word      |
    | LAST          | 0x2008  | 0xEB3      | Pointer to name field of last word      |
    | 'MODE         | 0x200A  | 0x883      | Pointer to Compiler or Interpreter      |
    | 'ABORT        | 0x200C  | 0x908      | Pointer to QUIT word, error handler     |
    | HLD           | 0x200E  | 0x24FF     | Pointer to text buffer for number       |
    | SPAN          | 0x2010  | 0x0        | Number of input characters              |
    | >IN           | 0x2012  | 0x0        | Pointer to next input character         |
    | #TIB          | 0x2014  | 0x0        | Number of character received from input |
    | tmp           | 0x2016  | 0x0        | Scatch pad                              |
    
### Standard Built-in Words - for details, reference [Forth Standard](https://forth-standard.org/)
#### Data Stack

    | DUP, DROP, SWAP, OVER, ROT, PICK |   |   |
    | ?DUP, DEPTH, S0, SP@             |   |   |

#### Arithmetic

    | +, -, *, /, MOD, MAX, MIN             | ( a b -- c ) | binary ops       |
    | ABS, NEGATE, LSHIFT, RSHIFT           | ( a -- a' )  | unitary ops      |
    | 1+, 1-, 2+, 2-, 2*, 2/                | ( a -- a' )  | constant ops     |
    | UM/MOD, UM*, M*, UM+, */MOD, /MOD, */ |              | multi-oprand ops |

#### Binary and Logic

    | AND, OR, XOR, INVERT    |   |   |
    | >, =, <, 0>, 0=, 0<, U< |   |   |

#### IO

    | ?KEY, EMIT, KEY, >CHAR, SPACE, CHARS, SPACES |   |   |
    | TYPE, CR, .                                  |   |   |

#### Branching and Return Stack

    | IF, ELSE, THEN                           |   |   |
    | BEGIN, AGAIN, UNTIL, WHILE, WHEN, REPEAT |   |   |
    | FOR, AFT, NEXT                           |   |   |
    | I, R>, R@, >R, RP                        |   |   |

#### Word Defining

    | :, ;, CODE, CREATE, DOES>, ', IMMEDIATE |   |   |
    | VARIABLE, CONSTANT                      |   |   |
    | 2VARAIBLE, 2CONSTANT                    |   |   |

#### Memory Access/Management

    | !, +!, @, C!, C@, ,(comma), C, ? |   |   |
    | ALLOT, CMOVE, MOVE, FILL         |   |   |

#### Output Formatting

    | <#, HOLD, #, #S, SIGN, #>, STR |   |   |
    | .R, U.R, U.                    |   |   |

#### Comments

    | .(, \, ( |   |   |

#### String

    | $", ." |   |   |

#### Misc. 

    | BL, CELL, COUNT, CELL+, CELL-, CELLS | | |

#### Double Precision

    | DNEGATE, D+, D-                   |   |   |
    | 2!, 2@, 2DUP, 2DROP, 2SWAP, 2OVER |   |   |
    | S>D, D>S                          |   |   |

#### Debugging Tools

    | HERE, HEX, DECIMAL, DUMP, WORDS, SEE |   |   |
    | FORGET, TRACE, BYE                   |   |   |

### eForth1 specific - for parsing and system interface
##### String Processing

    | do$, $\|, ." |   |   |

#### Primitives

    | NOP, EXIT, ENTER, DOLIT, DOVAR, QBRANCH, BRANCH, DONEXT, EXECUTE |   |   |

#### Outer Interpreter and Parser

   | >UPPER, DIGIT, EXTRACT, DIGIT?, NUMBER?, (parse),   |   |   |
   | AHEAD, PACK$, PARSE, TOKEN, WORD, NAME>, SAME?      |   |   |
   | LITERAL, FIND, NAME?, ^H, TAP, kTAP, ACCEPT, EXPECT |   |   |
   | $INTERPRET, $COMPILE, [, ], [COMPILE], COMPILE      |   |   |
   | .ADDR, .OP, .OK, ?UNIQUE, $,n, >NAME                |   |   |
   | EVAL, QUIT, QUERY, ABORT, ERROR, COLD, COMPILE-ONLY |   |   |



