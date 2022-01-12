#pragma once

#include <Arduino.h>
#include <functional>
#include <crc8.h>
#include "crsf_protocol.h"

enum eFailsafeAction { fsaNoPulses, fsaHold };

class CrsfSerial
{
public:
    // Packet timeout where buffer is flushed if no data is received in this time
    static const unsigned int CRSF_PACKET_TIMEOUT_MS = 100;
    static const unsigned int CRSF_FAILSAFE_STAGE1_MS = 300;

    CrsfSerial(HardwareSerial &port, uint32_t baud = CRSF_BAUDRATE);
    void loop();
    void write(uint8_t b);
    void write(const uint8_t *buf, size_t len);
    void queuePacket(uint8_t addr, uint8_t type, const void *payload, uint8_t len);

    // Return current channel value (1-based) in us
    int getChannel(unsigned int ch) const { return _channels[ch - 1]; }
    const crsfLinkStatistics_t *getLinkStatistics() const { return &_linkStatistics; }
    bool isLinkUp() const { return _linkIsUp; }
    bool getPassthroughMode() const { return _passthroughMode; }
    void setPassthroughMode(bool val, unsigned int baud = 0);

    // Event Handlers
    std::function<void()> onLinkUp;
    std::function<void()> onLinkDown;
    std::function<void(uint8_t)> onShiftyByte;
    std::function<void()> onPacketChannels;
    std::function<void(crsfLinkStatistics_t *)> onPacketLinkStatistics;

private:
    HardwareSerial &_port;
    uint8_t _rxBuf[CRSF_MAX_PACKET_LEN+3];
    uint8_t _rxBufPos;
    Crc8 _crc;
    crsfLinkStatistics_t _linkStatistics;
    uint32_t _lastReceive;
    uint32_t _lastChannelsPacket;
    bool _linkIsUp;
    bool _passthroughMode;
    int _channels[CRSF_NUM_CHANNELS];

    void handleSerialIn();
    void handleByteReceived();
    void shiftRxBuffer(uint8_t cnt);
    void processPacketIn(uint8_t len);
    void checkPacketTimeout();
    void checkLinkDown();

    // Packet Handlers
    void packetChannelsPacked(const crsf_header_t *p);
    void packetLinkStatistics(const crsf_header_t *p);
};