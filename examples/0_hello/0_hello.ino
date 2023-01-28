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


