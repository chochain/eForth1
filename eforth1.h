#ifndef __EFORTH_EFORTH1_H
#define __EFORTH_EFORTH1_H

void vm_console(Stream &io_stream);
void ef_add_task(char (*task)());
void ef_setup(Stream &io_stream);
void ef_run();

#endif // __EFORTH_EFORTH1_H

