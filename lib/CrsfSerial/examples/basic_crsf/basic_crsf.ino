#include <CrsfSerial.h>

CrsfSerial crsf(Serial1, CRSF_BAUDRATE); // pass any HardwareSerial port

/***
 * This callback is called whenever new channel values are available.
 * Use crsf.getChannel(x) to get us channel values.
 ***/
void packetChannels()
{
    Serial.print("CH0=");
    Serial.println(crsf.getChannel(0));
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