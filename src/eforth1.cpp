/*
 * @file
 * @brief - eForth1 main program
 */
///
///> display eForth system information
///
#include "eforth_core.h"
#include "eForth1.h"

void _stat(U8 *ram, int sz, Stream *io) {
	LOG_H("\nROM_SZ=x",  sz);
    LOG_H(", RAM_SZ=x",  FORTH_RAM_SZ);
    LOG_V(", Addr=",     (U16)sizeof(IU)*8);
    LOG_V("-bit, CELL=", CELLSZ);
    LOG("-byte\nMemory MAP:");
    LOG_H("\n  ROM  :x0000+", FORTH_ROM_SZ);
    LOG_H("\n  VAR  :x", FORTH_UVAR_ADDR);  LOG_H("+", FORTH_UVAR_SZ);  LOG("  <=> EEPROM");
    LOG_H("\n  DIC  :x", FORTH_DIC_ADDR);   LOG_H("+", FORTH_DIC_SZ);   LOG(" <=> EEPROM");
    LOG_H("\n  STACK:x", FORTH_STACK_ADDR); LOG_H("+", FORTH_STACK_SZ);
    LOG_H("\n  TIB  :x", FORTH_TIB_ADDR);   LOG_H("+", FORTH_TIB_SZ);
    LOG_H("\n  ROOF :x", FORTH_MAX_ADDR);
#if ARDUINO
    U16 h = (U16)&ram[FORTH_RAM_SZ];
    U16 s = (U16)&h;
    LOG_H(" [heap=x", h);
    LOG_V("--> ", s - h);
    LOG_H(" bytes <--auto=x", s); LOG("]");
#endif // ARDUINO
}
///
///> EEPROM interface
///
#if ARDUINO
#include <EEPROM.h>
#else  // !ARDUINO
class MockPROM                                ///< mock EEPROM access class
{
    U8 _prom[FORTH_UVAR_SZ + FORTH_DIC_SZ];   ///< mock EEPROM storage
public:
    U8   read(U16 idx)         { return _prom[idx]; }
    void update(U16 idx, U8 v) { _prom[idx] = v; }
};
MockPROM EEPROM;                              ///> fake Arduino EEPROM unit
#endif // ARDUINO
///
///> EEPROM Save/Load
///
#define GET(d)    (*(DU*)&ram[d])
#define SET(d, v) (*(DU*)&ram[d] = (v))
int ef_save(U8 *ram)
{
    U16 here = GET(sizeof(DU) * 2);
    int sz   = here - FORTH_RAM_ADDR;
    for (int i=0; i < sz; i++) {
        EEPROM.update(i, ram[i]);     /// * store dictionary byte-by-byte
    }
    return sz;
}
int ef_load(U8 *ram)
{
    U16 pidx = sizeof(DU) * 2;         ///< CP addr in EEPROM (aka HERE)
    U16 vCP  = ((U16)EEPROM.read(pidx+1)<<8) + EEPROM.read(pidx);
    int  sz  = vCP - FORTH_RAM_ADDR;
    if (!vCP || sz > FORTH_DIC_SZ) return 0;

    IU  hidx = sizeof(DU) * 9;         /// * >IN (buffer pointer)
    DU  vIN  = GET(hidx);              /// * keep vIN, vNTIB
    DU  vNTIB= GET(hidx + sizeof(DU));

    for (int i=0; i < sz; i++) {
        ram[i] = EEPROM.read(i);       /// * retrieve dictionary byte-by-byte
    }
    SET(hidx, vIN);                    /// * restore vIN, vNTIB
    SET(hidx + sizeof(DU), vNTIB);

    return sz;
}
#undef GET
#undef SET

#if ARDUINO
extern U32    forth_rom[];                 ///< from eforth_rom.c
extern U32    forth_rom_sz;                ///< actual size of ROM
static U8     *ram;                        ///< forth memory block dynamic allocated
static Stream *io;                         ///< IO stream (Serial Monitor)
///
///> setup (called by Arduino setup)
///
void ef_setup(const char *code, Stream &io_stream)
{
	io  = &io_stream;
    ram = (U8*)malloc(FORTH_RAM_SZ);       ///< dynamically allocated (to prevent IDE complaint)

    vm_init((PGM_P)forth_rom, ram, io, code);
    _stat(ram, forth_rom_sz, io);
}
///
///> VM outer interpreter proxy
///
void ef_run()
{
	vm_outer();
}
///
///> VM RAM space
///
char *ef_ram(int i) {
    return (char*)&ram[i - FORTH_RAM_ADDR];
}

#else  // !ARDUINO

static U8 _rom[FORTH_ROM_SZ] = {}; ///< fake rom to simulate run time
const char code[] =
    "words\n"
    "123 456\n"
    "+\n"
    "0 CALL\n"
    "123 456\n"
    "1 CALL\n"
    "=\n"
    "BYE\n";
///
///> main to support C development debugging
///
void my_dot() {
	printf(" => my_dot()\n");
}

void my_add() {
	int b = vm_pop();
    int a = vm_pop();
	printf(" => my_add(%d, %d)\n", a, b);
	vm_push(a + b);
}

int main(int ac, char* av[]) {
    setvbuf(stdout, NULL, _IONBF, 0);    /// * autoflush (turn STDOUT buffering off)

    int sz  = ef_assemble(_rom);         ///< fill ROM for testing
    U8 *ram = (U8*)malloc(FORTH_RAM_SZ); ///< forth memory block dynamic allocated
    _stat(_rom, sz, NULL);

    vm_cfunc(0, my_dot);                 ///< register C API[0]
    vm_cfunc(1, my_add);                 ///< register C API[1]
    
    vm_init((char*)_rom, ram, 0, code);
    vm_outer();

    return 0;
}
#endif // ARDUINO

