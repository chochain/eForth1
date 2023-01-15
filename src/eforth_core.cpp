/**
 * @file
 * @brief eForth core controller module
 */
#include "eforth_core.h"
///
/// interrupt handlers
///
U8  t_idx  { 0 };                 ///< timer ISR index
U16 p_xt[] { 0, 0, 0 };           ///< pin change interrupt vectors
U16 t_xt[8]; 		              ///< timer interrupt vectors
U16 t_max[8]; 		              ///< timer CTC top value

volatile U8  t_hit { 0 };         ///< 8-bit for 8 timer ISR
volatile U8  p_hit { 0 };         ///< pin change interrupt (PORT-B,C,D)
volatile U16 t_cnt[8];            ///< timer CTC counters

void intr_reset() {
    CLI();
    t_idx = t_hit = p_hit = 0;
    SEI();
}
///
///> fetch interrupt hit flags
///
U16 intr_hits() {
    CLI();
    U16 hx = (p_hit << 8) | t_hit;  // capture interrupt flags
    p_hit = t_hit = 0;
    SEI();
    return hx;
}
///
///> service interrupt routines
///
void intr_service(void (*cb)(U16)) {
	for (int i=0; t_hit && i<t_idx; i++, t_hit>>=1) {
		if (t_hit & 1) cb(t_xt[i]);
	}
	for (int i=0; p_hit && i<3; i++, p_hit>>=1) {
		if (p_hit & 1) cb(p_xt[i]);
	}
}
///
///> add timer interrupt service routine
///
void intr_add_timer(U16 n, U16 xt) {
    if (xt==0 || t_idx > 7) return; // range check

    CLI();
    t_xt[t_idx]  = xt;              // ISR xt
    t_cnt[t_idx] = 0;               // init counter
    t_max[t_idx] = n;               // period (in 0.1s)
    t_idx++;
    SEI();
}
#if ARDUINO
///
///> add pin change interrupt service routine
///
void intr_add_pci(U16 p, U16 xt) {
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
void intr_enable_pci(U16 f) {
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
void enable_timer(U16 f) {
    CLI();
    pinMode(7, OUTPUT);                    // DEBUG: timer2 interrupt enable/disable

    TCCR2A = TCCR2B = TCNT2 = 0;           // reset counter
    if (f) {
        TCCR2A = _BV(WGM21);               // Set CTC mode
        TCCR2B = _BV(CS22)|_BV(CS21);      // prescaler 256 (16000000 / 256) = 62500Hz = 16us
        OCR2A  = 249;                      // 250Hz = 4ms, (250 - 1, must < 256)
        TIMSK2 |= _BV(OCIE2A);             // enable timer2 compare interrupt
        digitalWrite(7, HIGH);             // DEBUG: timer2 enabled
    }
    else {
        TIMSK2 &= _BV(OCIE2A);             // disable timer2 compare interrupt
        digitalWrite(7, LOW);              // DEBUG: timer disabled
    }
    SEI();
}
///
///> Arduino interrupt service routines
///
ISR(TIMER2_COMPA_vect) {
    volatile static int cnt = 0;
    if (++cnt < 25) return;                // 25 * 4ms = 100ms
    cnt = 0;
    for (U8 i=0, b=1; i < EfIntr::t_idx; i++, b<<=1) {
        digitalWrite(7, digitalRead(7) ? LOW : HIGH);  // DEBUG: ISR called
        if (++EfIntr::t_cnt[i] < EfIntr::t_max[i]) continue;
        EfIntr::t_hit    |= b;
        EfIntr::t_cnt[i]  = 0;
    }
}
ISR(PCINT0_vect) { EfIntr::p_hit |= 1; }
ISR(PCINT1_vect) { EfIntr::p_hit |= 2; }
ISR(PCINT2_vect) { EfIntr::p_hit |= 4; }

#else // !ARDUINO

void intr_add_pci(U16 p, U16 xt)   {}           // mocked functions for x86
void intr_enable_pci(U16 f)        {}
void intr_enable_timer(U16 f)      {}

#endif // ARDUINO
///
/// eForth core module
///
static Stream *io;                ///< IO interface
static U8     *_ram;              ///< forth memory block dynamic allocated
///
/// display eForth system information
///
void sys_info(U8 *cdata, int sz) {
	LOG_H("\nROM_SZ=x",   sz);
    LOG_H("\, RAM_SZ=x",  FORTH_RAM_SZ);
    LOG_V(", Addr=",      (U16)sizeof(IU)*8);
    LOG_V("-bit, CELL=",  CELLSZ);
    LOG("-byte\nMemory MAP:");
#if ARDUINO
    U16 h = (U16)&cdata[FORTH_RAM_SZ];
    U16 s = (U16)&s;
    LOG_H(" heap=x", h);
    LOG_V("--> ", s - h);
    LOG_H(" <--auto=x", s);
#endif // ARDUINO
    LOG_H("\n  ROM  :x0000+", FORTH_ROM_SZ);
    LOG_H("\n  DIC  :x", FORTH_DIC_ADDR);   LOG_H("+", FORTH_DIC_SZ);
    LOG_H("\n  UVAR :x", FORTH_UVAR_ADDR);  LOG_H("+", FORTH_UVAR_SZ);
    LOG_H("\n  STACK:x", FORTH_STACK_ADDR); LOG_H("+", FORTH_STACK_SZ);
    LOG_H("\n  TIB  :x", FORTH_TIB_ADDR);   LOG_H("+", FORTH_TIB_SZ);
}
#if ARDUINO
extern U32 forth_rom[];                    // from eforth_rom.c
extern U32 forth_rom_sz;
///
/// setup (called by Arduino setup)
///
void ef_setup(Stream &io_stream=Serial)
{
    io   = &io_stream;
    _ram = (U8*)malloc(FORTH_RAM_SZ);     // dynamically allocated

    sys_info(_ram, forth_rom_sz);
    vm_init((PGM_P)forth_rom, _ram, (void*)&io_stream);
}
///
/// console input with yield
///
U8 ef_getchar()
{
    while (!io->available()) {
        intr_service();
    }
    return (U8)io->read();
}
///
/// output one char to console
///
void ef_putchar(char c)
{
    io->print(c);
    if (c=='\n') io->flush();
}

#else  // !ARDUINO

U8   ef_getchar()                 { return getchar(); }
void ef_putchar(char c)           { printf("%c", c);  }

static U8 _rom[FORTH_ROM_SZ] = {};         // fake rom to simulate run time
///
/// main to support C development debugging
///
int main(int ac, char* av[])
{
    setvbuf(stdout, NULL, _IONBF, 0);      // autoflush (turn STDOUT buffering off)

    int sz = ef_assemble(_rom);

#if !ASM_ONLY
    sys_info(_rom, sz);
    _ram = (U8*)malloc(FORTH_RAM_SZ);      ///< forth memory block dynamic allocated

    vm_init((char*)_rom, _ram, NULL);
    vm_outer();
#endif // !ASM_ONLY

    return 0;
}
#endif // ARDUINO
