#pragma once

#include <stdint.h>

#define PACKED __attribute__((packed))

#define CRSF_BAUDRATE           420000
#define CRSF_NUM_CHANNELS 16
#define CRSF_CHANNEL_VALUE_MIN  172
#define CRSF_CHANNEL_VALUE_1000 191
#define CRSF_CHANNEL_VALUE_MID  992
#define CRSF_CHANNEL_VALUE_2000 1792
#define CRSF_CHANNEL_VALUE_MAX  1811
#define CRSF_CHANNEL_VALUE_SPAN (CRSF_CHANNEL_VALUE_MAX - CRSF_CHANNEL_VALUE_MIN)
#define CRSF_MAX_PACKET_LEN 64

// Clashes with CRSF_ADDRESS_FLIGHT_CONTROLLER
#define CRSF_SYNC_BYTE 0XC8

enum {
    CRSF_FRAME_LENGTH_ADDRESS = 1, // length of ADDRESS field
    CRSF_FRAME_LENGTH_FRAMELENGTH = 1, // length of FRAMELENGTH field
    CRSF_FRAME_LENGTH_TYPE = 1, // length of TYPE field
    CRSF_FRAME_LENGTH_CRC = 1, // length of CRC field
    CRSF_FRAME_LENGTH_TYPE_CRC = 2, // length of TYPE and CRC fields combined
    CRSF_FRAME_LENGTH_EXT_TYPE_CRC = 4, // length of Extended Dest/Origin, TYPE and CRC fields combined
    CRSF_FRAME_LENGTH_NON_PAYLOAD = 4, // combined length of all fields except payload
};

typedef enum
{
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_VARIO = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_BARO_ALTITUDE = 0x09,
    //CRSF_FRAMETYPE_HEARTBEAT = 0x0B,                   //no need to support? (rev07)
    //CRSF_FRAMETYPE_VIDEO_TRANSMITTER = 0x0F,           //no need to support? (rev07)
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    // CRSF_FRAMETYPE_OPENTX_SYNC = 0x10,               //not in edgeTX
    // CRSF_FRAMETYPE_RADIO_ID = 0x3A,                  //no need to support?
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    // CRSF_FRAMETYPE_LINK_RX_ID = 0x1C,                 //no need to support?
    // CRSF_FRAMETYPE_LINK_TX_ID = 0x1D,                 //no need to support?
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    // CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,               //no need to support?
  // Extended Header Frames, range: 0x28 to 0x96
    // CRSF_FRAMETYPE_DEVICE_PING = 0x28,               //no "flight controller" needs to know about this
    // CRSF_FRAMETYPE_DEVICE_INFO = 0x29,               //no "flight controller" needs to know about this
    // CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,  //no "flight controller" needs to know about this
    // CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,            //no "flight controller" needs to know about this
    // CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,           //no "flight controller" needs to know about this
    // CRSF_FRAMETYPE_COMMAND = 0x32,                   //no "flight controller" needs to know about this
  // KISS frames
    // CRSF_FRAMETYPE_KISS_REQ  = 0x78,                 //not in edgeTX
    // CRSF_FRAMETYPE_KISS_RESP = 0x79,                 //not in edgeTX
  // MSP commands
    // CRSF_FRAMETYPE_MSP_REQ = 0x7A,                   //not in edgeTX
    // CRSF_FRAMETYPE_MSP_RESP = 0x7B,                  //not in edgeTX
    // CRSF_FRAMETYPE_MSP_WRITE = 0x7C,                 //not in edgeTX
  // Ardupilot frames
    // CRSF_FRAMETYPE_ARDUPILOT_RESP = 0x80,
} crsf_frame_type_e;

typedef enum
{
    CRSF_ADDRESS_BROADCAST = 0x00,
    CRSF_ADDRESS_USB = 0x10,
    CRSF_ADDRESS_TBS_CORE_PNP_PRO = 0x80,
    CRSF_ADDRESS_RESERVED1 = 0x8A,
    CRSF_ADDRESS_CURRENT_SENSOR = 0xC0,
    CRSF_ADDRESS_GPS = 0xC2,
    CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
    CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
    CRSF_ADDRESS_RESERVED2 = 0xCA,
    CRSF_ADDRESS_RACE_TAG = 0xCC,
    CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
    CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
    CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE,
} crsf_addr_e;

typedef struct crsf_header_s
{
    uint8_t device_addr; // from crsf_addr_e
    uint8_t frame_size;  // counts size after this byte, so it must be the payload size + 2 (type and crc)
    uint8_t type;        // from crsf_frame_type_e
    uint8_t data[0];
} PACKED crsf_header_t;

typedef struct crsf_channels_s
{
    unsigned ch0 : 11;
    unsigned ch1 : 11;
    unsigned ch2 : 11;
    unsigned ch3 : 11;
    unsigned ch4 : 11;
    unsigned ch5 : 11;
    unsigned ch6 : 11;
    unsigned ch7 : 11;
    unsigned ch8 : 11;
    unsigned ch9 : 11;
    unsigned ch10 : 11;
    unsigned ch11 : 11;
    unsigned ch12 : 11;
    unsigned ch13 : 11;
    unsigned ch14 : 11;
    unsigned ch15 : 11;
} PACKED crsf_channels_t;

typedef struct crsfPayloadLinkstatistics_s
{
    uint8_t uplink_RSSI_1;
    uint8_t uplink_RSSI_2;
    uint8_t uplink_Link_quality;
    int8_t uplink_SNR;
    uint8_t active_antenna;
    uint8_t rf_Mode;
    uint8_t uplink_TX_Power;
    uint8_t downlink_RSSI;
    uint8_t downlink_Link_quality;
    int8_t downlink_SNR;
} crsfLinkStatistics_t;

typedef struct crsf_sensor_battery_s
{
    unsigned voltage : 16;  // V * 10 big endian
    unsigned current : 16;  // A * 10 big endian
    unsigned capacity : 24; // mah big endian
    unsigned remaining : 8; // %
} PACKED crsf_sensor_battery_t;

typedef struct crsf_sensor_gps_s
{
    int32_t latitude;   // degree / 10,000,000 big endian
    int32_t longitude;  // degree / 10,000,000 big endian
    uint16_t groundspeed;  // km/h / 10 big endian
    uint16_t heading;   // GPS heading, degree/100 big endian
    uint16_t altitude;  // meters, +1000m big endian
    uint8_t satellites; // satellites
} PACKED crsf_sensor_gps_t;

typedef struct crsf_sensor_vario_s
{
    int16_t verticalspd; // Vertical speed in cm/s, BigEndian
} PACKED crsf_sensor_vario_t;

typedef struct crsf_sensor_baro_altitude_s
{
    uint16_t altitude; // Altitude in decimeters + 10000dm, or Altitude in meters if high bit is set, BigEndian
    int16_t verticalspd;  // Vertical speed in cm/s, BigEndian
} PACKED crsf_sensor_baro_altitude_t;


typedef struct crsf_sensor_attitude_s
{
    uint16_t pitch;  // pitch in radians, BigEndian
    uint16_t roll;  // roll in radians, BigEndian
    uint16_t yaw;  // yaw in radians, BigEndian
} PACKED crsf_sensor_attitude_t;

#if !defined(__linux__)
static inline uint16_t htobe16(uint16_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap16(val);
#endif
}

static inline uint16_t be16toh(uint16_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap16(val);
#endif
}

static inline uint32_t htobe32(uint32_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap32(val);
#endif
}

static inline uint32_t be32toh(uint32_t val)
{
#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    return val;
#else
    return __builtin_bswap32(val);
#endif
}
#endif
