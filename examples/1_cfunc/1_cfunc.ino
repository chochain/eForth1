///
/// @file
/// @brief - your eForth can interact with user defined functions
/// shown in Wokwi project https://wokwi.com/projects/365630462031711233
///
/// Enter the following Forth code into Serial Monitor input
///  : flip 0 call ;                   \ ( -- ) toggle the red LED 
///  : red 5 1 call ;                  \ ( -- ) turn on the LED on pin 5 (red)
///  : blue 6 1 call ;                 \ ( -- ) turn on the LED on pin 6 (blue)
///  : on? 2 call ;                    \ ( n -- ) read the value on a given pin
///  flip                              \ toggle the red LED to on
///  5 on?                             \ query whether pin 5 (red LED) is on?
///
#include <eForth1.h>

void led_flip() {                      /// ( -- ) no input
     digitalWrite(5, digitalRead(5) ^ 1);
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

    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);

    ef_setup();
    
    vm_cfunc(0, led_flip);
    vm_cfunc(1, led_write);
    vm_cfunc(2, led_read);
}

void loop()
{
    ef_run();
}


