///
/// @file
/// @brief - IR remote
///
/// HX1838 receiver (up to 8m), remote controller using NEC code
/// Note: Aruino Uno support hardware interrupt on only pin 2,3
///
/// Dependency: Arduino IRremote library
///
#include <eForth1.h>

#define IR_RECEIVE_PIN 2
#include <TinyIRReceiver.hpp>  // a part of IRremote library

PROGMEM const char code[] =
": tic 0 CALL DUP -1 = IF DROP ELSE . CR THEN ;\n"
"' tic 100 0 TMISR\n"          // set timer interrupt to read IR code every 100ms
"1 timer\n"
;
volatile struct TinyIRReceiverCallbackDataStruct ir0;
///
/// INT0 ISR callback function (single capture)
///
void handleReceivedTinyIRData(uint8_t addr, uint8_t cmd, uint8_t flags) {
    ir0.Command     = cmd;
    ir0.Flags       = flags;
    ir0.justWritten = true;
}
///
/// NEC protocol map
///
int _ir2d(int iv) {
     switch(iv) {
     case 0x45: return -11;  case 0x46: return -10;  case 0x47: return -12;   // ch-  ch   ch+
     case 0x44: return -101; case 0x40: return -102; case 0x43: return -100;  // |<<  >>|  >||
     case 0x07: return 11;   case 0x15: return 12;   case 0x09: return 10;    //  -    +    EQ
     case 0x16: return 0;    case 0x19: return 100;  case 0x0d: return 200;   //  0   +100 +200
     case 0x0c: return 1;    case 0x18: return 2;    case 0x5e: return 3;     //  1    2    3
     case 0x08: return 4;    case 0x1c: return 5;    case 0x5a: return 6;     //  4    5    6
     case 0x42: return 7;    case 0x52: return 8;    case 0x4a: return 9;     //  7    8    9
     default:
         Serial.print("IR>>"); Serial.println(iv, HEX);
         return -1;
     }
}

void ir_read() {
    int rst = -1;
    if (ir0.justWritten) {
        ir0.justWritten = false;
        rst = _ir2d(ir0.Command);
    }
    vm_push(rst);
}

void setup() {
    Serial.begin(115200);
    while (!Serial);                    // wait for serial port to connect

    Serial.println(F("\nTinyIR library version " VERSION_TINYIR));
    initPCIInterruptForTinyReceiver();  // Enables INT0 (pin 2) on change of IR input
    
    ef_setup(code);                     // initialize VM (with embedded Forth)
    vm_cfunc(0, ir_read);
}

void loop() {
    ef_run();
}

