/*
This example reads the voltage on pin PIN_SNS_VIN and sends it to your radio in a telemetry packet
This code assumes you are using a voltage divider with a "high side" resistance of 15K and "low side" resistance of 2.2K
*/

#include <ArduinoCRSF.h>
#include <HardwareSerial.h>

#define PIN_RX 7
#define PIN_TX 8

#define PIN_SNS_VIN 3

#define RESISTOR1 15000.0
#define RESISTOR2 2200.0
#define ADC_RES 8192.0
#define ADC_VLT 3.3

// Set up a new Serial object
HardwareSerial crsfSerial(1);
ArduinoCRSF crsf;

void setup()
{
  Serial.begin(115200);
  Serial.println("COM Serial initialized");
  
  crsfSerial.begin(CRSF_BAUDRATE, SERIAL_8N1, PIN_RX, PIN_TX);
  if (!crsfSerial) while (1) Serial.println("Invalid crsfSerial configuration");

  crsf.begin(crsfSerial);
}

float cap = 0;
void loop()
{
  // Must call crsf.update() in loop() to process data
  crsf.update();
  
  int snsVin = analogRead(PIN_SNS_VIN);
  float batteryVoltage = ((float)snsVin * ADC_VLT / ADC_RES) * ((RESISTOR1 + RESISTOR2) / RESISTOR2);
  sendRxBattery(batteryVoltage, 1.2, cap += 10, 50);
  delay(10);
}

static void sendRxBattery(float voltage, float current, float capacity, float remaining)
{
  crsf_sensor_battery_t crsfBatt = { 0 };

  // Values are MSB first (BigEndian)
  crsfBatt.voltage = htobe16((uint16_t)(voltage * 10.0));   //Volts
  crsfBatt.current = htobe16((uint16_t)(current * 10.0));   //Amps
  crsfBatt.capacity = htobe16((uint16_t)(capacity)) << 8;   //mAh (with this implemetation max capacity is 65535mAh)
  crsfBatt.remaining = (uint8_t)(remaining);                //percent
  crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_BATTERY_SENSOR, &crsfBatt, sizeof(crsfBatt));
}