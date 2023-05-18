///
/// @file
/// @brief - eForth1 servo demo
///
/// show also in Wokwi: https://wokwi.com/projects/356866133593965569
/// In the Serial Monitor input bar, enter the following
///   1 timer     \ enable timer interrupt
///
#include <Servo.h>
#include <eForth1.h>

Servo sv[8];

PROGMEM const char code[] =
"CREATE w 8 ALLOT\n"
": xx ( -- ) 7 FOR I DUP 1 + 10 * SWAP w + C! NEXT ; xx\n"
": w@ ( n -- c w ) 90 SWAP w + C@ ;\n"
": ang= ( ph n -- ) SWAP OVER w@ ROT 1 CALL SWAP 0 CALL ;\n"
"VARIABLE x\n"
": step 7 FOR DUP I ang= NEXT DROP ;\n"
": swing x @ DUP 1+ x ! step ;\n"
"' swing 100 0 TMISR\n"
;

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

    ef_setup(code);                    ///> create eForth1 instance
    
    vm_cfunc(0, servo);                ///> register servo function
    vm_cfunc(1, pos);                  ///> register pos function
}

void loop() {
    ef_run();                          ///> invoke eForth1 outer interpreter
}

