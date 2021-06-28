/**
 * @file eforth_core.h
 * @brief eForth prototype and interface
 */
#ifndef __EFORTH_SRC_EFORTH_CORE_H
#define __EFORTH_SRC_EFORTH_CORE_H
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#ifdef ESP8266
#include "user_interface.h"
#endif // ESP8266

#define CASE_SENSITIVE  0             /**< define case sensitivity */
#define ROM_DUMP_ONLY   0             /**< create ROM only (i.e. no debugging) */
///
///@name Debug Tracing Flags
///@{
#define ASM_TRACE       0             /**< assembler tracing flag */
#define EXE_TRACE       0             /**< virtual machine execution tracing flag */
///@}
///
///@name Portable Types
///@{
typedef uint32_t  U32;                ///< 32-bit unsigned integer
typedef uint16_t  U16;                ///< 16-bit unsigned integer
typedef uint8_t   U8;                 ///< 8-bit unsigned integer

typedef int32_t   S32;                ///< 32-bit signed integer
typedef int16_t   S16;                ///< 16-bit signed integer
typedef int8_t    S8;                 ///< 8-bit signed integer

typedef U16       XA;				  ///< address sizing (16-bit)
///@}
///
///@name Capacity and Sizing
///@{
#define CELLSZ		     2            /**< 16-bit cell size */
#define FORTH_PRIMITIVES 64           /**< number of primitive words */
#define FORTH_ROM_SZ     0x1000       /**< size of ROM (for pre-defined words) */
#define FORTH_RAM_SZ     0x500        /**< size of RAM (for user defined words) */
#define FORTH_STACK_SZ   0x60*CELLSZ  /**< size of data/return stack */
#define FORTH_TIB_SZ     0x40         /**< size of terminal input buffer */
#define FORTH_PAD_SZ     0x20         /**< size of output pad */
///@}
///
///> note:
///>    Forth only needs a few bytes for Arduino auto (on heap), but
///>    Serial TX/RX buffers uses 0x40 * 2 = 128 bytes
///>
///> logic and stack op macros (processor dependent)
///>
///@name Memory Map Addressing
///@{
#define FORTH_BOOT_ADDR  0x0000
#define FORTH_RAM_ADDR   0x1000
#define FORTH_STACK_ADDR (FORTH_RAM_ADDR+0x0)
#define FORTH_STACK_TOP  (FORTH_STACK_ADDR+FORTH_STACK_SZ)
#define FORTH_TIB_ADDR   (FORTH_STACK_TOP)
#define FORTH_TVAR_ADDR  (FORTH_TIB_ADDR+FORTH_TIB_SZ)
#define FORTH_UVAR_ADDR  (FORTH_TVAR_ADDR+0x10)
#define FORTH_DIC_ADDR   (FORTH_UVAR_ADDR+0x10)
///@}
///
///@name Logical
/// TRUE cannot use 1 because NOT(ffffffff)==0 while NOT(1)==ffffffff
/// which does not need boolean op (i.e. in C)
/// @{
#define	TRUE	         -1
#define	FALSE	         0
///@}
///
/// Forth VM Opcodes (for Bytecode Assembler)
///
enum {
    opNOP = 0,    // 0
    opBYE,        // 1
    opQRX,        // 2
    opTXSTO,      // 3
    opDOCON,      // 4
    opDOLIT,      // 5
    opENTER,      // 6
    opEXIT,       // 7
    opEXECU,      // 8
    opDONEXT,     // 9
    opQBRAN,      // 10
    opBRAN,       // 11
    opSTORE,      // 12
    opAT,         // 13
    opCSTOR,      // 14
    opCAT,        // 15
    opONEP,       // 16 Dr. Ting's opRPAT
    opONEM,       // 17 Dr. Ting's opRPSTO
    opRFROM,      // 18
    opRAT,        // 19
    opTOR,        // 20
    opDELAY,      // 21 Dr. Ting's opSPAT
    opCLOCK,      // 22 Dr. Ting's opSPSTO
    opDROP,       // 23
    opDUP,        // 24
    opSWAP,       // 25
    opOVER,       // 26
    opZLESS,      // 27
    opAND,        // 28
    opOR,         // 29
    opXOR,        // 30
    opUPLUS,      // 31
    opDEPTH,      // 32 Dr. Ting's opNEXT (not needed)
    opQDUP,       // 33
    opROT,        // 34
    opLSHIFT,     // 35 Dr. Ting's opDDROP
    opRSHIFT,     // 36 Dr. Ting's opDDUP
    opPLUS,       // 37
    opINVERT,     // 38
    opNEGAT,      // 39
    opGREAT,      // 40 Dr. Ting's opDNEGA
    opSUB,        // 41
    opABS,        // 42
    opEQUAL,      // 43
    opULESS,      // 44
    opLESS,       // 45
    opUMMOD,      // 46
    opPIN,        // 47 Dr. Ting's opMSMOD
    opMAP,        // 48 Dr. Ting's opSLMOD
    opMOD,        // 49
    opSLASH,      // 50
    opUMSTAR,     // 51
    opSTAR,       // 52
    opMSTAR,      // 53
    opIN,         // 54 Dr. Ting's opSSMOD
    opOUT,        // 55 Dr. Ting's opSTASL
    opPICK,       // 56
    opPSTOR,      // 57
    opAIN,        // 58 Dr. Ting's opDSTOR
    opPWM,        // 59 Dr. Ting's opDAT
    opDNEGA,      // 60 Dr. Ting's opCOUNT
    opDOVAR,      // 61
    opDPLUS,      // 62 Dr. Ting's opMAX
    opDSUB        // 63 Dr. Ting's opMIN
};
///
/// protothread task declaration
///
typedef struct ef_task {
    char (*task)();            ///< user defined task (in protothreads)
    struct ef_task *next;      ///< linked-list
} *task_ptr;
//
// eForth function prototypes
//
///@name Arduino Support Macros
///@{
#if ARDUINO
#include <Arduino.h>
#include <avr/pgmspace.h>
#include <pt.h>
#include <time.h>
#define LOG(s)              io->print(F(s))
#define LOG_C(c)            ef_putchar(c)
#define LOG_V(s, n)         { io->print(F(s)); io->print((S16)n); }
#define LOG_H(s, n)         { io->print(F(s)); io->print(n, HEX); }

#else

#include <stdlib.h>
typedef const char          *PGM_P;
#define pgm_read_byte(b)    *((U8*)(b))
#define PROGMEM
#define Stream              char
#define pinMode(a,b)
#define digitalRead(p)      (0)
#define digitalWrite(p,v)
#define analogRead(p)		(0)
#define analogWrite(p,v)
#define map(a,b,c,d,e) 		(0)
#define millis()       		(0x12345678)
#define PT_SCHEDULE(t) 		(t)
#define LOG(s)              printf("%s", s)
#define LOG_C(c)            ef_putchar(c)
#define LOG_V(s, n)         printf("%s%d", s, n)
#define LOG_H(s, n)         printf("%s%x", s, n&0xffff)
#endif // ARDUINO
///@}
///
///@name eForth Virtual Machin Functions
///@{
void vm_init(
    PGM_P rom,              ///< pointer to Arduino flash memory block (ROM)
    U8 *cdata,              ///< pointer to Arduino RAM block (RAM)
    void *io_stream         ///< pointer to Stream object of Arduino
    );
int  vm_step();
///@}
///
///@name eForth IO Functions
///@{
void ef_prompt();
U8   ef_getchar();
void ef_putchar(char c);
void ef_yield();
///@}
///
///@name eForth Assembler Functions
///@{
int  ef_assemble(
    U8 *cdata               ///< pointer to Arduino memory block where assembled data will be populated
    );
void ef_dump_rom(
    U8 *cdata,              ///< pointer to assembled memory block (into C syntax)
    int len                 ///< number of byte to be dump
    );
///@}
#endif // __EFORTH_SRC_EFORTH_CORE_H
