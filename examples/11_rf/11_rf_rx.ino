/**
 * @file
 * @brief - nRF24L01 wireless demo - receiver
 *
 * Receive data packet periodically from remote sender
 *
 * Assuming you have
 *   1) nRF24L01 CE/CSN on 9,10 and MOSI, MISO, SLK on 11, 12, 13
 *
 * Tx module sending the following record
 *   typedef struct {
 *     uint16_t n;                  // record counter
 *     uint16_t ht;                 // huminity:temperature
 *     uint16_t d;                  // distance
 *   } Pkt;
 *
 */
#include <eForth1.h>

PROGMEM const char code[] =
"CREATE pk 3 CELLS ALLOT\n"         // create 3-cell packet storage
": bx DUP $FF AND 8 LSHIFT\n"       // flip value to big-endian
"  SWAP 8 RSHIFT $FF AND OR ;\n"
": hit pk 0 CALL ;\n"               // ( -- ) call rf_hit
": dsp pk @ bx U. .\" >\"\n"        // ( -- ) display packet
"  pk 2 CELLS + @ bx .\"  Dist:\" U.\n"
"  pk CELL + @ bx DUP\n"
"  100 / .\"  Hum:\" U.\n"
"  100 MOD .\"  Temp:\" U. CR ;\n"
": rcv hit IF dsp THEN ;\n"         // ( -- ) call rf_read if hit
"' rcv 1000 0 TMISR\n"              //        check every second
"1 TIMER\n"
;

#define INT(v)       static_cast<int>(v + 0.5)
#define IRQ_PIN      2      /* hardware interrupt, if needed */
#define RF_CE        9
#define RF_CSN       10
#define RF_ID        0      /* current RF device ID */

#include <SPI.h>
#include <NRFLite.h>
NRFLite  rf;

void rf_hit() {             // ( a -- f )
    int     i   = vm_pop();
    uint8_t *p  = (uint8_t*)ef_ram(i);
    
    if (rf.hasData()) {
        rf.readData(p);
        vm_push(-1);            // true
    }
    else vm_push(0);            // false
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);            // wait for serial port to connect

    pinMode(RF_CE,  OUTPUT);
    pinMode(RF_CSN, OUTPUT);

    if (!rf.init(RF_ID, RF_CE, RF_CSN)) {
        Serial.println(F("RF.init failed!"));
    }

    ef_setup(code);             // initialize VM (with embedded Forth)
    vm_cfunc(0, rf_hit);        // register rf.hasData
}

void loop()
{
    ef_run();
}
