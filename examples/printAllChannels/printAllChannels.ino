
#include <ArduinoCRSF.h>

CrsfSerial crsf(Serial2, CRSF_BAUDRATE);

//This callback is called whenever new channel values are available.
//Use crsf.getChannel(x) to get us channel values (1-16).
void packetChannels()
{
  for (int i = 1; i <= 16; i++){
    Serial.print(crsf.getChannel(ChNum));
    Serial.print(", ");
  }
  Serial.println(" ");
}

void setup()
{
    Serial.begin(115200);
    Serial.println("initialized");

    //crsf.onLinkUp = &crsfLinkUp;
    //crsf.onLinkDown = &crsfLinkDown;
    //crsf.onShiftyByte = &crsfShiftyByte;
    crsf.onPacketChannels = &packetChannels;
    //crsf.onPacketLinkStatistics = &packetLinkStatistics;
}

void loop()
{
    // Must call CrsfSerial.loop() in loop() to process data
    crsf.loop();
}
