### eForth1 Virtual Machine
eForth1 is built in three parts.

* 1. It compiles eforth_asm.c which metacompile the ROM image of Forth and store it in eforth_rom.c which is pre-build for you.
* 2. The Forth image, treated as an byte array in C, is then compiled with the eForth1 VM library package (i.e. src/eforth1.cpp, src/eforth_vm.cpp, src/eforth_core.cpp, and their associated .h files)
* 3. Arduino IDE, include the eForth1 package downloaded by Library Manager, compiles the eforth1.ino into ATmega328p machine code then load and run on the microcontroller. One can then interact with eForth1 through the Serial Monitor.
    
Note: If you want to add custom words into the Forth image, eforth_rom.c needs to be rebuilt (see Makefile).

#### Memory Map
Arduino IDE load the compiled eForth1 (~13KB) onto UNO/Nano 32K flash memory. Unlike 328eForth, it does not overwrite the bootloader. The following memory map is logical instead of the physical address of ATmega328p processor. Built-in words are stored as an array inside the flash memory and treated as a piece of ROM. On the other hand, user defined words and other data parts occuping about 1.25K of the total 2K static RAM. The "ROM" and "RAM" are virtually stitched togather by eForth1 as one continuous memory space. A true virtual machine.

  | Address       | Size (B) | Store | Desc                    | Save to EEPROM |
  |:--------------|:---------|:------|:------------------------|----------------|
  | 0x0000-0x1FFF | 8K       | Flash | built-in words (as ROM) |                |
  | 0x2000-0x201F | 32       | SRAM  | user variables          | Yes            |
  | 0x2020-0x23FF | 1K - 32  | SRAM  | user colon words        | Yes            |
  | 0x2400-0x247F | 128      | SRAM  | data and return stacks  |                |
  | 0x2480-0x24FF | 128      | SRAM  | input buffer            |                |

Note: currently, built-in words (defined in eforth_asm.c) occupied only 3.8K. Many more custom words can be added if desired.
        
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
    
Note: Though ATmega328p is a Little-Endian processor, eForth1 uses Big-Endian encoding for memory layout with respect to eForth tradition and future interoperability.

#### User Variables

  | User Variable | Address | Init Value | Function                                  |
  |:--------------|:--------|:-----------|:------------------------------------------|
  | 'TIB          | 0x2000  | 0x2080     | Pointer to Terminal Input Buffer          |
  | BASE          | 0x2002  | 0x10       | Numeric Radix                             |
  | CP            | 0x2004  | 0x2020     | Top of dictionary                         |
  | CONTEXT       | 0x2006  | 0xEB3      | Pointer to name field of last word        |
  | LAST          | 0x2008  | 0xEB3      | Pointer to name field of last word        |
  | 'MODE         | 0x200A  | 0x883      | Pointer to Compiler or Interpreter        |
  | 'ABORT        | 0x200C  | 0x908      | Pointer to QUIT word, error handler       |
  | HLD           | 0x200E  | 0x2500     | Pointer to top of text buffer for numbers |
  | SPAN          | 0x2010  | 0x0        | Number of input characters                |
  | >IN           | 0x2012  | 0x0        | Pointer to next input character           |
  | #TIB          | 0x2014  | 0x0        | Number of character received from input   |
  | tmp           | 0x2016  | 0x0        | Scatch pad                                |
    
### Standard Built-in Words - for details, reference [Forth Standard](https://forth-standard.org/)

  | Forth Standard Words                                  | parameters     | function         |
  |:------------------------------------------------------|:---------------|:-----------------|
  | **Data Stack words**                                  |                |                  |
  | <code>DUP  DROP  SWAP  OVER  ROT  PICK</code>         |                |                  |
  | <code>?DUP  DEPTH  S0  SP@</code>                     |                |                  |
  |                                                       |                |                  |
  | **Arithmetic words**                                  |                |                  |
  | <code>+  -  *  /  MOD  MAX  MIN</code>                | ( a b -- c )   | binary ops       |
  | <code>ABS  NEGATE  LSHIFT  RSHIFT</code>              | ( a -- a' )    | unitary ops      |
  | <code>1+  1-  2+  2-  2*  2/</code>                   | ( a -- a' )    | constant ops     |
  | <code>UM/MOD  UM*  M*  UM+  */MOD  /MOD  */</code>    |                | multi-oprand ops |
  |                                                       |                |                  |
  | **Binary and Logic words**                            |                |                  |
  | <code>AND  OR  XOR  INVERT</code>                     | ( a -- a' )    | unitary ops      |
  | <code>0> 0= 0<</code>                                 | ( a -- f )     |                  |
  | <code>>  =  <  U<</code>                              | ( a b -- f )   |                  |
  | <code>WITHIN</code>                                   | ( L H a -- f ) | trinary op       |
  |                                                       |                |                  |
  | **IO words**                                          |                |                  |
  | <code>?KEY  KEY  >CHAR  CHARS</code>                  |                |                  |
  | <code>EMIT  TYPE  SPACE  SPACES CR  .</code>          |                |                  |
  |                                                       |                |                  |
  | **Branching and Return Stack words**                  |                |                  |
  | <code>IF  ELSE  THEN</code>                           |                |                  |
  | <code>BEGIN  AGAIN  UNTIL  WHILE  WHEN  REPEAT</code> |                |                  |
  | <code>FOR  AFT  NEXT</code>                           |                |                  |
  | <code>I  R>  R@  >R  RP</code>                        |                |                  |
  |                                                       |                |                  |
  | **Word Defining and Compiler words**                  |                |                  |
  | <code>:  ;  CODE  CREATE  DOES>  '</code>             |                |                  |
  | <code>FIND  WORD  AHEAD  LITERAL  PARSE</code>        |                |                  |
  | <code>[  ]  [COMPILE]  COMPILE  IMMEDIATE</code>      |                |                  |
  | <code>VARIABLE  CONSTANT</code>                       |                |                  |
  | <code>2VARAIBLE  2CONSTANT</code>                     |                |                  |
  | <code>QUIT  ABORT</code>                              |                |                  |
  |                                                       |                |                  |
  | **Memory Management words**                           |                |                  |
  | <code>!  +!  @  C!  C@  ,(comma)  C  ?</code>         |                |                  |
  | <code>ALLOT  CMOVE  MOVE  FILL</code>                 |                |                  |
  |                                                       |                |                  |
  | **Output Formatting words**                           |                |                  |
  | <code><#  HOLD  #  #S  SIGN  #>  STR</code>           |                |                  |
  | <code>.R  U.R  U.</code>                              |                |                  |
  |                                                       |                |                  |
  | **Commenting words**                                  |                |                  |
  | <code>.(  \  (</code>                                 |                |                  |
  |                                                       |                |                  |
  | **String words***                                     |                |                  |
  | <code>$"  ."</code>                                   |                |                  |
  | <code>ACCEPT</code>                                   |                |                  |
  |                                                       |                |                  |
  | **Misc. words**                                       |                |                  |
  | <code>BL  CELL  COUNT  CELL+  CELL-  CELLS</code>     |                |                  |
  |                                                       |                |                  |
  | **Double Precision words**                            |                |                  |
  | <code>DNEGATE  D+  D-</code>                          |                |                  |
  | <code>2!  2@  2DUP  2DROP  2SWAP  2OVER</code>        |                |                  |
  | <code>S>D  D>S</code>                                 |                |                  |
  |                                                       |                |                  |
  | **Debugging Tools words**                             |                |                  |
  | <code>NOP  HERE  HEX  DECIMAL</code>                  |                |                  |
  | <code>DUMP  WORDS  SEE</code>                         |                |                  |
  | <code>FORGET  TRACE  BYE</code>                       |                |                  |
  |                                                       |                |                  |


### eForth1 specific - for parsing and system interface

  | eForth1 specific                                              | Parameters | function |
  |:--------------------------------------------------------------|:-----------|:---------|
  | **String Processing words**                                   |            |          |
  | <code>do$  $\|  ."\|</code>                                   |            |          |
  |                                                               |            |          |
  | **Primitive words**                                           |            |          |
  | <code>ENTER  EXIT  EXECUTE</code>                             |            |          |
  | <code>DOLIT  DOVAR</code>                                     |            |          |
  | <code>QBRANCH  BRANCH  DONEXT</code>                          |            |          |
  |                                                               |            |          |
  | **Outer Interpreter and Parser words**                        |            |          |
  | <code>>UPPER  DIGIT  EXTRACT  DIGIT?  NUMBER?  (parse)</code> |            |          |
  | <code>PACK$  TOKEN  NAME?  NAME>  SAME?  >NAME  $,n</code>    |            |          |
  | <code>^H  TAP  kTAP  EXPECT  ?UNIQUE</code>                   |            |          |
  | <code>$INTERPRET  $COMPILE  EVAL  COMPILE-ONLY</code>         |            |          |
  | <code>.ADDR  .OP  .OK</code>                                  |            |          |
  | <code>QUERY  ERROR  COLD</code>                               |            |          |

