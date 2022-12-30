#include <ArduinoCRSF.h>
#include <HardwareSerial.h>

#define PIN_RX 7
#define PIN_TX 8

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

void loop()
{
  // Must call crsf.update() in loop() to process data
  crsf.update();

  sendGps(42.12345, -82.12345, 200.5, 20.13, 690, 4);
  sendBaroAltitude(234.1, 154.1);
  sendAttitude(0.05,-2.43,1.23);
}

void sendGps(float latitude, float longitude, float groundspeed, float heading, float altitude, float satellites)
{
  crsf_sensor_gps_t crsfGps = { 0 };

  // Values are MSB first (BigEndian)
  crsfGps.latitude = htobe32((int32_t)(latitude*10000000.0));
  crsfGps.longitude = htobe32((int32_t)(longitude*10000000.0));
  crsfGps.groundspeed = htobe16((uint16_t)(groundspeed*10.0));
  crsfGps.heading = htobe16((int16_t)(heading*1000.0)); //TODO: heading seems to not display in EdgeTX correctly, some kind of overflow error
  crsfGps.altitude = htobe16((uint16_t)(altitude + 1000.0));
  crsfGps.satellites = (uint8_t)(satellites);
  crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_GPS, &crsfGps, sizeof(crsfGps));
}

void sendBaroAltitude(float altitude, float verticalspd)
{
  crsf_sensor_baro_altitude_t crsfBaroAltitude = { 0 };

  // Values are MSB first (BigEndian)
  crsfBaroAltitude.altitude = htobe16((uint16_t)(altitude*10.0 + 10000.0));
  //crsfBaroAltitude.verticalspd = htobe16((int16_t)(verticalspd*100.0)); //TODO: fix verticalspd in BaroAlt packets
  crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_BARO_ALTITUDE, &crsfBaroAltitude, sizeof(crsfBaroAltitude) - 2);
  
  //Supposedly vertical speed can be sent in a BaroAltitude packet, but I cant get this to work.
  //For now I have to send a second vario packet to get vertical speed telemetry to my TX.
  crsf_sensor_vario_t crsfVario = { 0 };

  // Values are MSB first (BigEndian)
  crsfVario.verticalspd = htobe16((int16_t)(verticalspd*100.0));
  crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_VARIO, &crsfVario, sizeof(crsfVario));
}

void sendAttitude(float pitch, float roll, float yaw)
{
  crsf_sensor_attitude_t crsfAttitude = { 0 };

  // Values are MSB first (BigEndian)
  crsfAttitude.pitch = htobe16((uint16_t)(pitch*10000.0));
  crsfAttitude.roll = htobe16((uint16_t)(roll*10000.0));
  crsfAttitude.yaw = htobe16((uint16_t)(yaw*10000.0));
  crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_ATTITUDE, &crsfAttitude, sizeof(crsfAttitude));
}
