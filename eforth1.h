#ifndef __EFORTH_EFORTH1_H
#define __EFORTH_EFORTH1_H
#include <Arduino.h>
#include <time.h>
#include <pt.h>

void ef_add_task(char (*task)());
void ef_setup(Stream &io_stream=Serial);
void ef_run();

#endif // __EFORTH_EFORTH1_H

