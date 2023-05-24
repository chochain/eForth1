///
/// @file
/// @brief - 4 digit 7-segment LED driver
///
/// Assuming you have a 12-pin 4 digit 7-segment LED hooked up as following
///   * A,B,C,D    => 4,5,6,7
///   * E,F,G,DP   => 8,9,10,11
///   * DIG1,2,3,4 => 2,3,12,13
///   as shown in Wokwi project https://wokwi.com/projects/358961171560345601
///
/// Try type the following into Serial Monitor input bar
///   ok> 1 timer
///
#include <eForth1.h>

PROGMEM const char code[] =
": ini 11 FOR 1 I 2+ PINMODE NEXT ; ini FORGET ini\n"  // set pin 2~13 to OUTPUT
"CREATE x $F360 , $B5F4 , $66D6 , $D770 , $F776 ,\n"   // x keeps 7-seg pin patterns of 0~9
"CREATE d $3834 , $2C1C ,\n"                           // d keeps digit control pin patterns
": 7d d + C@ DUP $10C OUT $230 OUT ;\n"                // ( n -- ) set output digit
"VARIABLE vx 2 ALLOT\n"                                // vx cache pattern for 4 digits before display
": ?v 3 FOR DUP 10 MOD x + C@ I vx + C! 10 / NEXT DROP ;\n"  // ( n -- ) process number into 4-digit bit patterns
": 7s DUP 7d vx + C@ DUP $1F0 OUT $20F OUT ;\n"        // ( n -- ) display n'th digit of vx on 7-seg
"VARIABLE n\n"                                         // nth digit of the 7-seg to display
": dsp n @ 1+ 3 AND DUP n ! 7s ;\n"                    // ( -- ) increment n (3 max) and display 
"' dsp 5 0 TMISR\n"                                    // timer interrupt to update display every 5ms
"VARIABLE cnt 0 cnt !\n"                               // cnt is a counter we want it value on display
": c++ cnt @ 1+ DUP 9999 > IF DROP 0 THEN DUP cnt ! ?v ;\n"  // ( -- ) increment the counter and cache patterns
"' c++ 1000 1 TMISR\n"                                 // timer interrupt to update c every second
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
