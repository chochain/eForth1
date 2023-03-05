///
/// @file
/// @brief - eForth1 servo demo
///
/// from Wokwi: https://wokwi.com/projects/356866133593965569 , enter the following
/// Example1:
///   ok> : sync 7 for dup r@ 0 call next drop ;  \ sync 8 servos
///   ok> : rnd clock drop abs 180 mod ;          \ randomize servo angle
///   ok> : sweep rnd sync ;                      \ 8 servos in action
///   ok> : ' sweep 50 tmisr                      \ make sweep as ISR at 500ms
///   ok> : 1 timer                               \ timer on
///
/// Example2:
///   ok> variable w 7 cells allot
///   ok> : xx ( -- ) 7 for r@ dup 1 + 10 * swap cells w + ! next ; xx
///   ok> : w@ ( n -- c w ) 90 swap cells w + @ ;
///   ok> : ang= ( ph n -- ) swap over w@ rot 1 call swap 0 call ;
///   ok> variable x
///   ok> : step 7 for dup r@ ang= next drop ;
///   ok> : swing x @ dup 1+ x ! step ;
///   ok> ' swing 10 tmisr
///   ok> 1 timer
///
#include <Servo.h>
#include <eForth1.h>

Servo sv[8];

void servo() {
    int p = vm_pop();         ///> servo id
    int a = vm_pop();         ///> servo angle
    sv[p].write(a);
}

const float PERIOD PROGMEM = 2.0 * PI / 64.0;
void pos() {                           /// ( center amp phase -- angle )
    float v = sin(PERIOD * vm_pop());  ///> directional vector
    int   d = vm_pop() * v;            ///> delta = amplitude * dir_vector
    int   r = static_cast<int>(vm_pop() + d + 0.5);
    vm_push(r);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(12, OUTPUT);               ///> assume we have an LED attached
    pinMode(13, OUTPUT);               ///> built-in LED
    digitalWrite(12, HIGH);            ///> turn it on so we can play around

    for (int i=0; i<8; i++) {
        sv[i].attach(4 + i);           ///> setup servos on pin 4,5,6,..,10,11
    }

    ef_setup();                        ///> create eForth1 instance
    vm_cfunc(0, servo);                ///> register servo function
    vm_cfunc(1, pos);                  ///> register pos function
}

void loop() {
    ef_run();                          ///> invoke eForth1 outer interpreter
}

