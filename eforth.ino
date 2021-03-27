#include "eforth.h"

extern int  assemble(U8 *rom);
extern void vm_init(U8 *rom);
extern void vm_run();

U32 data[FORTH_DATA_SZ] = {};           		// 64K forth memory block

void dump_data(U8* byte, int len) {
#if DATA_DUMP
    for (int p=0; p<len; p+=0x20) {
        PRINTF("\n%04x: ", p);
        for (int i=0; i<0x20; i++) {
        	U8 c = byte[p+i];
            PRINTF("%02x", c);
            PRINTF("%s", (i%4)==3 ? " " : "");
        }
        for (int i=0; i<0x20; i++) {
            U8 c = byte[p+i];
            PRINTF("%c", c ? ((c>32 && c<127) ? c : '_') : '.');
        }
    }
#endif // DATA_DUMP
}

void setup()
{
    Serial.begin(115200);
    
	U8 *rom = (U8*)data;
	setvbuf(stdout, NULL, _IONBF, 0);		// autoflush (turn STDOUT buffering off)

	int sz  = assemble(rom);
	dump_data(rom, sz+0x20);

	PRINTF("\nceForth v4.0 ROM[%04x]\n", sz);
	vm_init(rom);
}

void loop()
{
    while (!Serial.available());
    
	vm_run();
}


