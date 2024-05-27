#ifndef __EFORTH1_H
#define __EFORTH1_H

typedef void (*CFP)();            ///> function pointer

void vm_cfunc(int n, CFP fp);     ///< assign C interface (in slot n)
void vm_push(int v);              ///< push value onto VM data stack
int  vm_pop();                    ///< pop TOS off VM data stack

#if ARDUINO
#include <Arduino.h>
#include <time.h>

void ef_setup(const char *code=0, Stream &io_stream=Serial);
void ef_run();
char *ef_ram(int i);              ///< expose VM RAM space to Sketch

#endif // ARDUINO
#endif // __EFORTH1_H

