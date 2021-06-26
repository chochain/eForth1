#include "eforth1.h"
#include <AltSoftSerial.h>

AltSoftSerial bt;            // set RX on 8, TX on 9

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect

    bt.begin(9600);
    delay(1000);

	ef_setup(bt);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    ef_run();
}


