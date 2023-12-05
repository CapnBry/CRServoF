## CRServoF - The CSRF serial protocol to PWM servo converter

I wanted to create a small project to mess around with PWM servo output for ExpressLRS, and thought this might be of use for other people.

[![YouTube Demo](https://img.youtube.com/vi/WrQQ0svOxig/hqdefault.jpg)](https://youtu.be/WrQQ0svOxig)

### What it does

If you have a receiver that outputs CRSF serial protocol (ExpressLRS, Crossfire, Tracer) but want to directly drive servos without a flight controller, I guess you're in the right place. That's exactly what this does. Hook up a CRSF RX to UART2 and your servos to various pins of an STM32F103C8 "blue pill" board and away you go. Not much to it other than that.

### Wiring and Flashing

See the wiki [Flashing and Wiring](https://github.com/CapnBry/CRServoF/wiki/Wiring)

### Channel Mapping

To change the channel mapping, use the `OUTPUT_MAP[]` array at the top. These are 1-based channels from the CRSF output, so 1 is usually Roll, 2 is Pitch and so on. 5 is AUX1 up to 12 is AUX8 for ExpressLRS, or up to 16 AUX12 for Crossfire models. The default map is `[ Roll, Pitch, Throttle, Yaw, AUX2, AUX3, AUX4, AUX12 ]` for my radio setup. To invert the channel output, +100% becomes -100%, just use a negative number for the channel (e.g. -12 for AUX8 inverted).

### Failsafe

The code has failsafe detection which happens if no channel packets are received for a short time (300ms currently). The default failsafe setting is to set CH1-4 to `1500, 1500, 988, 1500`, CH4-7 to hold their last position, and CH8 to stop putting out pulses. To change the failsafe behavior, modify the `OUTPUT_FAILSAFE[]` array with either the microseconds position to set on failsafe or `fsaNoPulses` (stop outputting PWM) or `fsaHold` (hold last received value).

### Arming / Disarming

CRServoF includes an optional feature to require an arming signal for other channels to be processed. To use this feature, include the buildflag `USE_ARMSWITCH`. CRServoF expects a "high" value (>1500us) on CH5 to arm. If disarmed, the failsafe values mentioned above will be sent, make sure that you use the correct values applicable to your use case.

### VBAT

The code sends a BATTERY telemetry item back to the CRSF RX, using A0 as the input value. **You can not plug VBAT directly in**. The maximum input voltage is 3.3V so the voltage needs to be scaled down. The code expects a resistor divider `VBAT -- 8.2kohm -A0- 1.2kohm -- GND` with VBAT on one end, GND on the other, and A0 connected in the middle. That should be good up to 6S voltage if I did my math right. The voltage can be calibrated using the `VBAT_SCALE` define in the top of main.cpp, and different resistors can be used by changing the `VBAT_R1` and `VBAT_R2` defines.

### ExpressLRS_via_BetaflightPassthrough

The serial UART will attempt to emulate a Betaflight CLI so ExpressLRS can flash the connected RX with yet another RC version. This works, I dunno, like 80% of the time? It is hard to get all the timing just right, but if it fails, you will likely need to repower the whole device because the RX is in the bootloader and probably at the wrong autobaud.




