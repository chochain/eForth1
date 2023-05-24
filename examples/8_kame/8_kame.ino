///
/// @file
/// @brief - Kame, a popular 3D printable model for 4-legged robot with 8 servos
///   this sketch demonstrates how eForth1 can control the robot
///   with HC-SR04 ultrasound and HX1838 IR remote
///   + pin 2 is used for IR receiver 
///   + pin 3, 9 are used for ultrasound ECHO and TRIG
///   + pin 4, 5, 6, 7 are attached to FL, FR, RL, RR femur servos
///   + pin A5, A4, A3, A2 attached to FL, FR, RL, RR tibia servos
///
#include <eForth1.h>
///
///> common types
///
typedef uint8_t  U8;
typedef uint16_t U16;
typedef uint32_t U32;
///=======================================================================
///> IR controls with IRremote library using hardware interrupt INT0
//
#define IR_RECEIVE_PIN 2         /** external interrupt INT0 */
#define USE_ATTACH_INTERRUPT     /** enforce hardward interrupt */
#define NO_LED_FEEDBACK_CODE     /** save some bytes */
#include <TinyIRReceiver.hpp>
volatile struct TinyIRReceiverCallbackDataStruct ir0;
///
///> default INT0 ISR callback function (single capture)
///
void handleReceivedTinyIRData(U8 addr, U8 cmd, U8 flags) {
    ir0.Command     = cmd;
    ir0.Flags       = flags;
    ir0.justWritten = true;
}
///
/// NEC protocol tranlator for IR remote keypad
///
int _ir2d(U16 iv) {
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
///
///> C-API furntion to read IR code
///
void ir_read() {
    int rst = -1;
    if (ir0.justWritten) {
        ir0.justWritten = false;
        rst = _ir2d(ir0.Command);
    }
    vm_push(rst);
}
///=======================================================================
///> ultrasound using hardward interrupt INT1
///  @brief -
///     + ECHO_PIN rised to HIGH after the ultrasound pulses been send out
///     + it stay HIGH during the measuring
///     + it goes LOW once echo signal bouncing from distant object is received
///
#define TRIG_PIN 9
#define ECHO_PIN 3             /** hardware interrupt 1 for UNO, Nano */
volatile U16 d0 = 400;
///
///> ultrasound interrupt handler
///
void sonar() {
    static volatile U32 t0 = 0;
    if (digitalRead(ECHO_PIN) == HIGH) t0 = micros();
    else {
        U16 d = (U16)((micros() - t0) * 17 / 100);
        if (d < 400 && d > 30) d0 = (d0 + d) / 2;
    }
}

void usound_setup() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), sonar, CHANGE); // hardware interrupt
}
///
///> C-API functo to fetch previous distance read, and trigger next read
///
void dist() {
    digitalWrite(13, digitalRead(13)^1);  // blinks
    vm_push(d0);                          // return previous captured distance
    /// trigger next read
    digitalWrite(TRIG_PIN, LOW);          // clear trigger pin
    delayMicroseconds(2);          
    digitalWrite(TRIG_PIN, HIGH);         // send 10us pulse 
    delayMicroseconds(10);    
    digitalWrite(TRIG_PIN, LOW);
}
///=======================================================================
///> Servo control section
///
#include <Servo.h>
Servo sv[8];                                             ///>  8 servo instances
PROGMEM const U8 pin[] = { 4, 5, 6, 7, A5, A4, A3, A2 }; /// * (FL0, FR0, RL0, RR0) (FL1, FR1, RL1, RR1)
U8 rev = B01101010;                                      /// * reverse servo map (front, up is 0)

void servo_setup() {
    for (U8 i=0; i<8; i++) {
        U16 p = (U16)pgm_read_byte((U8*)&pin+i);
        pinMode(p, OUTPUT);
        sv[i].attach(p);                  /// * attach pins to servos
    }
}
///
///> C-API function to set servo angle  (cyclic motion - sine wave)
///  @parameters on data stack
///     (phase<<8) | (amplitude)
///     (center<<8) | (servo id)
///
///  TODO: consider cardioid (r = 1 - cos(ph))
///
const float PERIOD PROGMEM = PI / 128.0;        /// 360 degree in 256 partions
void servo() {                                  /// ( ph|amp ctr|id -- )
    U16   ct_id = vm_pop();                     ///> (center<<8) | servo id
    U16   ph_am = vm_pop();                     ///> (cyclic phase angle<<8) | amplitude
    float v = sin(PERIOD * (ph_am >> 8));       ///> directional vector 
    int   d = static_cast<int>(v * (ph_am & 0xff) + 0.5);  ///> delta = dir_vector * amplitude
    U8    i  = ct_id & 0xff;                    ///> servo id
    int   a = d + (ct_id >> 8);                 ///> angle = delta + center
    sv[i].write((rev >> i) & 1 ? 180 - a : a);  ///> set servo# angle
}
//
// servo cyclic control data structure
//   U8 ph     (cyclic phase angle 0~255)
//   U8 amp    (amplitude, stride range)
//   U8 ctr    (servo center angle)
//   U8 id     (servo id 0~7)
//
PROGMEM const char code[] = "\n"
"CREATE cyc 32 ALLOT\n"                                   // 8 4-byte cyclic controls
"CREATE inc 8 ALLOT\n"                                    // 8 1-byte phase angle increment (=~freq)
": u16 SWAP 8 LSHIFT OR ;\n"                              // ( u8 u8 -- u16 ) combine 2 8-bit to a 16-bit
": ci 2 LSHIFT cyc + ;\n"                                 // ( i -- ) get index to &cyc[i]
": cyc! DUP ci >R u16 >R u16 R> R> 2! ;\n"                // ( ph amp ctr id -- ) store cyclic record 
": fmr 3 FOR DUP I inc + C! NEXT DROP ;\n"                // ( a -- ) set phase angles to all femurs
": tba 3 FOR DUP I 4 + inc + C! NEXT DROP ;\n"            // ( a -- ) set phase angles to all tibias
": home\n"                                                // ( -- ) default servo control parameters
"  3 FOR 0 40 80 I cyc! 0 20 90 I 4 + cyc! NEXT\n"        
"  0 fmr 8 tba ; home\n"
": svo 7 FOR I ci 2@ 0 CALL NEXT ; ' svo 32 0 TMISR\n"    // update servo with their controls every 32ms
"CREATE H $0000 , $0000 , $0000 , $0000 ,\n"              // Home posture phase angles array (8x1-byte)
"CREATE F $8000 , $0080 , $C040 , $40C0 ,\n"              // forward phase angles array
"CREATE B $8000 , $0080 , $40C0 , $C040 ,\n"              // backward
"CREATE L $8080 , $0000 , $40C0 , $C040 ,\n"              // left turn
"CREATE R $8080 , $0000 , $C040 , $40C0 ,\n"              // right turn
"CREATE vt H , L , F , R , L , H , R , L , B , R ,\n"     // command dispatch table (for keypad 0~9)
": adv DUP inc + C@ SWAP ci DUP >R C@ + R> C! ;\n"        // ( i -- ) advance servo[i] phase angle
": step 7 FOR I adv NEXT ; ' step 64 1 TMISR\n"           // update servo phase angles every 64ms
": ph! 7 FOR DUP I + C@ I ci C! NEXT DROP ;\n"            // ( pa -- ) set phase angles given an array
": cmd home DUP 0> IF 8 fmr THEN CELLS vt + @ ph! ;\n"    // ( i -- ) IR command dispatcher
": tic 2 CALL DUP 0< IF DROP ELSE cmd THEN ;\n"           // read IR and execute command if keypad pressed
"' tic 128 2 TMISR\n"                                     // check IR remote every 128ms
"1 timer\n"
;

void setup() {
    Serial.begin(115200);
    while (!Serial);                      // wait for serial port to connect

    initPCIInterruptForTinyReceiver();    // Enables INT0 (pin 2) on change of IR input
    ///
    /// initialize devices and Forth VM
    usound_setup();
    servo_setup();
    ef_setup(code);                       // passing code to Forth 
    ///
    /// assign functions to C-API slots
    ///
    vm_cfunc(0, servo);
    vm_cfunc(1, dist);
    vm_cfunc(2, ir_read);
}

void loop() {
    ef_run();
}

