/**
 * @file
 * @brief - Temperature & Humidity Measure (with 4 digit 7-segment LED driver)
 *
 * Assuming you have a DHT11 sensor and a 4-digit 7-segment LED
 * 12-pin 4 digit 7-segment LED hooked up as following
 *      A             D4 A  F  D3 D2 B
 *     ---       +----^--^--^--^--^--^---+   
 *  F |   | B    |     |     |     |     |
 *     -G-       |  8  |  8  |  8  |  8  |
 *  E |   | C    |     |     |     |     |
 *     ---       +----v--v--v--v--v--v---+
 *      D  *DP        E  D  DP C  G  D1
 *
 *      A,B,C,D,E,F,G,DP => 4,5,6,7,8,9,10,11
 *      DIG1,2,3,4       => A0,A1,A2,A3 (i.e. digital pin 14,15,16,17)
 *
 * DHT11 on pin A6 (i.e. digital pin 20)
 */
#include <eForth1.h>

PROGMEM const char code[] =
": ini 15 FOR 1 I 2+ PINMODE NEXT ; ini FORGET ini\n"  // set pin 2~17 to OUTPUT
"CREATE x $F360 , $B5F4 , $66D6 , $D770 , $F776 ,\n"   // x keeps 7-seg pin patterns of 0~9
"CREATE d $0E0D , $0B07 ,\n"                           // d keeps (0~3) digit control pin patterns
": 7d d + C@ $30F OUT ;\n"                             // ( n -- ) set output digit
"CREATE vx 4 ALLOT\n"                                  // vx cache pattern for 4 digits before display
": v! 3 FOR DUP 10 MOD x + C@ I vx + C! 10 / NEXT DROP ;\n"  // ( n -- ) process number into 4-digit bit patterns
": 7s DUP 7d vx + C@ DUP $1F0 OUT $20F OUT ;\n"        // ( n -- ) display n'th digit of vx on 7-seg
"VARIABLE n\n"                                         // nth digit of the 7-seg to display
": dsp n @ 1+ 3 AND DUP n ! 7s ; ' dsp 5 0 TMISR\n"    // ( -- ) scan thru the 4 digits every 5ms
": dht 0 CALL v! ;\n"                                  // ( -- ) read DHT every 1s
"' dht 2000 1 TMISR\n"
"1 TIMER\n"
;

#include <dht.h>
#define DHT11_PIN 20            // i.e. A6
dht DHT;

void dht() {
    int chk = DHT.read11(DHT11_PIN);
    int v = (chk == DHTLIB_OK)
        ? static_cast<int>(100.0*DHT.temperature) + static_cast<int>(DHT.humidity)
        : 9999;
    vm_push(v);
    digitalWrite(13, digitalRead(13)^1);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect

    ef_setup(code);           // initialize VM (with embedded Forth)

    vm_cfunc(0, dht);         // register DHT function
}

void loop()
{
    ef_run();
}
