/**
 * @file
 * @brief eForth UART class
 * @example
 HardwareUART uart;
 void setup() {
     uart.begin(9600);
 }
 void loop() {
     uart.print('.');
     while (uart.available()) {
         char c = uart.read();
         uart.print(F("rx="));
         uart.print((int)c, 16);
     }
     delay(1000);
 }
 *
 */
#ifndef __EFORTH_UART_H
#define __EFORTH_UART_H

#include <avr/io.h>

typedef unsigned long UL;
#define HEX           16
#define CPU_FREQ      16000000UL
#define UART_BAUD     9600

struct HardwareUART
{
    /// Serial interface
    void begin(UL baud) {
        UL prescale = (CPU_FREQ / (16UL * baud)) - 1;

        DDRD  &= ~_BV(PD0);                    /// * Enable RXD pullup
        PORTD |= _BV(PD0);                     /// * Default UART init

        UBRR0L = prescale;                     /// * Set the baudrate
        UBRR0H = prescale >> 8;
    
        UCSR0B = (1<<TXEN0)  | (1<<RXEN0);     /// * Enable the interface
        UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);    /// * Async 8N1
    }
    void end(void)               { UCSR0B = 0; }
    int  available(void)         { return UCSR0A & (1<<RXC0);  }
    int  availableForWrite(void) { return UCSR0A & (1<<UDRE0); }
    int  read()                  { return available() ? (int)_rx() : -1; }
    void flush()                 { /* do nothing */ }
    operator bool()              { return true; }
    
    /// Print interface
    void print(char c)   { _tx(c); }
    void print(char *s)  { _txn(s, strlen(s)); }
    void print(PGM_P p)  {                     ///< print program memory string
        for (int i=0, n=strlen_P(p); i < n; i++) {
            _tx(pgm_read_byte(p++));
        }
    }
    void print(const __FlashStringHelper *s) { print((PGM_P)s); }
    void print(int v, int base=10) {           ///< print integer
        char buf[32];                          ///< output buffer
        int  i = 32, n = abs(v);
        buf[--i] = '\0';                       /// * pad last byte just in case
        while (i > 0 && n != 0) {
            char c = (char)(n % base);
            buf[--i] = c < 10 ? c + '0' : c + 'A';
            n /= base;
        }
        buf[--i] = v < 0 ? '-' : ' ';          /// * sign
        print(&buf[i]);                        /// * dump as string
    }

    /// private methods
    void _tx(char c) {
        while (!availableForWrite());
        UDR0 = c;                              /// * Send input byte
    }
    void _txn(const char *p, int len) {        ///< send data buffer to UART
        for (int i=0; i < len; i++) _tx(*p++);
    }
    char _rx(void) {                           ///< receive one byte
        while (!available());                  /// * Wait for UART
        return UDR0;                           /// * return received byte
    }
    void _rxn(char *p, int len) {              ///< received bytes into data buffer
        for (int i=0; i < len; i++) *p++ = _rx();
    }
};

extern HardwareUART UART;                      /// * static instance

#endif // __EFORTH_UART_H
