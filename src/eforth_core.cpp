/**
 * @file eforth_core.cpp
 * @brief eForth core controller module
 */
#include "eforth_core.h"

static U8     *_ram;                     ///< forth memory block dynamic allocated
static Stream *io;                       ///< console interface

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
///
/// eForth yield to user tasks
///
#define YIELD_PERIOD    10
void ef_yield()
{
    static U8 n = 0;
    if (++n < 10) return;                 // give more cycles to VM
    n = 0;
}
/// 
/// console input with yield
///
U8 ef_getchar()
{
    while (!io->available()) {
        ef_yield();
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
/// single step eForth virtual machine
///
void ef_run()
{
    vm_outer();
    ef_yield();
}

#else // ARDUINO

void ef_yield()         {}
void ef_wait(U32 ms)    {}
U8   ef_getchar()       { return getchar(); }
void ef_putchar(char c) { printf("%c", c);  }

static U8 _rom[FORTH_ROM_SZ] = {};          // fake rom to simulate run time
///
/// main to support C development debugging
///
int main(int ac, char* av[])
{
    setvbuf(stdout, NULL, _IONBF, 0);       // autoflush (turn STDOUT buffering off)

    int sz = ef_assemble(_rom);

#if !ASM_ONLY
    sys_info(_rom, sz);
    _ram = (U8*)malloc(FORTH_RAM_SZ);       ///< forth memory block dynamic allocated

    vm_init((char*)_rom, _ram, NULL);
    vm_outer();
#endif // !ASM_ONLY

    return 0;
}
#endif // ARDUINO
