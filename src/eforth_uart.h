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
        UCSR0C = (0<<UMSEL00) | (3<<UCSZ00) |  /// * Aync 8-bit
                 (0<<UPM00) | (0<<USBS0);      /// * N 1
    }
    int  available(void) { return UCSR0A & (1<<RXC0);  }
    int  ready(void)     { return UCSR0A & (1<<UDRE0); }
    char read()          { return _rx_byte(); }
    void print(char c)   { _tx_byte(c); }
    void print(char *s)  { _tx_data(s, strlen(s)); }
    void print(PGM_P p)  {
        for (int i=0, n=strlen_P(p); i < n; i++) {
            _tx_byte(pgm_read_byte(p++));
        }
    }
    void print(const __FlashStringHelper *s) { print((PGM_P)s); }
    
    void _tx_byte(char c) {
        while (!ready());
        UDR0 = c;                              /// * Send input byte
    }
    void _tx_data(const char *data, int len) { ///< send data buffer to UART
        const char *tail = data + len;
        while (data != tail) _tx_byte(*data++);
    }
    char _rx_byte(void) {                      ///< receive one byte
        while (!available());                  /// * Wait for UART
        return UDR0;                           /// * return received byte
    }
    void _rx_data(char *data, int len) {       ///< received bytes into data buffer
        const char *tail = data + len;
        while (tail != data) *data++ = _rx_byte();
    }
};
#endif // __EFORTH_UART_H
