/**
 * @file
 * @brief - nRF24L01 wireless demo - sender
 * 
 * Remote sensing system which send sensor data packet periodically
 *
 * Assuming you have
 *   1) nRF24L01 CE/CSN on 9,10 and MOSI, MISO, SLK on 11, 12, 13
 *   2) DHT11 humidity and temperature sensor on pin 7, and
 *   3) HC-SR04 ultrasound sensor, trigger on pin 4, echo on pin 3
 */
#include <eForth1.h>

PROGMEM const char code[] =
"CREATE n 0 , CREATE ht 0 , CREATE d 0 ,\n"
": send n @ ht @ d @ 0 CALL 1 n +! ;\n"     // ( -- ) call rf_send
": read 1 CALL ht ! 2 CALL d ! ;\n"         // ( -- ) read sensors
"' read 1000 0 TMISR\n"                     //        every second
"' send 2000 1 TMISR\n"                     // send every 2 seconds
"1 TIMER\n"                                 // enable timer interrupt
;

#define INT(v)       static_cast<int>(v + 0.5)
#define ECHO_PIN     3      /* hardware interrupt for UNO, Nano */
#define TRIG_PIN     4
#define VO_PIN       5      /* PWM for contrast   of LCD panel  */
#define LCD_PIN      6      /* PWM for brightness of LCD panel  */
#define DHT11_PIN    7
#define RF_CE        9
#define RF_CSN       10
#define RF_ID        1      /* current RF device ID */
#define RF_RCV       0      /* target RF device ID  */

typedef struct {
    uint16_t n;
    uint16_t ht;
    uint16_t d;
} Pkt;
///
///> ISR to capture echo and calculate distance (in cm)
///
volatile int ds;
void echo_isr() {
    static volatile unsigned long t = 0;
    ds = (digitalRead(ECHO_PIN) == HIGH)
        ? (t=micros(), 0)
        : static_cast<uint16_t>((micros() - t) * 17 / 100);
}
///
///> trigger - to begin sending ultrasound pulses
///
void trig() {
    digitalWrite(TRIG_PIN, LOW);      // clear trigger pin
    delayMicroseconds(2);          
    digitalWrite(TRIG_PIN, HIGH);     // send 10us pulse 
    delayMicroseconds(10);    
    digitalWrite(TRIG_PIN, LOW);
}
///
///> fetch previous distance read, and trigger next read
///
void fetch_dist() {
     vm_push(ds);
     trig();                          // trigger next distance read
}

#include <dht.h>
dht      dht0;
volatile int ht;
///
///> fetch temperature and humidity
///
void fetch_dht() {
    if (dht0.read11(DHT11_PIN) == DHTLIB_OK) {
        ht = INT(dht0.humidity*100.0) + INT(dht0.temperature);
    }
    else ht = 9999;
    vm_push(ht);
}

#include <LiquidCrystal.h> // includes the LiquidCrystal Library 
LiquidCrystal lcd(14, 15, 16, 17, 18, 19); // (RS, EN, D4, D5, D6, D7)

void display(Pkt *p) {
    lcd.clear();
    lcd.print(p->n);
    if (p->d) {
        lcd.print(F("> Dist:"));
        lcd.print(p->d);
    }
    lcd.setCursor(0,1);
    lcd.print(F("Hum:"));
    lcd.print(p->ht / 100);
    lcd.setCursor(8,1);
    lcd.print(F("Temp:"));
    lcd.print(p->ht % 100);
}

#include <SPI.h>
#include <NRFLite.h>
NRFLite  rf;
void rf_send() {
    Pkt p0;
    p0.d  = vm_pop();
    p0.ht = vm_pop();
    p0.n  = vm_pop();
    
    display(&p0);                // on LCD for debugging
    rf.send(RF_RCV, &p0, sizeof(p0));
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);            // wait for serial port to connect
    
    for (int i = 4; i < 20; i++) pinMode(i, OUTPUT);
    pinMode(DHT11_PIN, INPUT);
    pinMode(ECHO_PIN, INPUT);
    
    lcd.begin(16,2);            // Initializes LCD
    analogWrite(LCD_PIN, 128);  // 1/2 brightness
    analogWrite(VO_PIN,  20);   // tune contrast
    
    attachInterrupt(digitalPinToInterrupt(ECHO_PIN), echo_isr, CHANGE);

    if (!rf.init(RF_ID, RF_CE, RF_CSN)) Serial.println("RF.init failed!");

    ef_setup(code);             // initialize VM (with embedded Forth)
    vm_cfunc(0, rf_send);       // register RF sender
    vm_cfunc(1, fetch_dht);     // register DHT function
    vm_cfunc(2, fetch_dist);    // register HC-SR04 function
}

void loop()
{
    ef_run();
}
