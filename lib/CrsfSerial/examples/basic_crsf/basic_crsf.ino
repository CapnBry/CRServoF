#include <CrsfSerial.h>

// Pass any HardwareSerial port
// "Arduino" users (atmega328) can not use CRSF_BAUDRATE, as the atmega does not support it
// and should pass 250000, but then also must flash the receiver with RCVR_UART_BAUD=250000
// Also note the atmega only has one Serial, so logging to Serial must be removed
CrsfSerial crsf(Serial1, CRSF_BAUDRATE);

/***
 * This callback is called whenever new channel values are available.
 * Use crsf.getChannel(x) to get us channel values (1-16).
 ***/
void packetChannels()
{
    Serial.print("CH1=");
    Serial.println(crsf.getChannel(1));
}

void setup()
{
    Serial.begin(115200);

    // If something other than changing the baud of the UART needs to be done, do it here
    // Serial1.end(); Serial1.begin(500000, SERIAL_8N1, 16, 17);

    // Attach the channels callback
    crsf.onPacketChannels = &packetChannels;
}

void loop()
{
    // Must call CrsfSerial.loop() in loop() to process data
    crsf.loop();
}