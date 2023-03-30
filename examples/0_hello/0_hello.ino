///
/// @file
/// @brief - your eForth for Arduino, no more no less
///
/// Try type the following into Serial Monitor input bar
///   words
///   111 222 +
///   1 13 out
///   : tic 13 in 1 xor 13 out ;
///   see tic
///   tic
///   ' tic 500 0 tmisr
///   1 timer
///   : inner 99 for 123 drop next ;
///   : outer 99 for inner next ;
///   : bench clock dnegate outer clock d+ ;
///   $2000 $80 dump
///   bench
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
