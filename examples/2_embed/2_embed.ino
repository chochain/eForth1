
/// @file
/// @brief - eForth can embed Forth in the .ino sketch
///
/// Try type the following into Serial Monitor input bar
///   words
///   ' tic 500 0 tmisr
///   1 timer
///   bench
///
#include <eForth1.h>

const char code[] PROGMEM = 
": tic 13 in 1 xor 13 out ;\n"
": inner 999 for 123 drop next ;\n"
": outer 99 for inner next ;\n"
": bench clock dnegate outer clock d+ ;\n"
;

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect

    ef_setup(code);           // intialize with embedded Forth
}

void loop()
{
    ef_run();
}
