/*
 * app_arm.h
 *
 * Created: 10/20/2023 5:09:58 PM
 *  Author: dmcdougall
 */ 


/*
 * app_arm.h
 *
 * Created: 2/16/2017 2:54:25 PM
 *  Author: jcollier
 */

#ifndef APP_ARM_H_
#define APP_ARM_H_

COMPILER_PACK_SET(1)

typedef struct AlarmStatus_t
{
    union
    {
        struct
        {
            uint16_t armed                      : 1;
            uint16_t silentAlarm                : 1;
            uint16_t channel_Alarm              : 1;    // ANY Channel can cause this, WHICH alarmed is in channel port status
            uint16_t powerTamper_Armed          : 1;
            uint16_t powerTamper_Alarm          : 1;
            uint16_t daisyChainTamper_Alarm     : 1;
            uint16_t daisyChainTamper_Armed     : 1; 
            uint16_t reserved                   : 9;
        };
        uint16_t sAlarm;
    };
} AlarmStatus_t;

COMPILER_PACK_RESET()

enum
{
    REMAIN_DISARMED     = 0,                        // nDISARM Pin state
    CAN_ARM             = 1,    
};

enum
{
    SYSTEM_DISARMED     = 0,
    SYSTEM_ARMED        = 1,
};

enum
{
    SENSOR_DISARMED = 0,
    SENSOR_ARMED    = 1,
};

enum
{
    DIDNT_ALARM  = 0,
    CAUSED_ALARM = 1,
};

enum
{
    NOT_SILENT_ALARMING = 0,
    SILENT_ALARMING     = 1,
};

enum  // Alarm-able Events in void app_arm_alarmEvent(uint8_t alarmCause)
{
    
    CHANNEL_0_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_1_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_2_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_3_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_4_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_5_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_6_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_7_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_8_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_9_SWITCH_WAS_OPENED,        // 0x00
    CHANNEL_10_SWITCH_WAS_OPENED,       // 0x00
    CHANNEL_11_SWITCH_WAS_OPENED,       // 0x00
    POWER_TAMPER_nMASTER_ALARM,         // 0x00
    DAISY_CHAIN_TAMPER_ALARM,           // 0x05
};

enum
{
    ARM_IGNORE_NONE,
    ARM_IGNORE_TETHER,
    ARM_IGNORE_RFID,
};

// ********************************************************************
// ********************************************************************
// ********************************************************************

void app_arm_init(void);
void app_arm_arm(void);
uint8_t app_arm_request(bool disarmOnFailure, uint8_t armIgnore);
void app_arm_disable_demo_mode(void);
bool app_arm_silence_alarm(void);
void app_arm_disarm(uint16_t duration);
void app_arm_alarmEvent(uint8_t alarmCause);
void app_arm_reset_auto_arm_timer(void);
bool app_arm_is_disarmed(void);
bool app_arm_is_silentalarming(void);
bool app_arm_is_any_alarm_active(void);
bool app_arm_is_auto_timer_running(void);
bool app_arm_is_DisarmDurationTimer_running(void);
uint16_t app_arm_get_disarmDuration(void);
uint16_t app_arm_get_alarm_status(void);
void app_arm_check_why_arm_failed(void);
void app_arm_set_auto_arm_timer_to_default_time(bool defaultArmTime);
void app_arm_stop_DisarmDuration_timer(void);
bool app_arm_get_system_armed(void);
bool app_arm_only_powerTamper_alarming(void);
void app_arm_set_PowerTamper_armed(bool powerTamperArmedState);
void app_arm_clear_PowerTamper_alarm(void);
void app_arm_set_daisyChainTamper_armed(bool daisyChainArmedState);
bool app_arm_get_daisyChainTamper_armed(void);
bool app_arm_is_daisyChain_alarming(void);

#endif /* APP_ARM_H_ */
