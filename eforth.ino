#include "eforth.h"
#include <time.h>

extern int  assemble(U8 *cdata, U8 *stack);
extern void vm_init(U8 *cdata, U8 *stack);
extern void vm_run();

U8 _mem[FORTH_MEM_SZ] = {};           		// 4K forth memory block

void dump_data(U8* byte, int len) {
#if ASM_TRACE
    for (int p=0; p<len+0x20; p+=0x20) {
        PRINTF("\n%04x: ", p);
        for (int i=0; i<0x20; i++) {
        	U8 c = byte[p+i];
            PRINTF("%02x%s", c, (i%4)==3 ? " " : "");
        }
        for (int i=0; i<0x20; i++) {
            U8 c = byte[p+i];
            PRINTF("%c", c ? ((c>32 && c<127) ? c : '_') : '.');
        }
    }
    PRINTF("\nPrimitives=%d, Addr=%d-bit, CELLSZ=%d", FORTH_PRIMITIVES, 8*sizeof(XA), CELLSZ);
    PRINTF("\nHEAP = x%x", FORTH_MEM_SZ);
    PRINTF("\n  BOOT  x%04x", FORTH_BOOT_ADDR);
    PRINTF("\n  USER  x%04x+%04x", FORTH_TVAR_ADDR,  FORTH_TIB_ADDR-FORTH_TVAR_ADDR);
    PRINTF("\n  TIB   x%04x+%04x", FORTH_TIB_ADDR,   FORTH_TIB_SZ);
    PRINTF("\n  STACK x%04x+%04x", FORTH_STACK_ADDR, FORTH_STACK_SZ);
    PRINTF("\n  DIC   x%04x+%04x", FORTH_DIC_ADDR,   FORTH_MEM_SZ-FORTH_DIC_ADDR);
    PRINTF("\nHERE    x%x", len);
#endif // ASM_TRACE
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect
    
	U8 *cdata = _mem;
    U8 stack;
	int sz  = assemble(cdata, &stack);
	dump_data(cdata, sz);

	vm_init(cdata, &stack);
}

void loop()
{
    while (!Serial.available());
    
//	vm_run();
}


