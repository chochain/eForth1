#include <time.h>
#include <pt.h>
#include "eforth1.h"

#define PT_DELAY_msec(th, ms)  do {     \
    static unsigned long t;             \
    t = millis() + (unsigned long)(ms); \
    PT_WAIT_UNTIL((th), millis()>=t);   \
} while(0)

static struct pt ctx_hw;                    // protothread contexts
PT_THREAD(hw_task())                        // hardward protothread
{
    PT_BEGIN(&ctx_hw);
    while (1) {
        digitalWrite(LED_BUILTIN, HIGH);
        PT_DELAY_msec(&ctx_hw, 500);
        digitalWrite(LED_BUILTIN, LOW);
        PT_DELAY_msec(&ctx_hw, 500);
    }
    PT_END(&ctx_hw);
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);          // wait for serial port to connect

    ef_setup();
    ef_add_task(hw_task);
    
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    ef_run();
}


