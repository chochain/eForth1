///
/// @file
/// @brief - your eForth for Arduino, no more no less
///
/// Try type the following into Serial Monitor input bar
///   ok> words
///   ok> 111 222 +
///   ok> 1 13 out
///   ok> : xx 13 in 1 xor 13 out ;
///   ok> see xx
///   ok> xx
///   ok> ' xx 50 tmisr
///   ok> 1 timer
///   ok> : aa 99 for 123 drop next ; : bb 99 for aa next ;
///   ok> clock dnegate bb clock d+
///   ok> hex
///   ok> 2000 80 dump
///
#include <eForth1.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect

    ef_setup();
}

void loop()
{
    ef_run();
}
