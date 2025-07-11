/**
 * @file
 * @brief eForth prototype and interface
 */
#ifndef __EFORTH_CORE_H
#define __EFORTH_CORE_H
#include "eforth_config.h"
///
///@name Compiler options
///@{
#define EXE_TRACE     0  /**< VM execution tracing flag */
#define COMPUTED_GOTO 0  /**< dispatcher (~5% faster, +180 bytes RAM) */
///@}
///@name Arduino Support Macros
///@{
#if ARDUINO
#include <Arduino.h>
#include <avr/pgmspace.h>

#if defined(RAW_UART)
#include "eforth_uart.h"
#define StreamIO            HardwareUART
#else
#define StreamIO            Stream
#endif

#define LOG(s)              io->print(F(s))
#define LOG_C(c)            { io->print(c); if (c=='\n') io->flush(); }
#define LOG_V(s, n)         { io->print(F(s)); io->print((DU)(n)); }
#define LOG_H(s, n)         { io->print(F(s)); io->print((n), HEX); }
#define CLI()               cli()
#define SEI()               sei()

#else  // !ARDUINO

#include <stdlib.h>
typedef const char          *PGM_P;
#define StreamIO            char
#define pgm_read_byte(b)    *((U8*)(b))
#define PROGMEM
#define millis()            ((U32)clock())
#define pinMode(a,b)
#define digitalRead(p)      (0)
#define digitalWrite(p,v)
#define analogRead(p)       (0)
#define analogWrite(p,v)
#define map(a,b,c,d,e)      (0)
#define LOG(s)              printf("%s", (s))
#define LOG_C(c)            printf("%c", (c))
#define LOG_V(s, n)         printf("%s%d", (s), (n))
#define LOG_H(s, n)         printf("%s%x", (s), (U16)((n)&0xffff))
#define LOW                 (0)
#define HIGH                (1)
#define CLI()
#define SEI()

#endif // ARDUINO

///@}
///@name interrupt handle routines
///@{
void intr_reset();          ///< reset interrupts
U16  intr_hits();
IU   intr_service();
void intr_add_tmisr(U8 i, U16 ms, IU xt);
void intr_add_pcisr(U8 pin, IU xt);
void intr_timer_enable(U8 f);
void intr_pci_enable(U8 f);
///@}
///@name eForth Virtual Machine Functions
///@{
void vm_init(
    PGM_P rom,              ///< pointer to Arduino flash memory block (ROM)
    U8    *ram,             ///< pointer to Arduino RAM block (RAM)
    void  *io_stream,       ///< pointer to Stream object of Arduino
	const char *code        ///< embeded Forth code
    );
void vm_outer();            ///< Forth outer interpreter
///@}
///@name eForth Assembler Functions
///@{
int  ef_save(U8 *ram);      ///< save user variables and dictionary to EEPROM
int  ef_load(U8 *ram);      ///< load user variables and dictionary from EEPROM
///@}
#endif // __EFORTH_CORE_H
