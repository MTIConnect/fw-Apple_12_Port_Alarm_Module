/*
 * app_LED.h
 *
 * Created: 10/20/2023 5:07:20 PM
 *  Author: dmcdougall
 */ 

#ifndef APP_LED_H_
#define APP_LED_H_

// Used in appLedControl
#define APP_LED_BLUE_EXTERNAL           0x00
#define APP_LED_GREEN_EXTERNAL          0x01
#define APP_LED_RED_EXTERNAL            0x02
#define APP_LED_EXTERNAL                0x03
#define APP_LED_BLUE_GREEN_EXTERNAL     0x04
#define APP_LED_BLUE_RED_EXTERNAL       0x05
#define APP_LED_GREEN_RED_EXTERNAL      0x06
#define APP_LED_BLUE                    0x07
#define APP_LED_GREEN                   0x08
#define APP_LED_RED                     0x09
#define APP_LED_BLUE_GREEN              0x0A
#define APP_LED_BLUE_RED                0x0B
#define APP_LED_GREEN_RED               0x0C
#define APP_LED_RED_GREEN_BLUE          0x0D
#define APP_LED_RED_GREEN_BLUE_EXTERNAL 0x0E
// 0x0F, Left out on purpose so APP_LED_ALARM and APP_LED_ALARM_EXTERNAL couldn't be called
// If called by user the LEDs are all turned off for the duration specified
#define APP_LED_ALARM          0x10
#define APP_LED_ALARM_EXTERNAL 0x11
#define APP_LED_ALARM_ALL      APP_LED_ALARM_EXTERNAL

// Combined Colors
#define APP_LED_CYAN            APP_LED_BLUE_GREEN
#define APP_LED_CYAN_EXTERNAL   APP_LED_BLUE_GREEN_EXTERNAL
#define APP_LED_PURPLE          APP_LED_BLUE_RED
#define APP_LED_PURPLE_EXTERNAL APP_LED_BLUE_RED_EXTERNAL
#define APP_LED_YELLOW          APP_LED_GREEN_RED
#define APP_LED_YELLOW_EXTERNAL APP_LED_GREEN_RED_EXTERNAL
#define APP_LED_WHITE           APP_LED_RED_GREEN_BLUE
#define APP_LED_WHITE_EXTERNAL  APP_LED_RED_GREEN_BLUE_EXTERNAL

#define APP_LED_DURATION_FOREVER 0xFFFF

void app_LED_init(void);
void app_led_identify(uint16_t iDuration, uint8_t iColor, uint8_t iMode);
bool app_led_is_identifying(void);
void app_led_control(uint16_t iDuration, uint8_t iColor, uint8_t iMode);
// void appLedAlarm(void);
void app_led_communication_state(bool communicationState);
bool app_led_communication_get_state(void);
void app_led_armed_state(bool armedState);
void app_led_power_state(bool powerState);
void app_LED_emit_RGBCause_pattern(void);
void app_led_update(void);

/*
LED state

In Idle State
Communication Good = white
Communication bad = Yellow

In Idle State
Power Good = LED on
Power Bad = LED off

In Idle State
armed = on solid
disarmed = 5s flashy

Identifying = Identifying State
Not identifying = Idle State

Alarming = Alarming State
Not Alarming = Idle State


States:
APP_LED_STATE_IDLE,
APP_LED_STATE_IDENTIFYING,
APP_LED_STATE_ALARM,
*/

#endif /* APP_LED_H_ */
