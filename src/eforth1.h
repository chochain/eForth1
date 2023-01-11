#ifndef __EFORTH_EFORTH1_H
#define __EFORTH_EFORTH1_H

#if ARDUINO
#include <Arduino.h>
#include <time.h>

void ef_setup(Stream &io_stream=Serial);
void ef_run();
#endif // ARDUINO

#endif // __EFORTH_EFORTH1_H

