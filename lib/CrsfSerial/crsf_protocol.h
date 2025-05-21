#pragma once

#include <stdint.h>

#if !defined(PACKED)
#define PACKED __attribute__((packed))
#endif

#define CRSF_BAUDRATE           420000  // for receiverd only
#define CRSF_NUM_CHANNELS 16
#define CRSF_CHANNEL_VALUE_MIN  172 // 987us - actual CRSF min is 0 with E.Limits on
#define CRSF_CHANNEL_VALUE_1000 191
#define CRSF_CHANNEL_VALUE_MID  992
#define CRSF_CHANNEL_VALUE_2000 1792
#define CRSF_CHANNEL_VALUE_MAX  1811 // 2012us - actual CRSF max is 1984 with E.Limits on
#define CRSF_CHANNEL_VALUE_SPAN (CRSF_CHANNEL_VALUE_MAX - CRSF_CHANNEL_VALUE_MIN)
#define CRSF_ELIMIT_US_MIN         891   // microseconds for CRSF=0 (E.Limits=ON)
#define CRSF_ELIMIT_US_MAX         2119  // microseconds for CRSF=1984
#define CRSF_MAX_PACKET_SIZE 64 // max declared len is 62+DEST+LEN on top of that = 64
#define CRSF_MAX_PAYLOAD_LEN (CRSF_MAX_PACKET_SIZE - 4) // Max size of payload in [dest] [len] [type] [payload] [crc8]
#define CRSF_BITS_PER_CHANNEL   11

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

enum {
    CRSF_FRAME_GPS_PAYLOAD_SIZE = 15,
    CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE = 8,
    CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE = 10,
    CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE = 22, // 11 bits per channel * 16 channels = 22 bytes.
    CRSF_FRAME_ATTITUDE_PAYLOAD_SIZE = 6,
};

typedef enum
{
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_AIRSPEED = 0x0A,
    CRSF_FRAMETYPE_RPM = 0x0C,
    CRSF_FRAMETYPE_TEMP = 0x0D,
    CRSF_FRAMETYPE_CELLS = 0x0E,
    CRSF_FRAMETYPE_OPENTX_SYNC = 0x10,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    // Extended Header Frames, range: 0x28 to 0x96
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,
    CRSF_FRAMETYPE_COMMAND = 0x32,
    CRSF_FRAMETYPE_RADIO_ID = 0x3A,
    // MSP commands
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C, // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
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
    uint8_t sync_byte;   // CRSF_SYNC_BYTE
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

//CRSF_FRAMETYPE_BATTERY_SENSOR
typedef struct crsf_sensor_battery_s
{
    unsigned voltage : 16;  // mv * 100 BigEndian
    unsigned current : 16;  // ma * 100
    unsigned capacity : 24; // mah
    unsigned remaining : 8; // %
} PACKED crsf_sensor_battery_t;

// CRSF_FRAMETYPE_BARO_ALTITUDE
typedef struct crsf_sensor_baro_vario_s
{
    uint16_t altitude; // Altitude in decimeters + 10000dm, or Altitude in meters if high bit is set, BigEndian
    int16_t verticalspd;  // Vertical speed in cm/s, BigEndian
} PACKED crsf_sensor_baro_vario_t;

// CRSF_FRAMETYPE_AIRSPEED
typedef struct crsf_sensor_airspeed_s
{
    uint16_t speed;             // Airspeed in 0.1 * km/h (hectometers/h)
} PACKED crsf_sensor_airspeed_t;

// CRSF_FRAMETYPE_RPM
typedef struct crsf_sensor_rpm_s
{
    uint8_t source_id;          // Identifies the source of the RPM data (e.g., 0 = Motor 1, 1 = Motor 2, etc.)
    int32_t rpm0:24;            // 1 - 19 RPM values with negative ones representing the motor spinning in reverse
    int32_t rpm1:24;
    int32_t rpm2:24;
    int32_t rpm3:24;
    int32_t rpm4:24;
    int32_t rpm5:24;
    int32_t rpm6:24;
    int32_t rpm7:24;
    int32_t rpm8:24;
    int32_t rpm9:24;
    int32_t rpm10:24;
    int32_t rpm11:24;
    int32_t rpm12:24;
    int32_t rpm13:24;
    int32_t rpm14:24;
    int32_t rpm15:24;
    int32_t rpm16:24;
    int32_t rpm17:24;
    int32_t rpm18:24;
} PACKED crsf_sensor_rpm_t;

// CRSF_FRAMETYPE_TEMP
typedef struct crsf_sensor_temp_s
{
    uint8_t source_id;            // Identifies the source of the temperature data (e.g., 0 = FC including all ESCs, 1 = Ambient, etc.)
    int16_t temperature[20];      // up to 20 temperature values in deci-degree (tenths of a degree) Celsius (e.g., 250 = 25.0°C, -50 = -5.0°C)
} PACKED crsf_sensor_temp_t;

// CRSF_FRAMETYPE_CELLS
typedef struct crsf_sensor_cells_s
{
    uint8_t source_id;            // Identifies the source of the Main_battery data (e.g., 0 = battery 1, 1 = battery 2, etc.)
    uint16_t cell[29];            // up to 29 cell values in a resolution of a thousandth of a Volt (e.g. 3.850V = 3850)
} PACKED crsf_sensor_cells_t;

// CRSF_FRAMETYPE_VARIO
typedef struct crsf_sensor_vario_s
{
    int16_t verticalspd;  // Vertical speed in cm/s, BigEndian
} PACKED crsf_sensor_vario_t;

// CRSF_FRAMETYPE_GPS
typedef struct crsf_sensor_gps_s
{
    int32_t latitude; // degree / 10`000`000
    int32_t longitude; // degree / 10`000`000
    uint16_t groundspeed; // km/h / 10
    uint16_t heading; // degree / 100
    uint16_t altitude; // meter ­1000m offset
    uint8_t satellites; // counter
} PACKED crsf_sensor_gps_t;

// CRSF_FRAMETYPE_ATTITUDE
typedef struct crsf_sensor_attitude_s
{
    int16_t pitch; // radians * 10000
    int16_t roll; // radians * 10000
    int16_t yaw; // radians * 10000
} PACKED crsf_sensor_attitude_t;

// CRSF_FRAMETYPE_FLIGHT_MODE
typedef struct crsf_sensor_flight_mode_s
{
    char flight_mode[16];
} PACKED crsf_flight_mode_t;

typedef struct crsfPayloadLinkstatistics_s
{
    uint8_t uplink_RSSI_1;
    uint8_t uplink_RSSI_2;
    uint8_t uplink_Link_quality;
    int8_t uplink_SNR;
    uint8_t active_antenna;
    uint8_t rf_Mode;
    uint8_t uplink_TX_Power;
    uint8_t downlink_RSSI_1;
    uint8_t downlink_Link_quality;
    int8_t downlink_SNR;
} PACKED crsfLinkStatistics_t;

// crsf = (us - 1500) * 8/5 + 992
#define US_to_CRSF(us)      ((us) * 8 / 5 + (CRSF_CHANNEL_VALUE_MID - 2400))
// us = (crsf - 992) * 5/8 + 1500
#define CRSF_to_US(crsf)    ((crsf) * 5 / 8 + (1500 - 620))

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

static inline int htobe24(int val)
{
    return htobe32(val) >> 8;
}

#endif
