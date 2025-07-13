/**
 * @file
 * @brief eForth configurations and opcode list
 */
#ifndef __EFORTH_CONFIG_H
#define __EFORTH_CONFIG_H
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <time.h>
#ifdef ESP8266
#include "user_interface.h"
#endif // ESP8266

#define APP_NAME      "eForth1 v2.6"
#define USE_RAW_UART  1 
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
typedef S16       DU;                 ///< data/cell unit
typedef void (*CFP)();                ///< function pointer
///@}
///
///@name Capacity and Sizing
///@attention reassemble ROM needed if FORTH_TIB_SZ or FORTH_PAD_SZ changed
///@{
#define CELLSZ           2            /**< 16-bit cell size                    */
#define CFUNC_MAX        8            /**< size C function pointer slots (8)   */
#define FORTH_ROM_SZ     0x2000       /**< size of ROM (for pre-defined words) */
#define FORTH_UVAR_SZ    0x20         /**< size of Forth user variables        */
#define FORTH_DIC_SZ     (0x400-FORTH_UVAR_SZ) /**< size of dictionary space   */
#define FORTH_STACK_SZ   0x80         /**< size of data/return stack           */
#define FORTH_TIB_SZ     0x80         /**< size of terminal input buffer       */
#define FORTH_RAM_SZ     ( \
     FORTH_UVAR_SZ + FORTH_DIC_SZ + \
     FORTH_STACK_SZ + FORTH_TIB_SZ)   /**< total RAM allocated                 */
///@}
///
///> note:
///>    Forth only needs a few bytes for Arduino auto (on heap), but
///>    Serial default TX/RX buffers uses 0x40 * 2 = 128 bytes
///>
///> logic and stack op macros (processor dependent)
///>
///@name Memory Map Addressing
///@{
#define FORTH_BOOT_ADDR  0x0000
#define FORTH_RAM_ADDR   FORTH_ROM_SZ
#define FORTH_UVAR_ADDR  FORTH_RAM_ADDR
#define FORTH_DIC_ADDR   (FORTH_UVAR_ADDR + FORTH_UVAR_SZ)
#define FORTH_STACK_ADDR (FORTH_DIC_ADDR  + FORTH_DIC_SZ)
#define FORTH_STACK_TOP  (FORTH_STACK_ADDR + FORTH_STACK_SZ)
#define FORTH_TIB_ADDR   (FORTH_STACK_TOP)
#define FORTH_MAX_ADDR   (FORTH_TIB_ADDR + FORTH_TIB_SZ)
///@}
///
///@name Logical
/// TRUE cannot use 1 because NOT(ffff)==0 while NOT(1)==ffff
/// which does not need boolean op (i.e. in C)
/// @{
#define TRUE             -1
#define FALSE            0
///@}
///@name VM operational flags
///@{
#define fCOLON16        0x8000       /**< 16-bit flag, colon word */
#define fCOLON8         0x80         /**< 8-bit flag (high-byte)  */
///@}
#endif // __EFORTH_CONFIG_H
