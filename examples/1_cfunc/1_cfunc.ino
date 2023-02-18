#include <eForth1.h>

void led_flip() {                      /// ( -- ) no input
     digitalWrite(13, digitalRead(13) ^ 1);
}

void led_write() {                     /// ( n -- ) take one input from stack
     int t = vm_pop();
     digitalWrite(t, HIGH);
}

void led_read() {                      /// ( n -- v ) one input, and one output
     int t = vm_pop();
     int v = digitalRead(t);
     vm_push(v);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);                   // wait for serial port to connect

    for (int i=0; i<8; i++) {
        int p = (i < 4 ? 4 : 16) + i;
        pinMode(p, OUTPUT);            // 4,5,6,7,16,17,18,19
    }
    ef_setup();
    
    vm_cfunc(0, led_flip);
    vm_cfunc(1, led_write);
    vm_cfunc(2, led_read);
}

void loop()
{
    ef_run();
}


