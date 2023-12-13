/*
 * app_bbu.c
 *
 * Created: 10/20/2023 5:17:06 PM
 *  Author: dmcdougall
 */ 

#include <asf.h>
#include "app_bbu.h"
#include "app_LED.h"
#include "app_adc.h"
#include "app_arm.h"
#include "app_buzzer.h"
#include "app_gen_io.h"
#include "app_uart.h"  // For debug prints
#include "app_wdt.h"
#include "conf_clocks.h"
#include "slpTimer.h"  // 16 hour BBU time limit
#include "sysTimer.h"

#define BATTERY_LEVEL_SLEEP 3800  // mV
#define BATTERY_LEVEL_LOW   3900  // mV
#define BATTERY_LEVEL_INIT  4200  // mV

// #define ENABLE_BBU_DEBUG_MSGS 1 // Uncomment to print out Debug messages

#ifdef INCLUDE_ALL_DEBUG_FUNCTIONS
    #define BBU_TIME_LIMIT (1000 * 60 * 5)  // 5 minutes
#else
    #define BBU_TIME_LIMIT (1000 * 60 * 60 * 4)  // 4 hours
#endif

typedef enum AppBbuState_t
{
    APP_BBU_STATE_ACTIVE,
    APP_BBU_STATE_SLEEP,
    APP_BBU_STATE_TIRED,  // device woke up from battery rebound, or when radio use stops, and it needs to go back to sleep.
} AppBbuState_t;

static AppBbuState_t appBbuState;
static SYS_Timer_t appBatteryCheckTimer;
static SLP_Timer_t appBBUTimeLimitTimer;
static uint16_t batteryLevel;
static bool batteryCharging;
static bool shelfStorageTimerCompleteFlag;

static void appBatteryCheckTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    app_adc_configure(BAT_MON_AIN, app_bbu_battery_check_ADC_complete_callback);
    // Note that the timer is periodic, so that if the ADC happens to be busy,
    // we'll just lose a reading and catch one the next time the timer fires.
}

static void appBBUTimeLimitTimerHandler(SLP_Timer_t *timer)
{
    UNUSED(timer);
    shelfStorageTimerCompleteFlag = true;
#ifdef ENABLE_BBU_DEBUG_MSGS
    UART_DBG_TX("\n\t ***** BATTERY BACKUP TIME LIMIT REACHED, CHECKING SHELF STORAGE CONDITION *****");
#endif
//    app_lwmesh_send_error("0x1006 Max BBU time reached, Shelf Storage Check");
    // Cut Battery happens in main loop using storage feature
}

static void app_bbu_go_to_sleep(void)
{
    // Radio is already off
//     PuckStatus_t puckStatus;
//     puckStatus.sPuck = app_gen_io_get_puck_status();
// 
//     app_led_update();
// 
//     if ((TETHER_OPENED == puckStatus.noTether) && (SYSTEM_DISARMED == app_arm_is_disarmed()))
//     {
//         app_cutdetect_disable();
//     }

    app_bbu_sleep_on_exit(true);
    app_wdt_disable();
    system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
    system_sleep();
}

static void app_bbu_wakeup(void)
{
    app_wdt_kick();
    app_wdt_enable();
 //   app_lwmesh_wake();

    appBbuState = APP_BBU_STATE_TIRED;
    app_gen_io_set_status_deepSleep(APP_BBU_STATE_SLEEP);

    app_led_update();
}

void app_bbu_request_active(void)
{
    appBbuState = APP_BBU_STATE_ACTIVE;
    app_gen_io_set_status_deepSleep(APP_BBU_STATE_ACTIVE);
 //   app_puckToBaseCom_change_Timeout(UART_PING_TIMEOUT);

    SLP_TimerStop(&appBBUTimeLimitTimer);
    shelfStorageTimerCompleteFlag = false;  // Dont allow shelf storage timeout condition
    app_led_update();
}

void app_bbu_sleep_on_exit(bool sleepOnExit)
{
    if (APP_BBU_STATE_SLEEP == appBbuState)
    {
        if (sleepOnExit)
        {
            SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;
        }
        else
        {
            SCB->SCR &= ~SCB_SCR_SLEEPONEXIT_Msk;
        }
    }
}

void app_bbu_battery_check_ADC_complete_callback(uint16_t bat)
{
//     // Battery voltage has a hardware divide by two, adjust to give actual battery voltage
//     uint16_t tempBatteryLevel = bat * 2;
// 
//     PuckStatus_t puckStatus;
//  //   puckStatus.sPuck = app_gen_io_get_puck_status();
// 
//     if ((tempBatteryLevel > batteryLevel) && !puckStatus.notPowered)
//     {
//         batteryCharging = true;
//     }
//     else
//     {
//         batteryCharging = false;
//     }
// 
//     batteryLevel = tempBatteryLevel;
}

uint16_t app_bbu_get_battery_level(void)
{
    return batteryLevel;
}

bool app_bbu_get_battery_charging(void)
{
    return batteryCharging;
}

void app_bbu_init(void)
{
    appBatteryCheckTimer.interval = 20000;
    appBatteryCheckTimer.mode     = SYS_TIMER_PERIODIC_MODE;
    appBatteryCheckTimer.handler  = appBatteryCheckTimerHandler;
    SYS_TimerStart(&appBatteryCheckTimer);

    appBBUTimeLimitTimer.interval = BBU_TIME_LIMIT;
    appBBUTimeLimitTimer.mode     = SLP_TIMER_INTERVAL_MODE;
    appBBUTimeLimitTimer.handler  = appBBUTimeLimitTimerHandler;
    shelfStorageTimerCompleteFlag = false;

    batteryLevel    = BATTERY_LEVEL_INIT;
    batteryCharging = false;
//    app_adc_configure(BAT_MON_AIN, app_bbu_battery_check_ADC_complete_callback);
    // Note that we've just started a periodic timer.  If the ADC happens to be busy,
    // we'll just lose a reading and catch one the next time the timer fires.
}

bool app_bbu_sleeping(void)
{
    if (appBbuState == APP_BBU_STATE_SLEEP)
    {
        return true;
    }
    return false;
}

void app_bbu_sleep_request(void)
{
#ifdef ENABLE_DEEP_SLEEP_BEEP
    if (appBbuState == APP_BBU_STATE_ACTIVE)
    {
        if (SYSTEM_ARMED == app_arm_is_disarmed())
        {
            app_buzzer_start_pattern(BUZ_PAT_PUCK_DEEP_SLEEP);
        }
    }
#endif
    appBbuState = APP_BBU_STATE_SLEEP;
//    app_gen_io_set_status_deepSleep(APP_BBU_STATE_SLEEP);
}

// void app_bbu_change_main_clock_source(uint8_t clkSrc)
// {
//     static uint8_t currentClkSrc = SYSTEM_CLOCK_SOURCE_DPLL;
// 
//     if (currentClkSrc != clkSrc)
//     {
//         currentClkSrc = clkSrc;
// 
//         struct system_gclk_gen_config gclk_conf;
//         system_gclk_gen_get_config_defaults(&gclk_conf);
//         gclk_conf.source_clock    = clkSrc;
//         gclk_conf.division_factor = CONF_CLOCK_GCLK_0_PRESCALER;
//         gclk_conf.run_in_standby  = CONF_CLOCK_GCLK_0_RUN_IN_STANDBY;
//         gclk_conf.output_enable   = CONF_CLOCK_GCLK_0_OUTPUT_ENABLE;
//         system_gclk_gen_set_config(GCLK_GENERATOR_0, &gclk_conf);
//         system_gclk_gen_enable(GCLK_GENERATOR_0);
// 
//         delay_init();
//     }
// }

void app_bbu_task(void)
{
//     PuckStatus_t puckStatus;
// //    puckStatus.sPuck = app_gen_io_get_puck_status();
// 
//     if (!puckStatus.notPowered)
//     {
//         appBbuState = APP_BBU_STATE_ACTIVE;
//         app_gen_io_set_status_deepSleep(APP_BBU_STATE_ACTIVE);
//     }
//     else
//     {
//         if (batteryLevel < BATTERY_LEVEL_SLEEP)
//         {
//             // <3.8
//             app_bbu_sleep_request();
//         }
//         else if ((batteryLevel < BATTERY_LEVEL_LOW) && (appBbuState == APP_BBU_STATE_TIRED))
//         {
//             // <3.9
//             app_bbu_sleep_request();
//         }
// 
// //         if (BASE_UNPOWERED == app_puckToBase_get_baseNotPowered())
// //         {
// //             SLP_TimerStart(&appBBUTimeLimitTimer);
// //         }
//     }

    switch (appBbuState)
    {
        case APP_BBU_STATE_SLEEP:
            // Stay awake during alarm, sleep buzzer,
            // Stay awake when disarmed so system can shutdown at the correct battery voltage
//             if ((BUZ_PAT_NONE == app_buzzer_pattern_playing()) && (SYSTEM_ARMED == app_arm_is_disarmed()) &&
//                 (!app_arm_is_any_alarm_active() || app_arm_is_silentalarming()) && (app_rfid_is_idle()))
//             {
//                 if (app_lwmesh_sleep())
//                 {
//                     app_lwmesh_manage_clocks(CLK_RELEASE);
//                     app_puckToBaseCom_change_Timeout(UART_PING_TIMEOUT_SLEEP);
// 
// #ifdef ENABLE_BBU_DEBUG_MSGS
//                     UART_DBG_TX("%lu : *** GOING TO SLEEP ***\n", SYS_Timer_Time());
// #endif
// 
//                     app_bbu_go_to_sleep();
// 
//                     // ISR woke up the processor
//                     app_bbu_wakeup();
// 
// #ifdef ENABLE_BBU_DEBUG_MSGS
//                     UART_DBG_TX("%lu : *** WOKE UP ***\n", SYS_Timer_Time());
// #endif
//                 }
//            }
            break;
        default:

            break;
    }
}

bool app_bbu_TimeLimitTimerComplete(void)
{
    return shelfStorageTimerCompleteFlag;
}
