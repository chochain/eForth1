#include "eforth.h"

extern int  assemble(U8 *cdata, XA *rack);
extern void vm_init(U8 *cdata, XA *rack, S16 *stack);
extern void vm_run();

U8 _mem[FORTH_MEM_SZ] = {};           		// 4K forth memory block

void dump_data(U8* byte, int len) {
#if ASM_TRACE
    for (int p=0; p<len+0x20; p+=0x20) {
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
#endif // ASM_TRACE
}

void setup()
{
    Serial.begin(115200);
    
	U8  *cdata = _mem;
    XA  *rack  = (XA*)&_mem[FORTH_RACK_ADDR];
    S16 *stack = (XA*)&_mem[FORTH_STACK_ADDR];
	int sz  = assemble(cdata, rack);
	dump_data(cdata, sz);

	PRINTF("\nROM[%04x]\n", sz);
	vm_init(cdata, rack, stack);
}

void loop()
{
    while (!Serial.available());
    
	vm_run();
}


