/**
 * @file eforth_core.cpp
 * @brief eForth core controller module
 */
#include "eforth_core.h"

static U8 _ram[FORTH_RAM_SZ] = {};         ///< 4K forth memory block
static Stream   *io;                       ///< console interface
static task_ptr _task_list  = NULL;        ///< user task linked-list
///
/// version prompt
///
void ef_prompt()
{
    LOG("\r\neForthUNO v1.0");
}
///
/// display eForth system information
///
void sys_info(U8 *cdata) {
    LOG_H("\r\nRAM_SZ= x",  FORTH_RAM_SZ);
    LOG_V(", Primitives=",  FORTH_PRIMITIVES);
    LOG_V(", Addr=",        (U16)sizeof(XA)*8);
    LOG_V("-bit, CELLSZ=",  CELLSZ);
    LOG("\r\nMEMMAP:");
    LOG_H("\r\n  ROM   x0000+", FORTH_ROM_SZ);
    LOG_H("\r\n  STACK x", FORTH_STACK_ADDR); LOG_H("+", FORTH_STACK_SZ);
    LOG_H("\r\n  TIB   x", FORTH_TIB_ADDR);   LOG_H("+", FORTH_TIB_SZ);
    LOG_H("\r\n  USER  x", FORTH_TVAR_ADDR);  LOG_H("+", FORTH_DIC_ADDR-FORTH_TVAR_ADDR);
    LOG_H("\r\n  DIC   x", FORTH_DIC_ADDR);   LOG_H("+", FORTH_RAM_ADDR+FORTH_RAM_SZ-FORTH_DIC_ADDR);

#if ARDUINO
    U16 h = (U16)&cdata[FORTH_RAM_SZ];
    U16 s = (U16)&h;
    LOG_H("\r\nHEAP=x", h);
    LOG_H("--> x",    s-h);
    LOG_H(" <--SP=x", s);
#endif // ARDUINO
}
///
/// add user defined task to task queue
///
void ef_add_task(char (*task)()) {
    task_ptr tp = (task_ptr)malloc(sizeof(ef_task));
    
    tp->task = task;
    tp->next = _task_list;

    _task_list = tp;                       // linked list
}

#if ARDUINO
///
/// eForth yield to user tasks
///
void ef_yield()
{
    task_ptr tp=_task_list;
    while (tp) {
        PT_SCHEDULE(tp->task());           // steal cycles for hardware tasks
        tp = tp->next;
    }
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
///
/// setup (called by Arduino setup)
///
void ef_setup(Stream &io_stream)
{
    io = &io_stream;

    sys_info(_ram);
	vm_init((PGM_P)forth_rom, _ram, (void*)&io_stream);
}
///
/// single step eForth virtual machine
///
void ef_run()
{
    vm_step();
    ef_yield();
}

#else // ARDUINO

void ef_yield()         {}
void ef_wait(U32 ms)    {}
U8   ef_getchar()	    { return getchar(); }
void ef_putchar(char c) { printf("%c", c);  }

static U8 _rom[FORTH_ROM_SZ] = {};			// fake rom to simulate run time
///
/// main to support C development debugging
///
int main(int ac, char* av[])
{
	setvbuf(stdout, NULL, _IONBF, 0);		// autoflush (turn STDOUT buffering off)

	int sz = ef_assemble(_rom);
	ef_dump_rom(_rom, sz+0x20);

#if !ROM_DUMP_ONLY
	sys_info(_rom);
	vm_init((PGM_P)_rom, _ram, NULL);
	while (vm_step());
#endif // !ROM_DUMP_ONLY

	return 0;
}
#endif // ARDUINO
