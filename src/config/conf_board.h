/**
 * \file
 *
 * \brief User board configuration
 *
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------
//		Apple 12 Port Alarm Module Pin definitions
//-----------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------

#include "config.h"

#warning "TODO: Verify LED/HIGH LOW logic level"
#define LED_ON                      HIGH
#define LED_OFF                     LOW

// Pin 01 ------------------------------------------------------------------------------------------------------------
// PA00 - BUZZER

// Defines Buzzer output.
#define BUZZER_MODULE               TCC2
#define BUZZER_CHANNEL              0  // Pin 01 - Buzzer -- Must match the Wave Output number
#define BUZZER_OUTPUT               0  // The Wave Out number, **NOT** the GPIO pin!
//	#define BUZZER_OUTPUT					PIN_PA00
#define BUZZER_PIN                  PIN_PA00E_TCC2_WO0
#define BUZZER_MUX                  MUX_PA00E_TCC2_WO0
#define BUZZER_PINMUX               PINMUX_PA00E_TCC2_WO0
#define BUZZER_CLK_SRC              GCLK_GENERATOR_5


// Pin 02 ------------------------------------------------------------------------------------------------------------
// PA01 - STAY_POWERED
// Assert this signal to keep battery power ON. (KILL switch)
#define BATT_DEADMAN_SW_PIN         PIN_PA01
#define BATT_DEADMAN_STAY_POWERED   HIGH
#define BATT_DEADMAN_POWER_DOWN     LOW
 
 
// Pin 03 ------------------------------------------------------------------------------------------------------------
// Channel 11             
#define CHANNEL_11_PIN              PIN_PA02                        //
#define CHANNEL_11_EIC_PIN          PIN_PA02A_EIC_EXTINT2
#define CHANNEL_11_EIC_MUX          MUX_PA02A_EIC_EXTINT2
#define CHANNEL_11_EIC_PINMUX       PINMUX_PA02A_EIC_EXTINT2
#define CHANNEL_11_EIC_LINE         2
 
 
// Pin 04 ------------------------------------------------------------------------------------------------------------
// Channel 10 
#define CHANNEL_10_PIN              PIN_PA03                        //
#define CHANNEL_10_EIC_PIN          PIN_PA03A_EIC_EXTINT3
#define CHANNEL_10_EIC_MUX          MUX_PA03A_EIC_EXTINT3
#define CHANNEL_10_EIC_PINMUX       PINMUX_PA03A_EIC_EXTINT3
#define CHANNEL_10_EIC_LINE         3


// Pin 05 ------------------------------------------------------------------------------------------------------------
// Channel 9
#define CHANNEL_9_PIN               PIN_PA04                        //
#define CHANNEL_9_EIC_PIN           PIN_PA04A_EIC_EXTINT4
#define CHANNEL_9_EIC_MUX           MUX_PA04A_EIC_EXTINT4
#define CHANNEL_9_EIC_PINMUX        PINMUX_PA04A_EIC_EXTINT4
#define CHANNEL_9_EIC_LINE          4


// Pin 06 ------------------------------------------------------------------------------------------------------------
// ARM - Heartbeat indicating arm status & daisy chain continuity
#define ARM_PIN                     PIN_PA05                        //
#define ARM_EIC_PIN                 PIN_PA05A_EIC_EXTINT5
#define ARM_EIC_MUX                 MUX_PA05A_EIC_EXTINT5
#define ARM_EIC_PINMUX              PINMUX_PA05A_EIC_EXTINT5
#define ARM_EIC_LINE                5


// Pin 07 ------------------------------------------------------------------------------------------------------------
// ARM_LED - Attaches the GND to the LEDs so they blink in unison
#define DISARMED_FLASH_PIN          PIN_PA06

// Pin 08 ------------------------------------------------------------------------------------------------------------
// Battery Monitor definitions
#define BAT_MON_PIN                 PIN_PA07  // Pin 48 -- Battery Monitor
#define BAT_MON_AIN                 ADC_POSITIVE_INPUT_PIN7


// Pin 09 ------------------------------------------------------------------------------------------------------------
// VDDANA


// Pin 10 ------------------------------------------------------------------------------------------------------------
// GND


// Pin 11 ------------------------------------------------------------------------------------------------------------
// nMASTER - if pulled low on boot this is the master AM, otherwise its a slave AM, only other time to check is power loss
// nMaster is only read on boot otherwise it would need to be an interrupt"
#define nMASTER_PIN                 PIN_PA08


// Pin 12 ------------------------------------------------------------------------------------------------------------
// PA09
// ISM- nDISARM signal from AP 3.1 and/or the control module for disarming the system
#define nDISARM_PIN                 PIN_PA09                        //
#define nDISARM_EIC_PIN             PIN_PA09A_EIC_EXTINT9
#define nDISARM_EIC_MUX             MUX_PA09A_EIC_EXTINT9
#define nDISARM_EIC_PINMUX          PINMUX_PA09A_EIC_EXTINT9
#define nDISARM_EIC_LINE            9


// Pin 13 ------------------------------------------------------------------------------------------------------------
// PA10
// ALARM - Alarm clock (sync signal) for synching alarms in the daisy chain
#define ALARM_DETECT_PIN            PIN_PA10                        //
#define ALARM_DETECT_EIC_PIN        PIN_PA10A_EIC_EXTINT10
#define ALARM_DETECT_EIC_MUX        MUX_PA10A_EIC_EXTINT10
#define ALARM_DETECT_EIC_PINMUX     PINMUX_PA10A_EIC_EXTINT10
#define ALARM_DETECT_EIC_LINE       10


// Pin 14 ------------------------------------------------------------------------------------------------------------
// PA11
// Power Good Pin definition
#define POWER_GOOD_PIN              PIN_PA11                        //
#define POWER_GOOD_EIC_PIN          PIN_PA11A_EIC_EXTINT11
#define POWER_GOOD_EIC_MUX          MUX_PA11A_EIC_EXTINT11
#define POWER_GOOD_EIC_PINMUX       PINMUX_PA11A_EIC_EXTINT11
#define POWER_GOOD_EIC_LINE         11


// Pin 15 ------------------------------------------------------------------------------------------------------------
// PA14
// Channel 8
#define CHANNEL_8_PIN               PIN_PA14                        //
#define CHANNEL_8_EIC_PIN           PIN_PA14A_EIC_EXTINT14
#define CHANNEL_8_EIC_MUX           MUX_PA14A_EIC_EXTINT14
#define CHANNEL_8_EIC_PINMUX        PINMUX_PA14A_EIC_EXTINT14
#define CHANNEL_8_EIC_LINE          14


// Pin 16 ------------------------------------------------------------------------------------------------------------
// PA15
// nALARM - Signal an alarm on the ALARM daisy chain line
#define ALARM_TRIGGER_PIN           PIN_PA15

// Cut Detect (Tone) definitions
// #define TONE_SIGNAL_MODULE  TCC0  // Pin 32 - Tether Signal Output
// #define TONE_SIGNAL_CHANNEL 1
// #define TONE_SIGNAL_OUTPUT  1
// #define TONE_SIGNAL_PIN     PIN_PA23F_TCC0_WO5
// #define TONE_SIGNAL_MUX     MUX_PA23F_TCC0_WO5
// #define TONE_SIGNAL_PINMUX  PINMUX_PA23F_TCC0_WO5

// Pin 17 ------------------------------------------------------------------------------------------------------------
// PA16
// Channel 7
#define CHANNEL_7_PIN               PIN_PA16                        //
#define CHANNEL_7_EIC_PIN           PIN_PA16A_EIC_EXTINT0
#define CHANNEL_7_EIC_MUX           MUX_PA16A_EIC_EXTINT0
#define CHANNEL_7_EIC_PINMUX        PINMUX_PA16A_EIC_EXTINT0
#define CHANNEL_7_EIC_LINE          0


// Pin 18 ------------------------------------------------------------------------------------------------------------
// PA17
// Channel 6
#define CHANNEL_6_PIN               PIN_PA17                        //
#define CHANNEL_6_EIC_PIN           PIN_PA17A_EIC_EXTINT1
#define CHANNEL_6_EIC_MUX           MUX_PA17A_EIC_EXTINT1
#define CHANNEL_6_EIC_PINMUX        PINMUX_PA17A_EIC_EXTINT1
#define CHANNEL_6_EIC_LINE          1


// Pin 19 ------------------------------------------------------------------------------------------------------------
// PA18 - DEBUG UART TX

// Pin 20 ------------------------------------------------------------------------------------------------------------
// PA19 - DEBUG UART RX
// Debug RX/TX
#define DEBUG_UART_MODULE   SERCOM1
#define DEBUG_UART_BAUDRATE 115200
#define DEBUG_UART_TX_PIN   PIN_PA18  // Pin 27 - DEBUG UART TX
#define DEBUG_UART_RX_PIN   PIN_PA19  // Pin 28 - DEBUG UART RX

#define DEBUG_UART_SERCOM_MUX_SETTING USART_RX_3_TX_2_XCK_3
#define DEBUG_UART_SERCOM_PINMUX_PAD0 PINMUX_UNUSED
#define DEBUG_UART_SERCOM_PINMUX_PAD1 PINMUX_UNUSED
#define DEBUG_UART_SERCOM_PINMUX_PAD2 PINMUX_PA18C_SERCOM1_PAD2
#define DEBUG_UART_SERCOM_PINMUX_PAD3 PINMUX_PA19C_SERCOM1_PAD3

#define DEBUG_UART_SERCOM_DMAC_ID_RX SERCOM1_DMAC_ID_RX


// Pin 21 ------------------------------------------------------------------------------------------------------------
// PA22
// Channel 5
#define CHANNEL_5_PIN               PIN_PA22                        //
#define CHANNEL_5_EIC_PIN           PIN_PA22A_EIC_EXTINT6
#define CHANNEL_5_EIC_MUX           MUX_PA22A_EIC_EXTINT6
#define CHANNEL_5_EIC_PINMUX        PINMUX_PA22A_EIC_EXTINT6
#define CHANNEL_5_EIC_LINE          6


// Pin 22 ------------------------------------------------------------------------------------------------------------
// PA23
// Channel 4
#define CHANNEL_4_PIN               PIN_PA23                        //
#define CHANNEL_4_EIC_PIN           PIN_PA23A_EIC_EXTINT7
#define CHANNEL_4_EIC_MUX           MUX_PA23A_EIC_EXTINT7
#define CHANNEL_4_EIC_PINMUX        PINMUX_PA23A_EIC_EXTINT7
#define CHANNEL_4_EIC_LINE          7


// Pin 23 ------------------------------------------------------------------------------------------------------------
// PA24
// Channel 3
#define CHANNEL_3_PIN               PIN_PA24                        //
#define CHANNEL_3_EIC_PIN           PIN_PA24A_EIC_EXTINT12
#define CHANNEL_3_EIC_MUX           MUX_PA24A_EIC_EXTINT12
#define CHANNEL_3_EIC_PINMUX        PINMUX_PA24A_EIC_EXTINT12
#define CHANNEL_3_EIC_LINE          12


// Pin 24 ------------------------------------------------------------------------------------------------------------
// PA25
// Channel 3
#define CHANNEL_2_PIN               PIN_PA25                        //
#define CHANNEL_2_EIC_PIN           PIN_PA25A_EIC_EXTINT13
#define CHANNEL_2_EIC_MUX           MUX_PA25A_EIC_EXTINT13
#define CHANNEL_2_EIC_PINMUX        PINMUX_PA25A_EIC_EXTINT13
#define CHANNEL_2_EIC_LINE          13


// Pin 25 ------------------------------------------------------------------------------------------------------------
// PA27
// Channel 1
#define CHANNEL_1_PIN               PIN_PA27                        //
#define CHANNEL_1_EIC_PIN           PIN_PA27A_EIC_EXTINT15
#define CHANNEL_1_EIC_MUX           MUX_PA27A_EIC_EXTINT15
#define CHANNEL_1_EIC_PINMUX        PINMUX_PA27A_EIC_EXTINT15
#define CHANNEL_1_EIC_LINE          15


// Pin 26 ------------------------------------------------------------------------------------------------------------
// nRST


// Pin 27 ------------------------------------------------------------------------------------------------------------
// PA28
// Channel 0
#define CHANNEL_0_PIN               PIN_PA28A_EIC_EXTINT8                        //
#define CHANNEL_0_EIC_PIN           PIN_PA28A_EIC_EXTINT8
#define CHANNEL_0_EIC_MUX           MUX_PA28A_EIC_EXTINT8
#define CHANNEL_0_EIC_PINMUX        PINMUX_PA28A_EIC_EXTINT8
#define CHANNEL_0_EIC_LINE          8


// Pin 28 ------------------------------------------------------------------------------------------------------------
// GND


// Pin 29 ------------------------------------------------------------------------------------------------------------
// VDDCORE - 1V8


// Pin 30 ------------------------------------------------------------------------------------------------------------
// PA23
// VDDIN - 3V3


// Pin 31 ------------------------------------------------------------------------------------------------------------
// SWDCLK


// Pin 32 ------------------------------------------------------------------------------------------------------------
// SWDIO

#endif  // CONF_BOARD_H
