/**
 * @file
 * @brief - 4 digit 7-segment LED driver
 *
 *  Assuming you have a 12-pin 4 digit 7-segment LED hooked up as following
 *
 *      A             D4 A  F  D3 D2 B
 *     ---       +----^--^--^--^--^--^---+   
 *  F |   | B    |     |     |     |     |
 *     -G-       |  8  |  8  |  8  |  8  |
 *  E |   | C    |     |     |     |     |
 *     ---       +----v--v--v--v--v--v---+
 *      D  *DP        E  D  DP C  G  D1
 *
 *  + A,B,C,D,E,F,G,DP => 4,5,6,7,8,9,10,11
 *  + D1,D2,D3,D4      => A0,A1,A2,A3       i.e. digital pin 14,15,16,17
 *
 *  How To:
 *  Open Serial Monitor (or your favorate terminal emulator) as the console input to oneForth
 *  + baud rate set to 115200
 *  + line ending set to Both NL & CR (if using emulator, set Add CR on, ECHO on)
 *  + type the following Forth code into Serial Monitor input
 *   ok> 1 timer
 */
 #include <eForth1.h>

PROGMEM const char code[] =
": ini 15 FOR 1 I 2+ PINMODE NEXT ; ini FORGET ini\n"  // set pin 2~17 to OUTPUT
"CREATE x $F360 , $B5F4 , $66D6 , $D770 , $F776 ,\n"   // x keeps 7-seg pin patterns of 0~9, 1-byte each
"CREATE vx 4 ALLOT\n"                                  // vx cache pattern for 4 digits before display
": v! 3 FOR DUP 10 MOD x + C@ I vx + C! 10 / NEXT DROP ;\n"  // ( n -- ) process number into 4-digit bit patterns
"CREATE d $0E0D , $0B07 ,\n"                           // d keeps digit control pin patterns
": sd d + C@ $30F OUT ;\n"                             // ( n -- ) set output digit
": 7s DUP sd vx + C@ DUP $1F0 OUT $20F OUT ;\n"        // ( n -- ) display n'th digit of vx on 7-seg
"VARIABLE i\n"                                         // keep i'th digit of the 7-seg to display
": dsp i @ 1+ 3 AND DUP i ! 7s ;\n"                    // ( -- ) cycle i and display 
"' dsp 5 0 TMISR\n"                                    // timer interrupt to update display every 5ms
"VARIABLE cnt 0 cnt !\n"                               // cnt is a counter we want it value on display
": c++ cnt @ 1+ DUP 9999 > IF DROP 0 THEN DUP cnt ! v! ;\n"  // ( -- ) increment the counter and cache patterns
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
