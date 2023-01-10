#ifndef __EFORTH_EFORTH1_H
#define __EFORTH_EFORTH1_H

#if ARDUINO
#include <Arduino.h>
#include <time.h>
#include <pt.h>

void ef_add_task(char (*task)());
void ef_setup(Stream &io_stream=Serial);
void ef_run();
#endif // ARDUINO

#endif // __EFORTH_EFORTH1_H

