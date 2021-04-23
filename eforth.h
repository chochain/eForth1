#ifndef __EFORTH_SRC_EFORTH_H
#define __EFORTH_SRC_EFORTH_H
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <Arduino.h>

#define EFORTH_16BIT    1
//
// debugging flags
//
#define PRINTF(s, ...)  Serial.printf(s, ##__VA_ARGS__)
#define GETCHAR()       Serial.read()
#define ASM_TRACE       1
#define EXE_TRACE       0
//
// portable types
//
typedef uint64_t  U64;
typedef uint32_t  U32;
typedef uint16_t  U16;
typedef uint8_t   U8;
typedef int64_t   S64;

typedef int32_t   S32;
typedef int16_t   S16;
typedef int8_t    S8;

typedef U16       XA;				// Address size
//
// capacity and sizing
//
#define CELLSZ		     2
#define FORTH_PRIMITIVES 64
#define FORTH_STACK_SZ   0x40
#define FORTH_TIB_SZ     0x40
#define FORTH_MEM_SZ     0x2000
//
// logic and stack op macros (processor dependent)
//
#define FORTH_BOOT_ADDR  0x0
#define FORTH_TVAR_ADDR  0x4
#define FORTH_UVAR_ADDR  0x10
#define FORTH_TIB_ADDR   0x20
#define FORTH_STACK_ADDR (FORTH_TIB_SZ+FORTH_TIB_SZ)
#define FORTH_DIC_ADDR   (FORTH_STACK_ADDR+FORTH_STACK_SZ*CELLSZ)
//
// TRUE cannot use 1 because NOT(ffffffff)==0 while NOT(1)==ffffffff
// which does not need boolean op (i.e. in C)
//
#define	TRUE	         -1
#define	FALSE	         0
//
// Forth VM Opcodes (for Bytecode Assembler)
//
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
    opRPAT,       // 16   borrowed for trc_on
    opRPSTO,      // 17   borrowed for trc_off
    opRFROM,      // 18
    opRAT,        // 19
    opTOR,        // 20
    opSPAT,       // 21
    opSPSTO,      // 22   
    opDROP,       // 23
    opDUP,        // 24
    opSWAP,       // 25
    opOVER,       // 26
    opZLESS,      // 27
    opAND,        // 28
    opOR,         // 29
    opXOR,        // 30
    opUPLUS,      // 31
    opNEXT,       // 32
    opQDUP,       // 33
    opROT,        // 34
    opDDROP,      // 35
    opDDUP,       // 36
    opPLUS,       // 37
    opINVER,      // 38
    opNEGAT,      // 39
    opDNEGA,      // 40
    opSUB,        // 41
    opABS,        // 42
    opEQUAL,      // 43
    opULESS,      // 44
    opLESS,       // 45
    opUMMOD,      // 46
    opMSMOD,      // 47
    opSLMOD,      // 48
    opMOD,        // 49
    opSLASH,      // 50
    opUMSTA,      // 51
    opSTAR,       // 52
    opMSTAR,      // 53
    opSSMOD,      // 54
    opSTASL,      // 55
    opPICK,       // 56
    opPSTOR,      // 57
    opDSTOR,      // 58
    opDAT,        // 59
    opCOUNT,      // 60
    opDOVAR,      // 61
    opMAX,        // 62
    opMIN         // 63
};
#endif // __EFORTH_SRC_EFORTH_H
