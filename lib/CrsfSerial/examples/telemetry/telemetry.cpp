/**
 * This example demonstrates using CrsfSerial to send telemetry data to a receiver
 * as would be done by a flight controller.
 * Note that the telemetry bandwidth back to the handset is extremely limited and
 * therefore queuing of items should be scheduled to prioritize items wanting more
 * frequent updates. The ExpressLRS queue is lossy and has one slot for each telemetry
 * type which will replace the value if updated before it can be sent.
 */
#include <CrsfSerial.h>

/**
 * Example uses an RP2040
 * [env:example_telem_pico]
 * platform = raspberrypi
 * board = pico
 * framework = arduino
 */

#define DPIN_CRSF_TX                p4
#define DPIN_CRSF_RX                p5

static UART CrsfSerialStream(DPIN_CRSF_TX, DPIN_CRSF_RX);
static CrsfSerial crsf(CrsfSerialStream, CRSF_BAUDRATE);

static void sendTemperatures()
{
    crsf_sensor_temp_t crsfTemp = { 0 };
    crsfTemp.source_id = 0; // Each group of temperature sensors should have its own ID
    crsfTemp.temperature[0] = htobe16(250); // 25.0C
    crsfTemp.temperature[1] = htobe16(-109); // -10.9C
    crsfTemp.temperature[2] = htobe16(1051); // 105.1C
    // up to 20x temperature sensors per source

    // When queueing, only send the size needed for the number of values filled
    constexpr size_t TEMPERATURE_COUNT = 3;
    crsf.queuePacket(CRSF_FRAMETYPE_TEMP, &crsfTemp, sizeof(crsfTemp.source_id) + TEMPERATURE_COUNT * sizeof(crsfTemp.temperature[0]));
}

static void sendRpms()
{
    crsf_sensor_rpm_t crsfRpm = { 0 };
    crsfRpm.source_id = 0; // Each group of RPM sensors should have its own ID
    crsfRpm.rpm0 = htobe24(18000);
    crsfRpm.rpm1 = htobe24(18001);
    crsfRpm.rpm2 = htobe24(18002);
    crsfRpm.rpm3 = htobe24(-18003); // negative indicates reverse RPM
    // up to 19x rpm sensors per source
    constexpr size_t RPM_SIZE = 3; // can't sizeof a bitfield

    // When queueing, only send the size needed for the number of values filled
    constexpr size_t RPM_COUNT = 4;
    crsf.queuePacket(CRSF_FRAMETYPE_RPM, &crsfRpm, sizeof(crsfRpm.source_id) + RPM_COUNT * RPM_SIZE);
}

static void sendCells()
{
    crsf_sensor_cells_t crsfCells = { 0 };
    crsfCells.source_id = 0; // Each battery pack should have its own ID
    crsfCells.cell[0] = htobe16(3500); // 3.500V
    crsfCells.cell[1] = htobe16(4350); // 4.350V
    crsfCells.cell[2] = htobe16(2900); // 2.900V
    crsfCells.cell[3] = htobe16(3141); // PiV :-D

    // When queueing, only send the size needed for the number of values filled
    constexpr size_t CELL_COUNT = 4;
    crsf.queuePacket(CRSF_FRAMETYPE_CELLS, &crsfCells, sizeof(crsfCells.source_id) + CELL_COUNT * sizeof(crsfCells.cell[0]));
}

static void sendVbat()
{
    crsf_sensor_battery_t crsfbatt = { 0 };
    crsfbatt.voltage = htobe16(123); // 12.3V
    crsfbatt.current = htobe16(196); // 19.6A
    crsfbatt.capacity = htobe24(1300); // 1300 mah consumed
    crsfbatt.remaining = 15; // 15% remaining

    crsf.queuePacket(CRSF_FRAMETYPE_BATTERY_SENSOR, &crsfbatt, sizeof(crsfbatt));
}

static void checkSendTelemetry()
{
    static uint32_t lastSend;
    uint32_t now = millis();
    constexpr uint32_t TELEM_INTERVAL_MS = 200U;
    if (now - lastSend < TELEM_INTERVAL_MS)
        return;
    lastSend = now;

    static enum eTlmItem { tlmVbat, tlmCells, tlmTemp, tlmRpms, TLM_COUNT } tlmItem;
    switch (tlmItem)
    {
        case tlmVbat: sendVbat(); break;
        case tlmCells: sendCells(); break;
        case tlmTemp: sendTemperatures(); break;
        case tlmRpms: sendRpms(); break;
        default: break; // COUNT
    }
    tlmItem = (eTlmItem)(((int)tlmItem + 1) % (int)TLM_COUNT);
}

void setup()
{
    Serial.begin(115200);

    crsf.begin();
}

void loop()
{
    // Must call CrsfSerial.loop() in loop() to process data
    crsf.loop();
    checkSendTelemetry();
}