/* 
 eForth1 Servos+Knobs Demo

 Enter the following into Serial Monitor input
   : a2d >r 0 1023 0 180 r> map ;    \ ( a -- d ) map 0-1023 to 0-180
   : svo dup ain a2d swap 0 call ;   \ ( n -- ) read knob[n], convert to angle, set servo[n]
   : x4 3 for r@ svo next ;          \ ( -- ) scan thru all 4 knobs
 Option1: 
   : my_app begin x4 again ;         \ ( -- ) an infinite loop
   my_app                            \ this is the typical way of an Arduino app

 Option2: or, use timer interrupt to make it multi-tasking
   ' x4 10 tmisr                     \ make x4 a timer ISR
   1 timer                           \ enable timer interrupt
   words                             \ Forth interpreter is still in service
*/   
#include <Servo.h>
#include <eForth1.h>

Servo sv[4];               ///> create 4 servos

/// servo arm angle setting ( angle servo# -- )
void servo() {
  int p = vm_pop();        ///> servo#
  int a = vm_pop();        ///> servo phase angle
  sv[p].write(a);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  for (int i=0; i<4; i++) {
    sv[i].attach(6 + i);    /// * servos attached to pin 6,7,8,9
  }

  ef_setup();               ///> initiate eForth1
  vm_cfunc(0, servo);       ///> register servo write function
}

void loop() {
  ef_run();                 ///> invoke eForth1 VM outer interpreter
}
