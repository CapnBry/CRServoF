#pragma once

#if defined(TARGET_BLUEPILL)
    #define DPIN_LED        LED_BUILTIN
    #define LED_INVERTED    1
    #define APIN_VBAT       A0
    #define USART_INPUT     USART2  // UART2 RX=PA3 TX=PA2
    #define OUTPUT_PIN_MAP  PA_15, PB_3, PB_10, PB_11, PA_6, PA_7, PB_0, PB_1 // TIM2 CH1-4, TIM3CH1-4

#elif defined(NEW_PLATFORM)

#endif

#if !defined(LED_INVERTED)
    #define LED_INVERTED    0
#endif

#if !defined(VBAT_R1) || !defined(VBAT_R2)
    // Resistor divider used on VBAT input, R1+R2 must be less than 3178
    #define VBAT_R1         820
    #define VBAT_R2         120
#endif