/*
This example reads the voltage on pin PIN_SNS_VIN and sends it to your radio in a telemetry packet
This code assumes you are using a voltage divider with a "high side" resistance of 15K and "low side" resistance of 2.2K
*/


#include <ArduinoCRSF.h>
#include <HardwareSerial.h>

#define PIN_LED 36
#define PIN_RX 7
#define PIN_TX 8
#define PIN_SNS_VIN 3

#define RESISTOR1 15000.0
#define RESISTOR2 2200.0
#define ADC_RES 8192.0
#define ADC_VLT 3.3

// Set up a new Serial object
HardwareSerial mySerial(1);
CrsfSerial crsf;

void setup() {
  Serial.begin(115200);
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  mySerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
  if (!mySerial)
    while (1) Serial.println("Invalid configuration, check config");
  crsf.begin(mySerial);

  crsf.onLinkUp = &crsfLinkUp;
  crsf.onLinkDown = &crsfLinkDown;
  crsf.onPacketChannels = &packetChannels;
  crsf.onPacketLinkStatistics = &packetLinkStatistics;
}

void loop() {
  crsf.update();
  
  int snsVin = analogRead(PIN_SNS_VIN);
  float batteryVoltage = ((float)snsVin * ADC_VLT / ADC_RES) * ((RESISTOR1 + RESISTOR2) / RESISTOR2);
  sendRxBt(batteryVoltage);
}

static void sendRxBt(float batteryVoltage) {
  crsf_sensor_battery_t crsfbatt = { 0 };

  // Values are MSB first (BigEndian)
  crsfbatt.voltage = htobe16((uint16_t)(batteryVoltage * 10));
  crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_BATTERY_SENSOR, &crsfbatt, sizeof(crsfbatt));
}

//-----------------------------------CRSF-Callbacks----------------------------------------------//
void packetChannels() {
  // for (int ChannelNum = 1; ChannelNum <= 16; ChannelNum++) {
  //   Serial.print(crsf.getChannel(ChannelNum));
  //   Serial.print(", ");
  // }
  // Serial.println(" ");
}

static void crsfLinkUp() {
  digitalWrite(PIN_LED, HIGH);
}

static void crsfLinkDown() {
  digitalWrite(PIN_LED, LOW);
  // Perform the failsafe action
}

static void packetLinkStatistics(crsfLinkStatistics_t *link) {
  //Serial.print(link->uplink_RSSI_1, DEC);
  //Serial.println("dBm");
}