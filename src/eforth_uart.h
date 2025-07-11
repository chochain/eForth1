/**
 * @file
 * @brief eForth UART class
 * @example
 UART uart;
 void setup() {
     uart.init();
 }
 void loop() {
     while (uart.available()) {
         char c = uart.read();
         uart.print(F("rx="));
         uart.print(c);
     }
     delay(1000);
     uart.print('.');
 }
 *
 */
#ifndef __EFORTH_UART_H
#define __EFORTH_UART_H
#include <avr/io.h>

#define CPU_FREQ      16000000UL
#define UART_BAUD     9600
#define BAUD_PRESCALE (((CPU_FREQ / (16UL * UART_BAUD))) - 1)

struct UART {
    void init(void) {
        UCSR0B = 0;                            /// * UART reset
    
        DDRD  &= ~_BV(PD0);                    /// * Enable RXD pullup
        PORTD |= _BV(PD0);                     /// * Default UART init

        UBRR0L = BAUD_PRESCALE;                /// * Set the baudrate
        UBRR0H = BAUD_PRESCALE >> 8;
    
        UCSR0B = (1<<TXEN0) | (1<<RXEN0);      /// * Enable the interface
        UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);    /// * Async 8N1
    }
    int  available(void) { return UCSR0A & (1<<RXC0);  }
    int  ready(void)     { return UCSR0A & (1<<UDRE0); }
    char read()          { return _rx(); }
    void print(char c)   { _tx(c); }
    void print(char *s)  { _txn(s, strlen(s)+1); }
    void print(PGM_P p)  {
        for (int i=0, n=strlen_P(p); i < n; i++) {
            _tx(pgm_read_byte(p++));
        }
    }
    void print(const __FlashStringHelper *s) { print((PGM_P)s); }
    void print(int v, int base=10) {
        char buf[32];
        int  i = 32, n = abs(v);
        buf[--i] = '\0';
        while (i > 0 && n != 0) {
            char c = (char)(n % base);
            buf[--i] = c < 10 ? c + '0' : c + 'A';
            n /= base;
        }
        buf[--i] = v < 0 ? '-' : ' ';
        print(&buf[i]);
    }
    void _tx(char c) {
        while (!ready());
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
#endif // __EFORTH_UART_H
