#include <ArduinoCRSF.h>

ArduinoCRSF::ArduinoCRSF() :
    _crc(0xd5),
    _lastReceive(0), _lastChannelsPacket(0), _linkIsUp(false)
{
     
}

void ArduinoCRSF::begin(Stream &port)
{
  this->_port = &port;
}

// Call from main loop to update
void ArduinoCRSF::update()
{
    handleSerialIn();
}

void ArduinoCRSF::handleSerialIn()
{
    while (_port->available())
    {
        uint8_t b = _port->read();
        _lastReceive = millis();

        _rxBuf[_rxBufPos++] = b;
        handleByteReceived();

        if (_rxBufPos == (sizeof(_rxBuf)/sizeof(_rxBuf[0])))
        {
            // Packet buffer filled and no valid packet found, dump the whole thing
            _rxBufPos = 0;
        }
    }

    checkPacketTimeout();
    checkLinkDown();
}

void ArduinoCRSF::handleByteReceived()
{
    bool reprocess;
    do
    {
        reprocess = false;
        if (_rxBufPos > 1)
        {
            uint8_t len = _rxBuf[1];
            // Sanity check the declared length, can't be shorter than Type, X, CRC
            if (len < 3 || len > CRSF_MAX_PACKET_LEN)
            {
                shiftRxBuffer(1);
                reprocess = true;
            }

            else if (_rxBufPos >= (len + 2))
            {
                uint8_t inCrc = _rxBuf[2 + len - 1];
                uint8_t crc = _crc.calc(&_rxBuf[2], len - 1);
                if (crc == inCrc)
                {
                    processPacketIn(len);
                    shiftRxBuffer(len + 2);
                    reprocess = true;
                }
                else
                {
                    shiftRxBuffer(1);
                    reprocess = true;
                }
            }  // if complete packet
        } // if pos > 1
    } while (reprocess);
}

void ArduinoCRSF::checkPacketTimeout()
{
    // If we haven't received data in a long time, flush the buffer a byte at a time (to trigger shiftyByte)
    if (_rxBufPos > 0 && millis() - _lastReceive > CRSF_PACKET_TIMEOUT_MS)
        while (_rxBufPos)
            shiftRxBuffer(1);
}

void ArduinoCRSF::checkLinkDown()
{
    if (_linkIsUp && millis() - _lastChannelsPacket > CRSF_FAILSAFE_STAGE1_MS)
    {
        _linkIsUp = false;
    }
}

void ArduinoCRSF::processPacketIn(uint8_t len)
{
    const crsf_header_t *hdr = (crsf_header_t *)_rxBuf;
    if (hdr->device_addr == CRSF_ADDRESS_FLIGHT_CONTROLLER)
    {
        switch (hdr->type)
        {
        case CRSF_FRAMETYPE_GPS:
            packetGps(hdr);
            break;
        case CRSF_FRAMETYPE_RC_CHANNELS_PACKED:
            packetChannelsPacked(hdr);
            break;
        case CRSF_FRAMETYPE_LINK_STATISTICS:
            packetLinkStatistics(hdr);
            break;
        case CRSF_FRAMETYPE_BARO_ALTITUDE:
            packetBaroAltitude(hdr);
            break;
        case CRSF_FRAMETYPE_VARIO:
            packetVario(hdr);
            break;
        }
    }
}

// Shift the bytes in the RxBuf down by cnt bytes
void ArduinoCRSF::shiftRxBuffer(uint8_t cnt)
{
    // If removing the whole thing, just set pos to 0
    if (cnt >= _rxBufPos)
    {
        _rxBufPos = 0;
        return;
    }

    // Otherwise do the slow shift down
    uint8_t *src = &_rxBuf[cnt];
    uint8_t *dst = &_rxBuf[0];
    _rxBufPos -= cnt;
    uint8_t left = _rxBufPos;
    while (left--)
        *dst++ = *src++;
}

void ArduinoCRSF::packetChannelsPacked(const crsf_header_t *p)
{
    crsf_channels_t *ch = (crsf_channels_t *)&p->data;
    _channels[0] = ch->ch0;
    _channels[1] = ch->ch1;
    _channels[2] = ch->ch2;
    _channels[3] = ch->ch3;
    _channels[4] = ch->ch4;
    _channels[5] = ch->ch5;
    _channels[6] = ch->ch6;
    _channels[7] = ch->ch7;
    _channels[8] = ch->ch8;
    _channels[9] = ch->ch9;
    _channels[10] = ch->ch10;
    _channels[11] = ch->ch11;
    _channels[12] = ch->ch12;
    _channels[13] = ch->ch13;
    _channels[14] = ch->ch14;
    _channels[15] = ch->ch15;

    for (unsigned int i=0; i<CRSF_NUM_CHANNELS; ++i)
        _channels[i] = map(_channels[i], CRSF_CHANNEL_VALUE_1000, CRSF_CHANNEL_VALUE_2000, 1000, 2000);

    _linkIsUp = true;
    _lastChannelsPacket = millis();
}

void ArduinoCRSF::packetLinkStatistics(const crsf_header_t *p)
{
    const crsfLinkStatistics_t *link = (crsfLinkStatistics_t *)p->data;
    memcpy(&_linkStatistics, link, sizeof(_linkStatistics));
}

void ArduinoCRSF::packetGps(const crsf_header_t *p)
{
    const crsf_sensor_gps_t *gps = (crsf_sensor_gps_t *)p->data;
    _gpsSensor.latitude = be32toh(gps->latitude);
    _gpsSensor.longitude = be32toh(gps->longitude);
    _gpsSensor.groundspeed = be16toh(gps->groundspeed);
    _gpsSensor.heading = be16toh(gps->heading);
    _gpsSensor.altitude = be16toh(gps->altitude);
    _gpsSensor.satellites = gps->satellites;
}

void ArduinoCRSF::packetVario(const crsf_header_t *p)
{
    const crsf_sensor_vario_t *vario = (crsf_sensor_vario_t *)p->data;
    _varioSensor.verticalspd = be16toh(vario->verticalspd);
}

void ArduinoCRSF::packetBaroAltitude(const crsf_header_t *p)
{
    const crsf_sensor_baro_altitude_t *baroAltitude = (crsf_sensor_baro_altitude_t *)p->data;
    _baroAltitudeSensor.altitude = be16toh(baroAltitude->altitude);
    _baroAltitudeSensor.verticalspd = be16toh(baroAltitude->verticalspd);
}

void ArduinoCRSF::packetAttitude(const crsf_header_t *p)
{
    const crsf_sensor_attitude_t *attitude = (crsf_sensor_attitude_t *)p->data;
    _attitudeSensor.pitch = be16toh(attitude->pitch);
    _attitudeSensor.roll = be16toh(attitude->roll);
    _attitudeSensor.yaw = be16toh(attitude->yaw);
}

void ArduinoCRSF::write(uint8_t b)
{
    _port->write(b);
}

void ArduinoCRSF::write(const uint8_t *buf, size_t len)
{
    _port->write(buf, len);
}

void ArduinoCRSF::queuePacket(uint8_t addr, uint8_t type, const void *payload, uint8_t len)
{
    if (!_linkIsUp)
        return;
    if (len > CRSF_MAX_PACKET_LEN)
        return;
   
    uint8_t buf[CRSF_MAX_PACKET_LEN+4];
    buf[0] = addr;
    buf[1] = len + 2; // type + payload + crc
    buf[2] = type;
    memcpy(&buf[3], payload, len);
    buf[len+3] = _crc.calc(&buf[2], len + 1);
    
   for(int i = 0; i <= len + 3; i++)
   {
      Serial.print(String(buf[i], HEX));
      Serial.print(" ");
   }
   Serial.println(" ");
   
    // Busywait until the serial port seems free
    //while (millis() - _lastReceive < 2)
    //    loop();
    write(buf, len + 4);
}
