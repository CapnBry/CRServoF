## CRServoF - The CSRF serial protocol to PWM servo converter

I wanted to create a small project to mess around with PWM servo output for ExpressLRS, and thought this might be of use for other people.

[![YouTube Demo](https://img.youtube.com/vi/WrQQ0svOxig/hqdefault.jpg)](https://youtu.be/WrQQ0svOxig)

### What it does

If you have a receiver that outputs CRSF serial protocol (ExpressLRS, Crossfire, Tracer) but want to directly drive servos without a flight controller, I guess you're in the right place. That's exactly what this does. Hook up a CRSF RX to UART2 and your servos to various pins of an STM32F103C8 "blue pill" board and away you go. Not much to it other than that.

### Wiring and Flashing

| Pin | Description |
|----|----|
| 5V | 5V power input, 3.5V - 5.5V |
| GND | Ground, gotta have this |
| PA2 | TX (connect to CRSF RX) |
| PA3 | RX (connect to CRSF TX) |
| PA15 | Servo output CH1 - CRSF Channel 1 |
| PB3 | Servo output CH2 - CRSF Channel 2 |
| PB10 | Servo output CH3 - CRSF Channel 3 |
| PB11 | Servo output CH4 - CRSF Channel 4 |
| PA6 | Servo output CH5 - CRSF Channel 6 |
| PA7 | Servo output CH6 - CRSF Channel 7 |
| PB0 | Servo output CH7 - CRSF Channel 8 |
| PB1 | Servo output CH8 - CRSF Channel 12 |
| A0 | VBAT needs voltage divider, see below |

To flash, use platformio and an STLINK adapter, build either target but the F103_serial environment creates a USB serial port for debug logging when plugged in over USB.

### Channel Mapping

To change the channel mapping, use the `OUTPUT_MAP[]` array at the top. These are 1-based channels from the CRSF output, so 1 is usually Roll, 2 is Pitch and so on. 5 is AUX1 up to 12 is AUX8 for ExpressLRS, or up to 16 AUX12 for Crossfire models. The default map is `[ Roll, Pitch, Throttle, Yaw, AUX2, AUX3, AUX4, AUX12 ]` for my radio setup. To invert the channel output, +100% becomes -100%, just use a negative number for the channel (e.g. -12 for AUX8 inverted).

### Failsafe

The code has failsafe detection which happens if no channel packets are received for a short time (300ms currently). The default failsafe setting is to set CH1-4 to `1500, 1500, 988, 1500`, CH4-7 to hold their last position, and CH8 to stop putting out pulses. To change the failsafe behavior, modify the `OUTPUT_FAILSAFE[]` array with either the microseconds position to set on failsafe or `fsaNoPulses` (stop outputting PWM) or `fsaHold` (hold last received value).

### VBAT

The code sends a BATTERY telemetry item back to the CRSF RX, using A0 as the input value. **You can not plug VBAT directly in**. The maximum input voltage is 3.3V so the voltage needs to be scaled down. The code expects a resistor divider `VBAT -- 8.2kohm -A0- 1.2kohm -- GND` with VBAT on one end, GND on the other, and A0 connected in the middle. That should be good up to 6S voltage if I did my math right. The voltage can be calibrated using the `VBAT_SCALE` define in the top of main.cpp, and different resistors can be used by changing the `VBAT_R1` and `VBAT_R2` defines.

### ExpressLRS_via_BetaflightPassthrough

The serial UART will attempt to emulate a Betaflight CLI so ExpressLRS can flash the connected RX with yet another RC version. This works, I dunno, like 80% of the time? It is hard to get all the timing just right, but if it fails, you'l likely need to repower the whole device because the RX is in the bootloader and probably at the wrong autobaud.




