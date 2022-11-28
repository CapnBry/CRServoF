#include <ArduinoCRSF.h>

CrsfSerial crsf;

void setup()
{
    Serial.begin(115200);
    Serial.println("initialized");
    
    //Serial2 may not be supported on every MCU.
    //it could instead be any Stream type Serial object.
    Serial2.begin(CRSF_BAUDRATE);
    crsf.begin(Serial2);
    
    //crsf.onLinkUp = &crsfLinkUp;
    //crsf.onLinkDown = &crsfLinkDown;
    //crsf.onShiftyByte = &crsfShiftyByte;
    crsf.onPacketChannels = &packetChannels;
    //crsf.onPacketLinkStatistics = &packetLinkStatistics;
}

void loop()
{
    // Must call crsf.update() in loop() to process data
    crsf.update();
}

//This callback is called whenever new channel values are available.
//Use crsf.getChannel(x) to get us channel values (1-16).
void packetChannels()
{
  for (int ChannelNum = 1; ChannelNum <= 16; ChannelNum++){
    Serial.print(crsf.getChannel(ChannelNum));
    Serial.print(", ");
  }
  Serial.println(" ");
}