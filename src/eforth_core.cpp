/**
 * @file
 * @brief eForth core controller module
 */
#include "eforth_core.h"
//
/// interrupt handlers
///
U8  t_idx  { 0 };                 ///< timer ISR index
U16 p_xt[] { 0, 0, 0 };           ///< pin change interrupt vectors
U16 t_xt[8];                      ///< timer interrupt vectors
U16 t_max[8];                     ///< timer CTC top value

volatile U8  t_hit { 0 };         ///< 8-bit for 8 timer ISR
volatile U8  p_hit { 0 };         ///< pin change interrupt (PORT-B,C,D)
volatile U16 t_cnt[8];            ///< timer CTC counters

void intr_reset() {
    intr_timer_enable(false);
    intr_pci_enable(false);
    CLI();
    t_idx = t_hit = p_hit = 0;
    for (int i=0; i < 8; i++) t_xt[i] = 0;
    for (int i=0; i < 3; i++) p_xt[i] = 0;
    SEI();
}
///
///> service interrupt routines
///
#if ARDUINO
#define _fake_intr(hits)
#else // !ARDUINO
U8  tmr_on = 0;                   ///< fake timer enabler
void _fake_intr(U16 hits)
{
    static int n = 0;                              // fake interrupt
    if (tmr_on && !hits && ++n >= 1000) {
        n=0; t_hit = 1;
    }
}
#endif // ARDUINO

#define ISR_PERIOD 50                              // skip 50ms before check ISR flag

IU intr_service() {
    volatile static U16 hits = 0, n = 0;

    _fake_intr(hits);                              // on x86 platform

    if (!hits && ++n < ISR_PERIOD) return 0;       // skip for performance
    n = 0;
    CLI();
    if (!hits) {
        hits = (p_hit << 8) | t_hit;               // capture interrupt flags
        p_hit = t_hit = 0;
    }
    SEI();
    if (hits) {                                    // serve fairly
        U8 hx = hits & 0xff;
        for (int i=0, t=1; hx && i<t_idx; i++, t<<=1, hx>>=1) {
            if (hits & t) { hits &= ~t; return t_xt[i]; }
        }
        hx = hits >> 8;
        for (int i=0, t=0x100; hx && i<3; i++, t<<=1, hx>>=1) {
            if (hits & t) { hits &= ~t; return p_xt[i]; }
        }
    }
    return 0;
}
///
///> add timer interrupt service routine
///
void intr_add_tmisr(U16 i, U16 ms, U16 xt) {
    if (xt==0 || i > 7) return; // range check

    CLI();
    t_xt[i]  = xt;              // ISR xt
    t_cnt[i] = 0;               // init counter
    t_max[i] = ms;              // period (in ms)
    if (i >= t_idx) t_idx = i + 1;
    SEI();
}
#if ARDUINO
///
///> add pin change interrupt service routine
///
void intr_add_pcisr(U16 p, U16 xt) {
    if (xt==0) return;              // range check
    CLI();
    if (p < 8)       {
        p_xt[2] = xt;
        PCMSK2 |= 1 << p;
    }
    else if (p < 13) {
        p_xt[0] = xt;
        PCMSK0 |= 1 << (p - 8);
    }
    else {
        p_xt[1] = xt;
        PCMSK1 |= 1 << (p - 14);
    }
    SEI();
}
///
///> enable/disable pin change interrupt
///
void intr_pci_enable(U16 f) {
    CLI();
    if (f) {
        if (p_xt[0]) PCICR |= _BV(PCIE0);  // enable PORTB
        if (p_xt[1]) PCICR |= _BV(PCIE1);  // enable PORTC
        if (p_xt[2]) PCICR |= _BV(PCIE2);  // enable PORTD
    }
    else PCICR = 0;
    SEI();
}
///
///> enable/disable timer2 interrupt
///
void intr_timer_enable(U16 f) {
    CLI();
    TCCR2A = TCCR2B = TCNT2 = 0;           // reset counter
    if (f) {
        TCCR2A = _BV(WGM21);               // Set CTC mode
        TCCR2B = _BV(CS22);                // prescaler 64 (16MHz / 64) = 250KHz => 4us period
        OCR2A  = 249;                      // 250x4us = 1ms, (250 - 1, must < 256)
        TIMSK2 |= _BV(OCIE2A);             // enable timer2 compare interrupt
    }
    else {
        TIMSK2 &= _BV(OCIE2A);             // disable timer2 compare interrupt
    }
    SEI();
}
///
///> Arduino interrupt service routines
///
ISR(TIMER2_COMPA_vect) {
    for (U8 i=0, b=1; i < t_idx; i++, b<<=1) {
        if (!t_xt[i] || (++t_cnt[i] < t_max[i])) continue;
        t_hit    |= b;
        t_cnt[i]  = 0;
    }
}
ISR(PCINT0_vect) { p_hit |= 1; }
ISR(PCINT1_vect) { p_hit |= 2; }
ISR(PCINT2_vect) { p_hit |= 4; }

#else // !ARDUINO

void intr_add_pcisr(U16 p, U16 xt) {}        // mocked functions for x86
void intr_pci_enable(U16 f)        {}
void intr_timer_enable(U16 f) {
    tmr_on = f;
}

#endif // ARDUINO
