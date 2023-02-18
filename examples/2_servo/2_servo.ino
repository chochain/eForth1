#include <Servo.h>
#include <eForth1.h>

Servo sv[4];

void servo() {
    int n = vm_pop();      ///> servo id
    int a = vm_pop();      ///> angle
    sv[n].write(a);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);                   // wait for serial port to connect

    ef_setup();
    
    vm_cfunc(0, servo);

    for (int i=0; i<4; i++) { 
        sv[i].attach(8+i);
    }
}

void loop()
{
    ef_run();
}


