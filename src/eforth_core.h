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

#define APP_NAME        "eForth1"
#define MAJOR_VERSION   "v1"
#define CASE_SENSITIVE  0             /**< define case sensitivity */
#define ASM_ONLY        0             /**< create ROM only (i.e. no debugging) */
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

typedef U16       IU;                 ///< instruction/address unit (16-bit)
///@}
///
///@name Capacity and Sizing
///@attention reassemble ROM needed if FORTH_TIB_SZ or FORTH_PAD_SZ changed
///@{
#define CELLSZ           2                     /**< 16-bit cell size                    */
#define FORTH_PRIMITIVES 64                    /**< number of primitive words           */
#define FORTH_ROM_SZ     0x2000                /**< size of ROM (for pre-defined words) */
#define FORTH_DIC_SZ     0x400                 /**< size of dictionary space            */
#define FORTH_UVAR_SZ    0x20                  /**< size of Forth user variables        */
#define FORTH_STACK_SZ   0x60*CELLSZ           /**< size of data/return stack           */
#define FORTH_PAD_SZ     0x20                  /**< size of output pad (in DIC space )  */
#define FORTH_TIB_SZ     0x80                  /**< size of terminal input buffer       */
#define FORTH_RAM_SZ     (FORTH_DIC_SZ + FORTH_STACK_SZ)  /**< size of total RAM needed */
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
#define FORTH_RAM_ADDR   FORTH_ROM_SZ
#define FORTH_TVAR_ADDR  (FORTH_RAM_ADDR   + 0x0)
#define FORTH_UVAR_ADDR  (FORTH_TVAR_ADDR  + 0x10)
#define FORTH_DIC_ADDR   (FORTH_UVAR_ADDR  + 0x10)
#define FORTH_STACK_ADDR (FORTH_RAM_ADDR   + FORTH_DIC_SZ)
#define FORTH_STACK_TOP  (FORTH_STACK_ADDR + FORTH_STACK_SZ)
#define FORTH_TIB_ADDR   (FORTH_STACK_TOP)
///@}
///
///@name Logical
/// TRUE cannot use 1 because NOT(ffffffff)==0 while NOT(1)==ffffffff
/// which does not need boolean op (i.e. in C)
/// @{
#define TRUE             -1
#define FALSE            0
///@}
///
/// Forth VM Opcodes (for Bytecode Assembler)
///
enum {
    opNOP = 0,    // 0
    opBYE,
    opQRX,
    opTXSTO,
    opDOCON,
    opDOLIT,
    opDOVAR,
    opENTER,
    opEXIT,
    opEXECU,
    opDONEXT,     // 10
    opQBRAN,
    opBRAN,
    opSTORE,
    opPSTOR,
    opAT,
    opCSTOR,
    opCAT,
    opRFROM,
    opRAT,
    opTOR,
    opDROP,       // 20
    opDUP,
    opSWAP,
    opOVER,
    opROT,
    opPICK,
    opAND,
    opOR,
    opXOR,
    opINV,        // 30
    opLSH,
    opRSH,
    opADD,
    opSUB,
    opMUL,
    opDIV,
    opMOD,
    opNEG,
    opGT,
    opEQ,         // 40
    opLT,
    opZGT,
    opZEQ,
    opZLT,
    opONEP,
    opONEM,
    opQDUP,
    opDEPTH,
    opULESS,
    opUMMOD,      // 50
    opUMSTAR,
    opMSTAR,
    opDNEG,
    opDADD,
    opDSUB,
    opDELAY,
    opCLK,
    opPIN,
    opMAP,
    opIN,         // 60
    opOUT,
    opAIN,
    opPWM
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
#define analogRead(p)       (0)
#define analogWrite(p,v)
#define map(a,b,c,d,e)      (0)
#define millis()            (0x12345678)
#define PT_SCHEDULE(t)      (t)
#define LOG(s)              printf("%s", s)
#define LOG_C(c)            ef_putchar(c)
#define LOG_V(s, n)         printf("%s%d", s, n)
#define LOG_H(s, n)         printf("%s%x", s, (n)&0xffff)
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
U8   ef_getchar();
void ef_putchar(char c);
void ef_yield();
///@}
///
///@name eForth Assembler Functions
///@{
void ef_assemble(
    U8 *cdata               ///< pointer to Arduino memory block where assembled data will be populated
    );
///@}
#endif // __EFORTH_SRC_EFORTH_CORE_H
