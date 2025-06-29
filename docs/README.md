### eForth1 Virtual Machine
#### Memory Map
Arduino IDE load the compiled eForth1 onto UNO/Nano flash memory. Unlike 328eForth, it does not overwrite the bootloader. The following memory map is logical instead of the physical address of ATmega328p processor. Built-in words are somewhere inside the 16K flash memory functions as a ROM, and the user defined words and other data parts stay in static RAM. They are stitched togather by eForth1 virtually as one continuous memory map.

  | Address       | Size (B) | Store | Desc                   | Save to EEPROM |
  |:--------------|:---------|:------|:-----------------------|----------------|
  | 0x0000-0x1FFF | 8K       | Flash | built-in words         |                |
  | 0x2000-0x201F | 32       | SRAM  | user variables         | Yes            |
  | 0x2020-0x23FF | 1K - 32  | SRAM  | user colon words       | Yes            |
  | 0x2400-0x247F | 128      | SRAM  | data and return stacks |                |
  | 0x2480-0x24FF | 128      | SRAM  | input buffer           |                |

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

  | Forth Standard Words                                | parameters     | function         |
  |:----------------------------------------------------|:---------------|:-----------------|
  | **Data Stack words**                                |                |                  |
  | <pre>DUP  DROP  SWAP  OVER  ROT  PICK</pre>         |                |                  |
  | <pre>?DUP  DEPTH  S0  SP@</pre>                     |                |                  |
  |                                                     |                |                  |
  | **Arithmetic words**                                |                |                  |
  | <pre>+  -  *  /  MOD  MAX  MIN</pre>                | ( a b -- c )   | binary ops       |
  | <pre>ABS  NEGATE  LSHIFT  RSHIFT</pre>              | ( a -- a' )    | unitary ops      |
  | <pre>1+  1-  2+  2-  2*  2/</pre>                   | ( a -- a' )    | constant ops     |
  | <pre>UM/MOD  UM*  M*  UM+  */MOD  /MOD  */</pre>    |                | multi-oprand ops |
  |                                                     |                |                  |
  | **Binary and Logic words**                          |                |                  |
  | <pre>AND  OR  XOR  INVERT</pre>                     | ( a -- a' )    | unitary ops      |
  | <pre>0> 0= 0<</pre>                                 | ( a -- f )     |                  |
  | <pre>>  =  <  U<</pre>                              | ( a b -- f )   |                  |
  | <pre>WITHIN</pre>                                   | ( L H a -- f ) | trinary op       |
  |                                                     |                |                  |
  | **IO words**                                        |                |                  |
  | <pre>?KEY  KEY  >CHAR  CHARS</pre>                  |                |                  |
  | <pre>EMIT  TYPE  SPACE  SPACES CR  .</pre>          |                |                  |
  |                                                     |                |                  |
  | **Branching and Return Stack words**                |                |                  |
  | <pre>IF  ELSE  THEN</pre>                           |                |                  |
  | <pre>BEGIN  AGAIN  UNTIL  WHILE  WHEN  REPEAT</pre> |                |                  |
  | <pre>FOR  AFT  NEXT</pre>                           |                |                  |
  | <pre>I  R>  R@  >R  RP</pre>                        |                |                  |
  |                                                     |                |                  |
  | **Word Defining and Compiler words**                |                |                  |
  | <pre>:  ;  CODE  CREATE  DOES>  '</pre>             |                |                  |
  | <pre>FIND  WORD  AHEAD  LITERAL  PARSE</pre>        |                |                  |
  | <pre>[  ]  [COMPILE]  COMPILE  IMMEDIATE</pre>      |                |                  |
  | <pre>VARIABLE  CONSTANT</pre>                       |                |                  |
  | <pre>2VARAIBLE  2CONSTANT</pre>                     |                |                  |
  | <pre>QUIT  ABORT</pre>                              |                |                  |
  |                                                     |                |                  |
  | **Memory Management words**                         |                |                  |
  | !  +!  @  C!  C@  ,(comma)  C  ?                    |                |                  |
  | ALLOT  CMOVE  MOVE  FILL                            |                |                  |
  |                                                     |                |                  |
  | **Output Formatting words**                         |                |                  |
  | <#  HOLD  #  #S  SIGN  #>  STR                      |                |                  |
  | .R  U.R  U.                                         |                |                  |
  |                                                     |                |                  |
  | **Commenting words**                                |                |                  |
  | .(  \  (                                            |                |                  |
  |                                                     |                |                  |
  | **String words***                                   |                |                  |
  | $"  ."                                              |                |                  |
  | ACCEPT                                              |                |                  |
  |                                                     |                |                  |
  | **Misc. words**                                     |                |                  |
  | BL  CELL  COUNT  CELL+  CELL-  CELLS                |                |                  |
  |                                                     |                |                  |
  | **Double Precision words**                          |                |                  |
  | DNEGATE  D+  D-                                     |                |                  |
  | 2!  2@  2DUP  2DROP  2SWAP  2OVER                   |                |                  |
  | S>D  D>S                                            |                |                  |
  |                                                     |                |                  |
  | **Debugging Tools words**                           |                |                  |
  | HERE  HEX  DECIMAL  DUMP  WORDS  SEE                |                |                  |
  | FORGET  TRACE  BYE                                  |                |                  |
  |                                                     |                |                  |

### eForth1 specific - for parsing and system interface

  | eForth1 specific                                 | Parameters | function |
  |:-------------------------------------------------|:-----------|:---------|
  | **String Processing words**                      |            |          |
  | do$  $\|  ."\|                                   |            |          |
  |                                                  |            |          |
  | **Primitive words**                              |            |          |
  | NOP  ENTER  EXIT  EXECUTE                        |            |          |
  | DOLIT  DOVAR                                     |            |          |
  | QBRANCH  BRANCH  DONEXT                          |            |          |
  |                                                  |            |          |
  | **Outer Interpreter and Parser words**           |            |          |
  | >UPPER  DIGIT  EXTRACT  DIGIT?  NUMBER?  (parse) |            |          |
  | PACK$  TOKEN  NAME?  NAME>  SAME?  >NAME  $,n    |            |          |
  | ^H  TAP  kTAP  EXPECT  ?UNIQUE                   |            |          |
  | $INTERPRET  $COMPILE  EVAL  COMPILE-ONLY         |            |          |
  | .ADDR  .OP  .OK                                  |            |          |
  | QUERY  ERROR  COLD                               |            |          |

