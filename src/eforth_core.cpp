/**
 * @file
 * @brief eForth core controller module
 */
#include "eforth_core.h"
//
/// interrupt handlers
///
typedef struct {
    U16 xt[11];                      ///< interrupt vectors 0-7: timer, 8-10: pin change
    U16 t_max[8];                    ///< timer CTC top value
    U8  t_idx { 0 };                 ///< timer ISR index
    volatile U16 t_cnt[8];           ///< timer CTC counters
    volatile U8  t_hit { 0 };        ///< 8-bit for 8 timer ISR
    volatile U8  p_hit { 0 };        ///< pin change interrupt (PORT-B,C,D)
} IsrRec;

IsrRec ir;                           ///< interrupt state management
///
///> reset interrupt states
///
void intr_reset() {
    intr_timer_enable(false);
    intr_pci_enable(false);
    CLI();
    ir.t_idx = ir.t_hit = ir.p_hit = 0;
    for (U8 i=0; i < 11; i++) ir.xt[i] = 0;
    SEI();
}

#if ARDUINO
#define _fake_intr(hits)
#else // !ARDUINO
U8  tmr_on = 0;                   ///< fake timer enabler
void _fake_intr(U8 hits)
{
    static int n = 0;                              // fake interrupt
    if (tmr_on && !hits && ++n >= 100) {
        n=0; ir.t_hit = 3;
    }
}
#endif // ARDUINO
///
///> service interrupt routines
///
#define YIELD_PERIOD 50      /** 256 max (1ms ~ 50*20us/op) */
IU intr_service() {
    volatile static U16 hits = 0;                  ///> cached interrupt flags
	static U8 cnt = 0;                             ///> throttle counter

    _fake_intr(hits);                              /// * on x86 platform

    if (!hits && ++cnt < YIELD_PERIOD) return 0;   /// * throttle down (boot performance a bit)
    cnt = 0;                                       /// * reset throttle counter

    CLI();
    if (!hits) {
        hits = ((U16)ir.p_hit << 8) | ir.t_hit;    // cache interrupt flags
        ir.p_hit = ir.t_hit = 0;                   // clear captured interrupts
    }
    SEI();
    for (U8 i=0; hits; i++, hits>>=1) {            // serve interrupts (hopefully fairly)
		if (hits & 1) {                            // check interrupt flag
			hits >>= 1;                            // clear flag
			return ir.xt[i];                       // return ISR to Forth VM
		}
    }
    return 0;
}
///
///> add timer interrupt service routine
///
void intr_add_tmisr(U8 i, U16 ms, U16 xt) {
    if (xt==0 || i > 7) return; // range check

    CLI();
    ir.xt[i]    = xt;                   // ISR xt
    ir.t_cnt[i] = (U16)millis() % ms;   // init counter (randomize, to even time slice)
    ir.t_max[i] = ms;                   // period (in ms)
    if (i >= ir.t_idx) ir.t_idx = i + 1;
    SEI();
}
#if ARDUINO
///
///> add pin change interrupt service routine
///
void intr_add_pcisr(U8 p, U16 xt) {
    if (xt==0) return;              // range check
    CLI();
    if (p < 8)       {
        ir.xt[10] = xt;
        PCMSK2 |= 1 << p;
    }
    else if (p < 13) {
        ir.xt[8] = xt;
        PCMSK0 |= 1 << (p - 8);
    }
    else {
        ir.xt[9] = xt;
        PCMSK1 |= 1 << (p - 14);
    }
    SEI();
}
///
///> enable/disable pin change interrupt
///
void intr_pci_enable(U8 f) {
    CLI();
    if (f) {
        if (ir.xt[8])  PCICR |= _BV(PCIE0);  // enable PORTB
        if (ir.xt[9])  PCICR |= _BV(PCIE1);  // enable PORTC
        if (ir.xt[10]) PCICR |= _BV(PCIE2);  // enable PORTD
    }
    else PCICR = 0;
    SEI();
}
///
///> enable/disable timer2 interrupt
///
void intr_timer_enable(U8 f) {
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
    for (U8 i=0, b=1; i < ir.t_idx; i++, b<<=1) {
        if (!ir.xt[i] || (++ir.t_cnt[i] < ir.t_max[i])) continue;
        ir.t_hit    |= b;
        ir.t_cnt[i]  = 0;
    }
}
ISR(PCINT0_vect) { ir.p_hit |= 1; }
ISR(PCINT1_vect) { ir.p_hit |= 2; }
ISR(PCINT2_vect) { ir.p_hit |= 4; }

#else // !ARDUINO

void intr_add_pcisr(U8 p, U16 xt) {}        // mocked functions for x86
void intr_pci_enable(U8 f)        {}
void intr_timer_enable(U8 f) {
    tmr_on = f;
}

#endif // ARDUINO
