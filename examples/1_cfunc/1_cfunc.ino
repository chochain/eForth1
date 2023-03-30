///
/// @file
/// @brief - your eForth can interact with user defined functions
/// shown in Wokwi project https://wokwi.com/projects/356793878308297729
///
/// Enter the following Forth code into Serial Monitor input
///  : flip 0 call ;                   \ toggle built-in LED
///  : p13on 13 1 call ;               \ turn built-in LED on
///  : p13?  13 2 call ;               \ read pin 13 
///  p13?
///
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

    ef_setup();
    
    vm_cfunc(0, led_flip);
    vm_cfunc(1, led_write);
    vm_cfunc(2, led_read);
}

void loop()
{
    ef_run();
}


