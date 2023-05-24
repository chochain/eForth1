///
/// @file
/// @brief - HC-SR04 ultrasound
/// Note: Due to interrupt handling in eForth1 is not real-time, i.e.
///       Forth VM poll interrupt flags every certain (~100) opcodes are executed.
///       This cause a delay in executing Forth handler.
///       For ultrasound type of application, 500us delay can cause offset by 17cm.
///       So, using of hardware interrupt handler is more appropriate.
///
#include <eForth1.h>

#define TRIG_PIN 9
#define ECHO_PIN 3                        /** hardware interrupt for UNO, Nano */

PROGMEM const char code[] =
": dist 0 CALL . CR ;\n"                  // fetch last read, and trigger next read
"' dist 1000 0 TMISR\n"                   // timer ISR slot0, kicks every 100ms
"1 TIMER\n"
;

///
///> hardware interrupt handler
///  @brief -
///     + ECHO_PIN rised to HIGH after the ultrasound pulses been send out
///     + it stay HIGH during the measuring
///     + it goes LOW once echo signal bouncing from distant object is received
///
volatile unsigned int d0 = 0;
void echo() {
    static volatile unsigned long t0 = 0;
    d0 = (digitalRead(ECHO_PIN) == HIGH)
        ? (t0=micros(), 0)
        : (unsigned int)((micros() - t0) * 17 / 100);
}
///
///> trigger - to begin sending ultrasound pulses
///
void trig() {
    digitalWrite(TRIG_PIN, LOW);          // clear trigger pin
    delayMicroseconds(2);          
    digitalWrite(TRIG_PIN, HIGH);         // send 10us pulse 
    delayMicroseconds(10);    
    digitalWrite(TRIG_PIN, LOW);
}
///
///> fetch previous distance read, and trigger next read
///
void dist() {
    digitalWrite(13, digitalRead(13)^1);  // blinks
    vm_push(d0);                          // return previous captured distance
    trig();                               // trigger next read
}

void setup() {
    Serial.begin(115200);
    while (!Serial);                     // wait for serial port to connect

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echo, CHANGE); // hardware interrupt

    ef_setup(code);                      // initialize VM (with embedded Forth)
    vm_cfunc(0, dist);
}

void loop() {
    ef_run();
}

