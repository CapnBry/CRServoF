#include <Arduino.h>
#include <CrsfSerial.h>
#include <median.h>
#include "target.h"

#define NUM_OUTPUTS 8

// Configuration
// Map input CRSF channels (1-based, up to 16 for CRSF, 12 for ELRS) to outputs 1-8
// use a negative number to invert the signal (i.e. +100% becomes -100%)
constexpr int OUTPUT_MAP[NUM_OUTPUTS] = { 1, 2, 3, 4, 6, 7, 8, 12 };
// The failsafe action for each channel (fsaNoPulses, fsaHold, or microseconds)
constexpr int OUTPUT_FAILSAFE[NUM_OUTPUTS] = {
    1500, 1500, 988, 1500,                  // ch1-ch4
    fsaHold, fsaHold, fsaHold, fsaNoPulses  // ch5-ch8
    };
// Define the pins used to output servo PWM, must use hardware PWM,
// and change HardwareTimer targets below if the timers change
constexpr PinName OUTPUT_PINS[NUM_OUTPUTS] = { OUTPUT_PIN_MAP };

#define PWM_FREQ_HZ     50
#define VBAT_INTERVAL   500
#define VBAT_SMOOTH     5
// Scale used to calibrate or change to CRSF standard 0.1 scale
#define VBAT_SCALE      1.0

// Optimal safety and performance: Arm switch on AUX1 (channel 5)
// It is not recommended to change the channel
// See https://www.expresslrs.org/software/switch-config/
// Only used if USE_ARMSWITCH defined
#define ELRS_ARM_CHANNEL 5

// Local Variables
#if defined(ARDUINO_ARCH_STM32)
static HardwareSerial CrsfSerialStream(USART_INPUT);
#elif defined(TARGET_RASPBERRY_PI_PICO)
static UART CrsfSerialStream(DPIN_CRSF_TX, DPIN_CRSF_RX);
#endif
static CrsfSerial crsf(CrsfSerialStream);
static int g_OutputsUs[NUM_OUTPUTS];
#if defined(TARGET_RASPBERRY_PI_PICO)
#include <Servo.h>
static Servo *g_Servos[NUM_OUTPUTS];
#endif
static struct tagConnectionState {
    uint32_t lastVbatRead;
    MedianAvgFilter<unsigned int, VBAT_SMOOTH>vbatSmooth;
    unsigned int vbatValue;

    char serialInBuff[64];
    uint8_t serialInBuffLen;
    bool serialEcho;
} g_State;

static void crsfOobData(uint8_t b)
{
    // A shifty byte is usually just log messages from ELRS
    Serial.write(b);
}

/**
 * @brief: Initialize a servo pin output for the first time
*/
static void servoPlatformBegin(unsigned int servo)
{
#if defined(ARDUINO_ARCH_STM32)
    // vv pinMode(p, OUTPUT) vv
    pin_function(OUTPUT_PINS[servo], STM_PIN_DATA(STM_MODE_OUTPUT_PP, GPIO_NOPULL, 0));
#endif
#if defined(TARGET_RASPBERRY_PI_PICO)
    // Pi Pico waits for the value before attaching the servo
    Servo *s = new Servo();
    s->attach(OUTPUT_PINS[servo], CRSF_ELIMIT_US_MIN, CRSF_ELIMIT_US_MAX);
    g_Servos[servo] = s;
#endif
}

/**
 * @brief: Set an already initialized servo to a usec value
*/
static void servoPlatformSet(unsigned int servo, int usec)
{
#if defined(ARDUINO_ARCH_STM32)
    pwm_start(OUTPUT_PINS[servo], PWM_FREQ_HZ, usec, MICROSEC_COMPARE_FORMAT);
#endif
#if defined(TARGET_RASPBERRY_PI_PICO)
    if (g_Servos[servo] == nullptr)
    {
        Servo *s = new Servo();
        s->attach(OUTPUT_PINS[servo], CRSF_ELIMIT_US_MIN, CRSF_ELIMIT_US_MAX, usec);
        g_Servos[servo] = s;
    }
    else
        g_Servos[servo]->writeMicroseconds(usec);
#endif
}

static void servoPlatformEnd(unsigned int servo)
{
#if defined(PLATFORM_STM32)
    pwm_stop(OUTPUT_PINS[servo]);
    // vv pinMode(p, INPUT_PULLDOWN) vv
    pin_function(OUTPUT_PINS[servo], STM_PIN_DATA(STM_MODE_INPUT, GPIO_PULLDOWN, 0));
#endif
#if defined(TARGET_RASPBERRY_PI_PICO)
     Servo *s = g_Servos[servo];
     g_Servos[servo] = nullptr;
     s->detach();
     delete s;
#endif
}

static void servoSetUs(unsigned int servo, int usec)
{
    if (usec > 0)
    {
        // 0 means it was disabled previously, enable OUTPUT mode
        if (g_OutputsUs[servo] == 0)
            servoPlatformBegin(servo);
        servoPlatformSet(servo, usec);
    }
    else
    {
        servoPlatformEnd(servo);
    }
    g_OutputsUs[servo] = usec;
}


static void outputFailsafeValues()
 {
    for (unsigned int out=0; out<NUM_OUTPUTS; ++out)
    {
        if (OUTPUT_FAILSAFE[out] == fsaNoPulses)
            servoSetUs(out, 0);
        else if (OUTPUT_FAILSAFE[out] != fsaHold)
            servoSetUs(out, OUTPUT_FAILSAFE[out]);
        // else fsaHold does nothing, keep the same value
    }
}


#if defined(USE_ARMSWITCH)
// If USE_ARMSWITCH flag is given during compilation, isArmed
// checks if the arm signal was sent on channel defined by CRSF_ELRS_ARM_CHANNEL.
// The arm signal has to be *over* 1500us
static bool isArmed()
{
    // Static variable to store arm count, initialized to 0
    static uint8_t armCount = 0;

    if (crsf.getChannel(ELRS_ARM_CHANNEL) <= 1500)
    {
        armCount = 0;
        return false;
    }
    // Require at least 4 packets with "arm" signal, in order to
    // prevent accidental arming due to corrupt signals, similar
    // to Betaflight
    if (armCount < 4)
    {
        armCount++;
        return false;
    }
    return true;
}
#endif

static void packetChannels()
{
#if defined(USE_ARMSWITCH)
    if (!isArmed())
    {
        outputFailsafeValues();
        return;
    }
#endif

    for (unsigned int out=0; out<NUM_OUTPUTS; ++out)
    {
        const int chInput = OUTPUT_MAP[out];
        int usOutput;
        if (chInput > 0)
            usOutput = crsf.getChannel(chInput);
        else
        {
            // if chInput is negative, invert the channel output
            usOutput = crsf.getChannel(-chInput);
            // (1500 - usOutput) + 1500
            usOutput = 3000 - usOutput;
        }
        servoSetUs(out, usOutput);
    }

    // for (unsigned int ch=1; ch<=4; ++ch)
    // {
    //     Serial.write(ch < 10 ? '0' + ch : 'A' + ch - 10);
    //     Serial.write('=');
    //     Serial.print(crsf.getChannel(ch), DEC);
    //     Serial.write(' ');
    // }
    // Serial.println();
}

static void packetLinkStatistics(crsfLinkStatistics_t *link)
{
  //Serial.print(link->uplink_RSSI_1, DEC);
  //Serial.println("dBm");
}

static void crsfLinkUp()
{
    digitalWrite(DPIN_LED, HIGH ^ LED_INVERTED);
}

static void crsfLinkDown()
{
    digitalWrite(DPIN_LED, LOW ^ LED_INVERTED);
    outputFailsafeValues();
 }

static void checkVbatt()
{
#if defined(APIN_VBAT)
    if (millis() - g_State.lastVbatRead < (VBAT_INTERVAL / VBAT_SMOOTH))
        return;
    g_State.lastVbatRead = millis();

    unsigned int idx = g_State.vbatSmooth.add(analogRead(APIN_VBAT));
    if (idx != 0)
        return;

    unsigned int adc = g_State.vbatSmooth;
    g_State.vbatValue = 330U * adc * (VBAT_R1 + VBAT_R2) / VBAT_R2 / ((1 << 12) - 1);

    crsf_sensor_battery_t crsfbatt = { 0 };
    uint16_t scaledVoltage = g_State.vbatValue * VBAT_SCALE;
    // Values are MSB first (BigEndian)
    crsfbatt.voltage = htobe16(scaledVoltage);
    crsf.queuePacket(CRSF_SYNC_BYTE, CRSF_FRAMETYPE_BATTERY_SENSOR, &crsfbatt, sizeof(crsfbatt));

    //Serial.print("ADC="); Serial.print(adc, DEC);
    //Serial.print(" "); Serial.print(g_State.vbatValue, DEC); Serial.println("V");
#endif // APIN_VBAT
}

static void passthroughBegin(uint32_t baud)
{
    if (baud != crsf.getBaud())
    {
        // Force a reboot command since we want to send the reboot
        // at 420000 then switch to what the user wanted
        const uint8_t rebootpayload[] = { 'b', 'l' };
        crsf.queuePacket(CRSF_ADDRESS_CRSF_RECEIVER, CRSF_FRAMETYPE_COMMAND, &rebootpayload, sizeof(rebootpayload));
    }

    crsf.setPassthroughMode(true, baud);
    g_State.serialEcho = false;
}

/***
 * @brief Processes a text command like we're some sort of CLI or something
 * @return true if CrsfSerial was placed into passthrough mode, false otherwise
*/
static bool handleSerialCommand(char *cmd)
{
    // Fake a CRSF RX on UART6
    bool prompt = true;
    if (strcmp(cmd, "#") == 0)
    {
        Serial.println("Fake CLI Mode, type 'exit' or 'help' to do nothing\r\n");
        g_State.serialEcho = true;
    }

    else if (strcmp(cmd, "serial") == 0)
        Serial.println("serial 5 64 0 0 0 0\r\n");

    else if (strcmp(cmd, "get serialrx_provider") == 0)
        Serial.println("serialrx_provider = CRSF\r\n");

    else if (strcmp(cmd, "get serialrx_inverted") == 0)
        Serial.println("serialrx_inverted = OFF\r\n");

    else if (strcmp(cmd, "get serialrx_halfduplex") == 0)
        Serial.println("serialrx_halfduplex = OFF\r\n");

    else if (strncmp(cmd, "serialpassthrough 5 ", 20) == 0)
    {
        // Just echo the command back, BF and iNav both send
        // difference blocks of text as they start passthrough
        Serial.println(cmd);

        unsigned int baud = atoi(&cmd[20]);
        passthroughBegin(baud);

        return true;
    }

    else
        prompt = false;

    if (prompt)
        Serial.print("# ");

    return false;
}

static void checkSerialInPassthrough()
{
    static uint32_t lastData = 0;
    static bool LED = false;
    bool gotData = false;

    // Simple data passthrough from in to crsf
    unsigned int avail;
    while ((avail = Serial.available()) != 0)
    {
        uint8_t buf[16];
        avail = Serial.readBytes((char *)buf, min(sizeof(buf), avail));
        crsf.write(buf, avail);
        digitalWrite(DPIN_LED, LED);
        LED = !LED;
        gotData = true;
    }

    // If longer than X seconds since last data, switch out of passthrough
    if (gotData || !lastData)
        lastData = millis();
    // Turn off LED 1s after last data
    else if (LED && (millis() - lastData > 1000))
    {
        LED = false;
        digitalWrite(DPIN_LED, LOW ^ LED_INVERTED);
    }
    // Short blink LED after timeout
    else if (millis() - lastData > 5000)
    {
        lastData = 0;
        digitalWrite(DPIN_LED, HIGH ^ LED_INVERTED);
        delay(200);
        digitalWrite(DPIN_LED, LOW ^ LED_INVERTED);
        crsf.setPassthroughMode(false);
    }
}

static void checkSerialInNormal()
{
    while (Serial.available())
    {
        char c = Serial.read();
        if (g_State.serialEcho && c != '\n')
            Serial.write(c);

        if (c == '\r' || c == '\n')
        {
            if (g_State.serialInBuffLen != 0)
            {
                Serial.write('\n');

                g_State.serialInBuff[g_State.serialInBuffLen] = '\0';
                g_State.serialInBuffLen = 0;

                bool goToPassthrough = handleSerialCommand(g_State.serialInBuff);
                // If need to go to passthrough, get outta here before we dequeue any bytes
                if (goToPassthrough)
                    return;
            }
        }
        else
        {
            g_State.serialInBuff[g_State.serialInBuffLen++] = c;
            // if the buffer fills without getting a newline, just reset
            if (g_State.serialInBuffLen >= sizeof(g_State.serialInBuff))
                g_State.serialInBuffLen = 0;
        }
    }  /* while Serial */
}

static void checkSerialIn()
{
    if (crsf.getPassthroughMode())
        checkSerialInPassthrough();
    else
        checkSerialInNormal();
}

static void setupCrsf()
{
    crsf.onLinkUp = &crsfLinkUp;
    crsf.onLinkDown = &crsfLinkDown;
    crsf.onOobData = &crsfOobData;
    crsf.onPacketChannels = &packetChannels;
    crsf.onPacketLinkStatistics = &packetLinkStatistics;
    crsf.begin();
}

static void setupGpio()
{
    pinMode(DPIN_LED, OUTPUT);
    digitalWrite(DPIN_LED, LOW ^ LED_INVERTED);
    analogReadResolution(12);

    // The servo outputs are initialized when the
    // first channels packet comes in and sets the PWM
    // output value, to prevent them from jerking around
    // on startup
}

void setup()
{
    Serial.begin(115200);

    setupGpio();
    setupCrsf();
}

void loop()
{
    crsf.loop();
    checkVbatt();
    checkSerialIn();
}
