#pragma once

#if defined(TARGET_BLUEPILL)
    #define DPIN_LED        LED_BUILTIN
    #define LED_INVERTED    1
    #define APIN_VBAT       A0
    #define USART_INPUT     USART2  // UART2 RX=PA3 TX=PA2
    #define OUTPUT_PIN_MAP  PA_15, PB_3, PB_10, PB_11, PA_6, PA_7, PB_0, PB_1 // TIM2 CH1-4, TIM3CH1-4

#elif defined(TARGET_CC3D)
    #define DPIN_LED        PB_3
    #define LED_INVERTED    1
    #define APIN_VBAT       PA_14
    #define USART_INPUT     USART3  // UART3 RX=PA11 TX=PA10 -CC3D Flexi port
    #define OUTPUT_PIN_MAP  PB_9, PB_8, PB_7, PA_8, PB_4, PA_2, PB_6, PB_5 // timers: TIM4_CH4,TIM4_CH3,TIM4_CH2,TIM1_CH1,IM3_CH1,TIM2_CH3,TIM4_CH1,TIM3_CH2

#elif defined(TARGET_PURPLEPILL)  // CJMCU1038 Board https://stm32-base.org/boards/STM32F103C8T6-Purple-Pill.html
    #define DPIN_LED        PB_11
    #define LED_INVERTED    1
    #define APIN_VBAT       PA_4  // PA_4=CS
    #define USART_INPUT     USART1  // UART1 RX=PA10 TX=PA9
    #define OUTPUT_PIN_MAP  PA_3, PA_2, PA_1, PA_0, PB_0, PB_1, PA_6, PA_7 // TIM2 CH1-4, TIM3CH1-4  PA_6=MIO PA_7=MOS

#elif defined(TARGET_RASPBERRY_PI_PICO)
    #define DPIN_LED        25
    #define LED_INVERTED    0
    //#define APIN_VBAT       26
    //#define USART_INPUT     Serial2
    #define DPIN_CRSF_RX    p5
    #define DPIN_CRSF_TX    p4
    #define OUTPUT_PIN_MAP  p10, p11, p12, p13, p14, p15, p16, p17

#endif

#if !defined(LED_INVERTED)
    #define LED_INVERTED    0
#endif

#if !defined(VBAT_R1) || !defined(VBAT_R2)
    // Resistor divider used on VBAT input, R1+R2 must be less than 3178
    #define VBAT_R1         820
    #define VBAT_R2         120
#endif
