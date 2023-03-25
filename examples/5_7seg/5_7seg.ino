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
#include "src/eForth1.h"

PROGMEM const char code[] =
": ini 11 FOR 1 I 2 + PINMODE NEXT ; ini FORGET ini\n" // set pin 2~13 to OUTPUT
"VARIABLE x\n"                                         // x keeps 7-seg pin patterns of 0~9
"$F360 x ! $B5F4 , $66D6 , $D770 , $F776 ,\n"
"VARIABLE d $3834 d ! $2C1C ,\n"                       // d keeps digit control pin patterns
": 7d d + C@ DUP $10C OUT $230 OUT ;\n"                // ( n -- ) set output digit
"VARIABLE vx 2 ALLOT\n"                                // vx cache pattern for 4 digits before display
": ?v 3 FOR DUP 10 MOD x + C@ I vx + C! 10 / NEXT DROP ;\n"  // ( n -- ) process number into 4-digit bit patterns
": 7s vx + C@ DUP $1F0 OUT $20F OUT ;\n"               // ( n -- ) display i'th digit on 7-seg
"VARIABLE n\n"                                         // n is the 7-seg digit to display
": n++ n @ 1 + 3 AND DUP n ! ;\n"                      // ( -- ) increment i and keep between 0~3
": dsp n++ DUP 7s 7d ;\n"                              // ( -- ) display 
"' dsp 5 0 TMISR\n"                                    // timer interrupt to update display every 5ms
"VARIABLE cnt 0 cnt !\n"                               // cnt is a counter we want it value on display
": c++ cnt @ 1 + DUP cnt ! ?v ;\n"                     // ( -- ) increment the counter and cache patterns
"' c++ 1000 1 TMISR\n"                                 // timer interrupt to update c every second
": aa 999 FOR 34 DROP NEXT ;\n"
": bb 99 FOR aa NEXT ;\n"
": cc CLOCK DNEGATE bb CLOCK D+ ;\n"
;

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect

    ef_setup(code);           // initialize VM (with embedded Forth)
}

void loop()
{
    ef_run();
}
