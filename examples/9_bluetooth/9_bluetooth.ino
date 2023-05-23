///
/// @file
/// @brief - Bluetooth connection
/// Note: Serial Terminal setup
/*
   Linux or Pi:
     + Install Bluez
     + Install a serial terminal (i.e. Minicom, picocom, or just emacs serial-term for me)
     + Setup Bluetooth connection (i.e. rfcomm)
       bluetoothd -C &
       hciconfig hci0 up
       hciconfig hci0 piscan
       sdptool add SP
       bluetoothctl scan on
       bluetoothctl agent on
       bluetoothctl default-agent
       bluetoothctl trust xx:xx:xx:xx:CF:F0
       bluetoothctl pair xx:xx:xx:xx:CF:F0
       rfcomm bind 0 xx:xx:xx:xx:CF:F0 1
       chmod 666 /dev/rfcomm0
       rfcomm connect 0 xx:xx:xx:xx:CF:F0 1
     + Open your serial terminal
       set parameters to 9600 8N1
       set Add Carriage Return (emacs serial-term needs to patch term.el)
       optionally, set Line mode for efficiency
       
   Windows:
     + Start->Setting->Bluetooth & devices to turn on Bluetooth
     + Connect Bluetooth: In taskbar, select Network, Sound or Battery->Manage Bluetooth devices 
     + Open your serial terminal (i.g. TeraTerm, PuTTY)
       set parameters to 9600 8N1
       set Add Carriage Return
       optionally, set Line mode for efficiency

   Mac:
     I have no idea. Let me know if you got it figured out!
*/     
#include <src/eForth1.h>
#include <AltSoftSerial.h>

AltSoftSerial bt;                         /// * default pin 8, 9

PROGMEM const char code[] = "\n";
void setup() {
    Serial.begin(115200);
    while (!Serial);                      // wait for serial port to connect

    bt.begin(9600);                       // establish Bluetooth connection
    delay(1000);
    
    Serial.println(F("Connecting Bluetooth..."));
    ef_setup(code, bt);                   // passing code and Bluetooth stream to Forth VM
}

void loop() {
    ef_run();
}

