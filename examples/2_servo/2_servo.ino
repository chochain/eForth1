///
/// @file
/// @brief - eForth1 servo demo
///
/// from Wokwi: https://wokwi.com/projects/356866133593965569 , enter the following
///   ok> : sync 7 for dup r@ 0 call next drop ;  \ sync 8 servos
///   ok> : rnd clock drop abs 180 mod ;          \ randomize servo angle
///   ok> : sweep rnd sync ;                      \ 8 servos in action
///   ok> : ' sweep 50 tmisr                      \ make sweep as ISR at 500ms
///   ok> : 1 timer                               \ timer on
///
#include <Servo.h>
#include <eForth1.h>

Servo sv[8];

void servo() {
    int n = vm_pop();      ///> servo id
    int a = vm_pop();      ///> angle
    sv[n].write(a);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);                   /// wait for serial port to connect

    ef_setup();                        /// * init eForth1
    
    vm_cfunc(0, servo);                /// * register servo as cfunc[0]

    for (int i=0; i<8; i++) { 
        sv[i].attach(4+i);             /// * 4,5,6,...,11
    }
}

void loop()
{
    ef_run();                          /// * enter eForth1 outer interpreter
}


