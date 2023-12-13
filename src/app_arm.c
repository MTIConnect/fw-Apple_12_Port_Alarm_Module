/*
 * app_arm.c
 *
 * Created: 10/20/2023 5:09:46 PM
 *  Author: dmcdougall
 */ 


/*
 * app_arm.c
 *
 * Created: 2/16/2017 2:53:57 PM
 *  Author: jcollier
 *  Copyright 2017, 2018, 2019 Mobile Technologies, Inc.
 */

#include <asf.h>
#include "app_arm.h"
#include "app_LED.h"
#include "app_buzzer.h"
#include "app_eeprom.h"
#include "app_gen_io.h"
#include "app_uart.h"
//#include "app_user_options.h"
#include "sysTimer.h"

#define AUTO_ARM_TIME              60000    // 1 minute non-boot auto arm
#define DEFAULT_MODE_AUTO_ARM_TIME 900000   // 15 min on boot
#define ALARM_TIME_BEFORE_SILENT   300000   // Limit changed from 10 min -> 5 min 8/25/2020
#define DISARM_DURATION            1000
#define DISARM_FLASH_TIME          1000     // Milli-seconds till pulse

// #define ENABLE_ARM_DEBUG_MSGS 1 // Uncomment to print out Debug messages

////////////////////////////////////////////////////////////////
// Local variables
static SYS_Timer_t appAutoArmTimer;
static SYS_Timer_t app_arm_alarm_LimitTimer;
static SYS_Timer_t appDisarmDurationTimer;
static SYS_Timer_t appDisarmFlashTimer;
static bool keyArmInBBU;
static AlarmStatus_t armAlarmStatus;
volatile uint16_t disarmDuration;

extern volatile PortStatus_t portStatus[CH_COUNT];



////////////////////////////////////////////////////////////////
// Local function prototypes
static void app_arm_alarm_LimitTimerHandler(SYS_Timer_t *timer);
static void appDisarmFlashTimerHandler(SYS_Timer_t *timer);
bool app_arm_is_armed(void);


////////////////////////////////////////////////////////////////
// Functions
bool app_arm_is_auto_timer_running(void)
{
    return SYS_TimerStarted(&appAutoArmTimer);
}

////////////////////////////////////////////////////////////////
bool app_arm_is_DisarmDurationTimer_running(void)
{
    return SYS_TimerStarted(&appDisarmDurationTimer);
}

////////////////////////////////////////////////////////////////
uint16_t app_arm_get_disarmDuration(void)
{
    return disarmDuration;
}

////////////////////////////////////////////////////////////////
bool app_arm_is_armed(void)
{
    return armAlarmStatus.armed;
}

////////////////////////////////////////////////////////////////
bool app_arm_is_silentalarming(void)
{
    return (SILENT_ALARMING == armAlarmStatus.silentAlarm);
}

////////////////////////////////////////////////////////////////
bool app_arm_is_any_alarm_active(void)
{   	
    bool result = false;
    
    if(app_arm_is_armed())
    {
        if(((armAlarmStatus.powerTamper_Alarm) ||\
            (armAlarmStatus.daisyChainTamper_Alarm) ||\
            (armAlarmStatus.channel_Alarm) ) )
        {
            result = true;
        }
        
    }

    return  result; 
}

////////////////////////////////////////////////////////////////
static bool app_arm_auto_request(void)
{

    if (SYS_TimerStarted(&appDisarmDurationTimer))
    {
        // Don't auto arm while disarmDuration timer is running
        UART_DBG_TX("Couldn't Arm because of DisarmDurationTimer\n");
        return false;
    }

//    AlarmModuleStatus_t alarmModSts;
//     alarmModSts.sAlarmModule = app_gen_io_get_AM_status();
    
    bool channelHasArmed = false; 
    

    #warning "TODO: What are the base arming requriements? Loopback? Powered?"
    
    if ( (app_gen_io_is_power_good() == POWER_GOOD) &&\
         (app_gen_io_get_nDISARM() == CAN_ARM) )
    {
        // We have power & nDISARM is HIGH, so we can arm new ports
        
        UART_DBG_TX("\n\n******** Powerd and ArmLB Connected ******** \n\n");
        
        // Refreshing all port
        for(int num = 0; num < CH_COUNT; num++)
        {
            if( app_gen_io_is_cable_present(num) ) 					
            {
                // If a cable is seen then the switch is pressed and its ready to arm
                // If we are alarming auto arm doesn't clear/rearm it. 
                
                if( app_gen_io_is_port_armed(num) == PORT_DISARMED )
                {
                    UART_DBG_TX("Armed port from Disarmed State\n");
                    app_gen_io_set_port_armed(num, SENSOR_ARMED); //set port to armed (!disarmed)
                    armAlarmStatus.armed = SYSTEM_ARMED;
                    channelHasArmed = true;
                    
                    SYS_TimerStop(&appDisarmFlashTimer);
                    //port_pin_set_output_level(DISARMED_FLASH_PIN, LOW);
                }
                        
                // this is armed port, is it also silent Alarming?
                if( (armAlarmStatus.silentAlarm == SILENT_ALARMING ) ) // && )
                {
                    UART_DBG_TX("Armed port from Silent ALarming State\n");
                    app_gen_io_set_port_armed(num, SENSOR_ARMED); //set port to armed (!disarmed)
                    armAlarmStatus.armed = SYSTEM_ARMED;
                    channelHasArmed = true;
                    SYS_TimerStop(&appDisarmFlashTimer);
                    //port_pin_set_output_level(DISARMED_FLASH_PIN, LOW);
                }
            }
        }
         
        if( channelHasArmed || app_gen_io_is_any_port_armed())
        {
            // There is at least 1 channel armed   
            armAlarmStatus.armed                    = SYSTEM_ARMED;
        }       
        
        app_arm_reset_auto_arm_timer();  
    }        

    UART_DBG_TX("\n+++++ AUTO ARM REQUEST COMPLETE +++++\n ");
    return true;
}

static void appAutoArmTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    app_arm_auto_request();
}

static void appDisarmDurationTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    SYS_TimerRestart(&appAutoArmTimer);
}

void app_arm_reset_auto_arm_timer(void)
{
    SYS_TimerRestart(&appAutoArmTimer);
#ifdef ENABLE_ARM_DEBUG_MSGS
    UART_DBG_TX("Auto-arm Timer was reset\n");
#endif
}

static void app_arm_alarm_LimitTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    app_arm_silence_alarm();
}

void app_arm_init(void)
{
    app_arm_disarm(0);
    disarmDuration = 0;

    if (APP_EEPROM_MODEL_TYPE_FACTORY == app_eeprom_read_connect_wanted())
    {
        appAutoArmTimer.interval = DEFAULT_MODE_AUTO_ARM_TIME;
        // app_arm_set_auto_arm_timer_to_default_time();
    }
    else
    {
        appAutoArmTimer.interval = AUTO_ARM_TIME;
        // app_arm_set_auto_arm_timer_to_default_time();
    }

    appAutoArmTimer.mode    = SYS_TIMER_INTERVAL_MODE;
    appAutoArmTimer.handler = appAutoArmTimerHandler;
    SYS_TimerStart(&appAutoArmTimer);

    app_arm_alarm_LimitTimer.interval = ALARM_TIME_BEFORE_SILENT;           // 10 min -> 5min 8/25/2020
    app_arm_alarm_LimitTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    app_arm_alarm_LimitTimer.handler  = app_arm_alarm_LimitTimerHandler;

    appDisarmDurationTimer.interval = DISARM_DURATION;
    appDisarmDurationTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    appDisarmDurationTimer.handler  = appDisarmDurationTimerHandler;

    keyArmInBBU = false;
    
    
    struct port_config pin_conf;
    port_get_config_defaults(&pin_conf);
    
    pin_conf.direction = PORT_PIN_DIR_OUTPUT;
    port_pin_set_config(DISARMED_FLASH_PIN, &pin_conf);
    port_pin_set_output_level(DISARMED_FLASH_PIN, HIGH);         // High => N-Channel FET Should default connected to detect cables at start. 
    
    appDisarmFlashTimer.interval = DISARM_FLASH_TIME;           // delay
    appDisarmFlashTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    appDisarmFlashTimer.handler  = appDisarmFlashTimerHandler;
}

// ****************************************************************************
//		ONLY MASTER HAS THE POWER TAMPER ALARM
// ****************************************************************************
void app_arm_set_PowerTamper_armed(bool powerTamperArmedState)
{
    if( powerTamperArmedState )
    {
        armAlarmStatus.powerTamper_Armed = SYSTEM_ARMED; 
    }
    else
    {
       armAlarmStatus.powerTamper_Armed = SYSTEM_DISARMED; 
    }
}


void app_arm_clear_PowerTamper_alarm(void)
{
    armAlarmStatus.powerTamper_Alarm = DIDNT_ALARM;
}

// ****************************************************************************
//		
// ****************************************************************************
void app_arm_set_daisyChainTamper_armed(bool daisyChainArmedState)
{
    if( daisyChainArmedState )
    {
        armAlarmStatus.daisyChainTamper_Armed = SYSTEM_ARMED; 
        armAlarmStatus.daisyChainTamper_Alarm = DIDNT_ALARM;
    }
    else
    {
        armAlarmStatus.daisyChainTamper_Armed = SYSTEM_DISARMED;
    }  
}

bool app_arm_get_daisyChainTamper_armed(void)
{
    return armAlarmStatus.daisyChainTamper_Armed;
}

// *****************************************************************************
//		FUNCTION TO MAKE AN ARM ATTEMPT
// *****************************************************************************
uint8_t app_arm_request(bool disarmOnFail, uint8_t armIgnore)
{
    app_arm_auto_request();
    
    
//     PuckStatus_t puckStatus;
//     BaseStatus_t baseStatus;
//     puckStatus.sPuck = app_gen_io_get_puck_status();
//     baseStatus.sBase = app_puckToBaseCom_get_status();
// 
//     bool ready_to_arm = true;
// 
//     app_buzzer_alarm_stop();
// 
//     // Check that the state of RFID keys, RF state, or connect state supports disarming.
// 
//     if (ARM_IGNORE_RFID != armIgnore)
//     {
//         // RFID state should only be ignored when failing Auth Replacement
//         // We still want to disallow auto arming while in that state, but
//         // bypass the state restriction on fail conditions since it's an explicit arm attempt
//         if (false == app_rfid_state_get_armOK())
//         {
//             ready_to_arm = false;
// #ifdef ENABLE_ARM_DEBUG_MSGS
//             UART_DBG_TX("\tRFID not ready\n");
// #endif
//         }
//     }
// 
//     if ((ARM_IGNORE_TETHER != armIgnore) || (TETHER_ALARMS_DISABLED == app_user_options_get_tetherAlarmEnable()))
//     {
//         if (TETHER_OPENED == puckStatus.noTether)  // Must have Tether
//         {
//             ready_to_arm = false;
// #ifdef ENABLE_ARM_DEBUG_MSGS
//             UART_DBG_TX("\tNo Tether\n");
// #endif
//         }
//     }
// 
//     if (baseStatus.notPowered)  // Must have Base Power
//     {
//         ready_to_arm = false;
// #ifdef ENABLE_ARM_DEBUG_MSGS
//         UART_DBG_TX("\tBase Not Powered\n");
// #endif
//     }
// 
//     if (PUCK_SWITCH_OPENED == puckStatus.puckSwitchLifted)  // Must have Puck Switch Pressed
//     {
//         ready_to_arm = false;
// #ifdef ENABLE_ARM_DEBUG_MSGS
//         UART_DBG_TX("\tPuck Switch Lifted\n");
// #endif
//     }
// 
//     // 	// Don't arm unless we have at least one user key.
//     //	ready_to_arm &= (app_eeprom_key_table_count_valid() > 0);
// 
//     if (ready_to_arm)
//     {
//         app_arm_arm();
// 
//         // 		if (ARM_IGNORE_TETHER == armIgnore)
//         // 		{
//         // 			// Set Demo flag
//         // 			armAlarmStatus.demoMode = true;
//         // 		}
//     }
//     else if (disarmOnFail)
//     {
//         app_arm_disarm(0);
//     }
//     else
//     {
//         // We might be silent alarming, and don't want to disarm here.
//     }
// 
//     app_led_update();
// 
//     // We'll send status elsewhere, so that we can send keys (or not)
//     // app_lwmesh_send_status();
// 
//     if (NORMALLY_LATCHED == app_user_options_get_normallyLatched())
//     {
//         app_puckToBase_send_SL(BASE_LOCK_COMMAND);
//     }
//     else
//     {
//         app_puckToBase_send_SL(BASE_UNLOCK_COMMAND);
//     }

    return armAlarmStatus.armed; //(ready_to_arm & !armAlarmStatus.disarmed);
}


// ****************************************************************************
//		FUNCTION TO ENTER SILENT ALARMING STATE -
// ****************************************************************************
bool app_arm_silence_alarm(void)
{
    app_arm_reset_auto_arm_timer();

    if ((app_arm_is_any_alarm_active()) && (armAlarmStatus.silentAlarm == NOT_SILENT_ALARMING))
    {
        armAlarmStatus.silentAlarm = SILENT_ALARMING;
        app_buzzer_alarm_stop();                   // Silence the alarm
        SYS_TimerStop(&app_arm_alarm_LimitTimer);  // Don't come here again
        // Have caller to send status, as it may want to send a key event.
        // app_lwmesh_send_status();
        return true;  // Silent Alarming
    }
    else
    {
        // Disarm
        app_led_update();
        app_arm_disarm(0);
        // Have caller to send status, as it may want to send a key event.
        // app_lwmesh_send_status();
        return false;  // Couldn't enter silent alarming mode
    }
}

// *****************************************************************************************************
//		FUNCTION TO ARM THE DEVICE
// *****************************************************************************************************
void app_arm_arm(void)
{
    // Refreshing all port
    for(int num = 0; num < CH_COUNT; num++)
    {
        if(app_gen_io_is_cable_present(num))
        {
            // If a cable is seen then the switch is pressed and its ready to arm       
            if( !app_gen_io_is_port_armed(num) ) 			    // this is disarmed port
            {
                app_gen_io_set_port_armed(num, SENSOR_ARMED); //set port to armed (!disarmed)
                armAlarmStatus.armed = SYSTEM_ARMED;
                
                SYS_TimerStop(&appDisarmFlashTimer);
                port_pin_set_output_level(DISARMED_FLASH_PIN, HIGH);
            }
        }
        
//         if( (armAlarmStatus.silentAlarm) )
//         {
//             app_gen_io_set_port_armed(i, SENSOR_DISARMED);
//         }
    }
    
    
    armAlarmStatus.armed                    = SYSTEM_ARMED;
    armAlarmStatus.silentAlarm              = NOT_SILENT_ALARMING;  
    armAlarmStatus.channel_Alarm            = DIDNT_ALARM;
    armAlarmStatus.daisyChainTamper_Alarm   = DIDNT_ALARM;

    SYS_TimerStop(&appDisarmDurationTimer);

    app_led_update();
}

// ****************************************************************************
//		FUNCTION TO DISARM THE DEVICE
// ****************************************************************************
void app_arm_disarm(uint16_t duration)
{
    disarmDuration = duration;
    if (duration > 0)
    {
        appDisarmDurationTimer.interval = 1000 * duration;
        SYS_TimerRestart(&appDisarmDurationTimer);
    }

    armAlarmStatus.armed                        = SYSTEM_DISARMED;
    armAlarmStatus.silentAlarm                  = NOT_SILENT_ALARMING;
    armAlarmStatus.powerTamper_Alarm            = DIDNT_ALARM;
    armAlarmStatus.channel_Alarm                = DIDNT_ALARM;
    armAlarmStatus.daisyChainTamper_Alarm       = DIDNT_ALARM;
    
    for(int num = 0; num < CH_COUNT; num++)
    {
        app_gen_io_set_port_armed(num, SENSOR_DISARMED);        
    }
    
    
    // Needs to exist for the condition when the switch is lifted after alarm
    // timeout occurs, that's not a disarm or a re-arm state
    app_arm_reset_auto_arm_timer();

//     app_led_update();
//     app_led_control(1, APP_LED_WHITE_EXTERNAL, 4);
    app_buzzer_alarm_stop();
    SYS_TimerStop(&app_arm_alarm_LimitTimer);   // try to silentAlarm if not alarming
    
    SYS_TimerStart(&appDisarmFlashTimer);       //
}

// ****************************************************************************
//		FUNCTION TO ALARM THE DEVICE
// ****************************************************************************
void app_arm_alarmEvent(uint8_t alarmCause)
{
    bool newAlarm = false;
    
    if( (SYSTEM_ARMED == armAlarmStatus.armed) ||\
        (SYSTEM_ARMED == armAlarmStatus.daisyChainTamper_Armed ) ||\
        (SYSTEM_ARMED == armAlarmStatus.powerTamper_Armed) )
    {
        // Either a channel is armed or the system alarms (power daisy chain) are armed
        switch (alarmCause)
        {
            case CHANNEL_0_SWITCH_WAS_OPENED:
            case CHANNEL_1_SWITCH_WAS_OPENED:
            case CHANNEL_2_SWITCH_WAS_OPENED:
            case CHANNEL_3_SWITCH_WAS_OPENED:
            case CHANNEL_4_SWITCH_WAS_OPENED:
            case CHANNEL_5_SWITCH_WAS_OPENED:
            case CHANNEL_6_SWITCH_WAS_OPENED:
            case CHANNEL_7_SWITCH_WAS_OPENED:
            case CHANNEL_8_SWITCH_WAS_OPENED:
            case CHANNEL_9_SWITCH_WAS_OPENED:
            case CHANNEL_10_SWITCH_WAS_OPENED:
            case CHANNEL_11_SWITCH_WAS_OPENED:
                armAlarmStatus.channel_Alarm            = CAUSED_ALARM;
                newAlarm                                = true;
                UART_DBG_TX("CHANNEL ALARMED");
                break;

            case POWER_TAMPER_nMASTER_ALARM:
                armAlarmStatus.powerTamper_Alarm        = CAUSED_ALARM;
                newAlarm                                = true;
                UART_DBG_TX("POWER TAMPER ALARM");
                break;
            
            case DAISY_CHAIN_TAMPER_ALARM:
                armAlarmStatus.daisyChainTamper_Alarm   = CAUSED_ALARM;
                newAlarm                                = true;
                UART_DBG_TX("DAISY CHAIN TAMPER ALARM");
                break;

            default:
                break;
        }

        if (newAlarm)
        {
            armAlarmStatus.silentAlarm = NOT_SILENT_ALARMING;
            SYS_TimerStart(&app_arm_alarm_LimitTimer);
            app_buzzer_alarm_start();
        }
        app_led_update();
    }
}

uint16_t app_arm_get_alarm_status(void)
{
    return armAlarmStatus.sAlarm;
}

void app_arm_check_why_arm_failed(void)
{
//     PuckStatus_t puckStatus;
//     BaseStatus_t baseStatus;
//     puckStatus.sPuck = app_gen_io_get_puck_status();
//     baseStatus.sBase = app_puckToBaseCom_get_status();
// 
//     if (!app_rfid_state_get_armOK())
//     {
//         printf("\tRFID not ready\n");
//     }
// 
//     if (TETHER_OPENED == puckStatus.noTether)
//     {
//         printf("\tNo Tether\n");
//     }
// 
//     if (baseStatus.notPowered)
//     {
//         printf("\tBase Not Powered\n");
//     }
// 
//     if (puckStatus.puckSwitchLifted)
//     {
//         printf("\tPuck Switch Lifted\n");
//     }
// 
//     if (!app_eeprom_key_table_count_valid())
//     {
//         printf("\tInvalid Key table count\n");
//     }
}

void app_arm_set_auto_arm_timer_to_default_time(bool defaultArmTime)
{
    if (defaultArmTime)
    {
        appAutoArmTimer.interval = DEFAULT_MODE_AUTO_ARM_TIME;
    }
    else
    {
        appAutoArmTimer.interval = AUTO_ARM_TIME;
    }
    SYS_TimerRestart(&appAutoArmTimer);
}

void app_arm_stop_DisarmDuration_timer(void)
{
    SYS_TimerStop(&appDisarmDurationTimer);
    SYS_TimerRestart(&appAutoArmTimer);
}


bool app_arm_get_system_armed(void)
{
    return armAlarmStatus.armed; 
}

bool app_arm_only_powerTamper_alarming(void)
{
    bool result = false;
    
   if( (CAUSED_ALARM == armAlarmStatus.powerTamper_Alarm) &&\
       (DIDNT_ALARM == armAlarmStatus.daisyChainTamper_Alarm) &&\
       (DIDNT_ALARM == armAlarmStatus.channel_Alarm) )
   {
      result = true;  
   }       

   return  result;
}


bool app_arm_is_daisyChain_alarming(void)
{
    return armAlarmStatus.daisyChainTamper_Alarm;
}

static void appDisarmFlashTimerHandler(SYS_Timer_t *timer)
{
   // port_pin_toggle_output_level(DISARMED_FLASH_PIN);
}
