/*
 * app_buzzer.h
 *
 * Created: 10/20/2023 5:13:22 PM
 *  Author: dmcdougall
 */ 


/*
 * app_buzzer.h
 *
 * Created: 8/20/2015 11:12:10 AM
 *  Author: jcollier
 */

#ifndef APP_BUZZER_H_
#define APP_BUZZER_H_

enum app_buzzer_pattern_t
{
    BUZ_PAT_NONE                    = 0x00,
    BUZ_PAT_ERROR                   = 0x01,
    BUZ_PAT_DELAY_ERROR             = 0x02,
    BUZ_PAT_SUCCEED                 = 0x03,
    BUZ_PAT_SUCCEED_CANT            = 0x04,
    BUZ_PAT_SUCCEED_LOW_BAT         = 0x05,
    BUZ_PAT_BAT_LOW                 = 0x06,
    BUZ_PAT_EOL                     = 0x07,
    BUZ_PAT_SKELETON                = 0x08,
    BUZ_PAT_DELETE                  = 0x09,
    BUZ_PAT_FACTORY_TEST            = 0x0A,
    BUZ_PAT_FACTORY_TEST2           = 0x0B,
    BUZ_PAT_PROVISION               = 0x0C,
    BUZ_PAT_AUTH_REPLACE_CONFIRMING = 0x0D,
    BUZ_PAT_BASE_NO_POWER           = 0x0E,
    BUZ_PAT_PUCK_DEEP_SLEEP         = 0x0F,
    BUZ_PAT_WARNING                 = 0x10,
    BUZ_PAT_ALERT                   = 0x11,
    BUZ_PAT_RFID_ERROR              = 0x12,
    BUZ_PAT_ALARM                   = 0xFF
};

enum app_buzzer_freq_t
{
    BEEP_PAUSE = 0,
    BEEP_REPEAT,
    BEEP_1,  // 4500 Hz
    BEEP_2,  // 4200 Hz
    BEEP_3,  // 4000 Hz
    BEEP_4,  // 3000 Hz
    BEEP_5,  // 2700 Hz
    BEEP_6,  // 2637 Hz
    BEEP_7   // 2000 Hz
};

enum app_buzzer_volume_t
{
    VOLUME_OFF = 0x00,
    VOLUME_MIN = 0x01,
    VOLUME_MAX = 0xFF,
};

enum app_buzzer_state_t
{
    BUZ_STATE_IDLE,
    BUZ_STATE_PROCESSING,
    BUZ_STATE_TIMEOUT,
    BUZ_STATE_END
};

//  Core
void app_buzzer_init(void);
void app_buzzer_task(void);
void app_buzzer_start_pattern(enum app_buzzer_pattern_t pattern);
void app_buzzer_stop_pattern(enum app_buzzer_pattern_t pattern);
enum app_buzzer_pattern_t app_buzzer_pattern_playing(void);

//  Alarm - Secure Tone
void app_buzzer_alarm_start(void);
void app_buzzer_alarm_stop(void);

#endif /* APP_BUZZER_H_ */
