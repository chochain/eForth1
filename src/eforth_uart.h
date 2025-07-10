#ifndef __EFORTH_UART_H
#define __EFORTH_UART_H
#include <avr/io.h>

#define UART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (UART_BAUDRATE * 16UL))) - 1)

struct UART {
    void init(void) {
        UCSRB = 0;                           /// * UART reset
    
        DDRD  &= ~_BV(PD0);                  /// * Enable RXD pullup
        PORTD |= _BV(PD0);                   /// * Default UART init

        UBRRL = BAUD_PRESCALE;               /// * Set the baudrate
        UBRRH = BAUD_PRESCALE >> 8;
    
        UCSRB = _BV(TXEN) | _BV(RXEN);       /// * Enable the interface

        UCSRC = _BV(URSEL) | _BV(USBS) |     /// * Use 8N1
                _BV (UCSZ0) | _BV (UCSZ1);   /// * with two stop bits
    }
    U8   available(void) { return UCSRA & _BV(RXC); }
    U8   read()          { return _rx_byte(); }
    void print(char c)   { _tx_byte(c); }
    void print(PGM_P s)  { _tx_data(s, STRLEN(s)); }
    void print(DU n)     { }
    void flush()         {}
    
    void _tx_byte(U8 byte) {
        while (!(UCSRA & _BV(UDRE)));        /// * Wait until UART not busy
        UDR = byte;                          /// * Send input byte
        while (!(UCSRA & _BV(UDRE)));        /// * Wait until not busy (sent)
    }
    void _tx_data(const U8 *data, int len) { ///< send data buffer to UART
        const U8 *tail = data + len;
        while (data != tail) uart_tx_byte(*data++);
    }
    U8   _rx_byte(void) {                    ///< receive one byte
        while (!uart_data_available());      /// * Wait for UART
        return UDR;                          /// * return received byte
    }
    void _rx_data(U8 *data, int len) {       ///< received bytes into data buffer
        const U8 *tail = data + len;
        while (tail != data) *data++ = uart_rx_byte();
    }
};
#endif // __EFORTH_UART_H
