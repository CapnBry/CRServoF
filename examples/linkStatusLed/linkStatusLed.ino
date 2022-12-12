#include <ArduinoCRSF.h>
#include <HardwareSerial.h>

#define PIN_RX 7
#define PIN_TX 8

#define PIN_LED 36

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
  
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
}

void loop()
{
  // Must call crsf.update() in loop() to process data
  crsf.update();
  
  updateLinkStatusLed();
}

void updateLinkStatusLed()
{
  if(crsf.isLinkUp())
  {
    digitalWrite(PIN_LED, HIGH);
  }
  else
  {
    digitalWrite(PIN_LED, LOW);
    // Perform the failsafe action
  }
}