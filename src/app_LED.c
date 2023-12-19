/*
 * app_LED.c
 *
 * Created: 10/20/2023 5:07:06 PM
 *  Author: dmcdougall
 */ 

#include <asf.h>
#include "app_LED.h"
#include "app_arm.h"
#include "app_bbu.h"
#include "app_eeprom.h"
#include "app_gen_io.h"
#include "conf_board.h"
#include "sysTimer.h"

////////////////////////////////////////////////////////////////
// Local Variables
static SYS_Timer_t appLedFlashTimer;
static SYS_Timer_t appLedDurationTimer;
static SYS_Timer_t appLedDisarmedFlashTimer;
static SYS_Timer_t appArmedUnpoweredFlashTimer;

static uint16_t appLedDuration;
static uint8_t appLedColor;
static uint8_t appLedMode;

static uint8_t appLedState;

////////////////////////////////////////////////////////////////
// Enums

enum
{
    APP_LED_STATE_IDLE,
    APP_LED_STATE_ALARM,
    APP_LED_STATE_ALARM_EXTERNAL,
    APP_LED_STATE_IDENTIFYING,
};

////////////////////////////////////////////////////////////////
// Functions

static void appLedFlashTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    static bool toggler = false;
    toggler             = !toggler;

//     switch (appLedColor)
//     {
//         case APP_LED_BLUE:
//         case APP_LED_BLUE_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_BLUE_PIN);
//             break;
//         case APP_LED_GREEN:
//         case APP_LED_GREEN_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_GREEN_PIN);
//             break;
//         case APP_LED_RED:
//         case APP_LED_RED_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_RED_PIN);
//             break;
//         case APP_LED_CYAN:
//         case APP_LED_CYAN_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_BLUE_PIN);
//             app_ext_gpio_toggle_cache(LED_GREEN_PIN);
//             break;
//         case APP_LED_PURPLE:
//         case APP_LED_PURPLE_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_BLUE_PIN);
//             app_ext_gpio_toggle_cache(LED_RED_PIN);
//             break;
//         case APP_LED_YELLOW:
//         case APP_LED_GREEN_RED_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_GREEN_PIN);
//             app_ext_gpio_toggle_cache(LED_RED_PIN);
//             break;
//         case APP_LED_WHITE:
//         case APP_LED_WHITE_EXTERNAL:
//             app_ext_gpio_toggle_cache(LED_WHITE_PIN);
//             break;
//         case APP_LED_ALARM:
//         case APP_LED_ALARM_EXTERNAL:
//             // Toggle between white and red or Yellow and Red
//             if (toggler)
//             {
//                 if (!app_eeprom_read_connect_wanted() || (app_lwmesh_is_comm_working()))
//                 {
//                     // Toggle between White and Red if network is up or in non-connect mode
//                     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//                     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//                     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//                     app_ext_gpio_set_cache(LED_WHITE_PIN, LED_ON);
//                 }
//                 else
//                 {
//                     // Toggle between Yellow and Red if network is down
//                     app_ext_gpio_set_cache(LED_RED_PIN, LED_ON);
//                     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_ON);
//                     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//                     app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);
//                 }
//             }
//             else
//             {
//                 // turn ON red alone, and OFF green & blue
//                 app_ext_gpio_set_cache(LED_RED_PIN, LED_ON);
//                 app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//                 app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//                 app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);
//             }
// 
//             break;
//     }
//     // Send all the updated LED bits out to the shift register.
//     app_ext_gpio_update();
}

static void appLedDurationTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    SYS_TimerStop(&appLedFlashTimer);
    appLedDuration = 0;
//     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
// 
//     if (APP_LED_STATE_IDENTIFYING == appLedState)
//     {
//         appLedState = APP_LED_STATE_IDLE;
//         app_lwmesh_send_status();
//     }

    app_led_update();
}

static void appArmedUnpoweredFlashTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

//     if (app_eeprom_read_connect_wanted() && (!app_lwmesh_is_comm_working()))
//     {
//         app_led_control(1, APP_LED_YELLOW_EXTERNAL, 2);
//     }
//     else
//     {
//         app_led_control(1, APP_LED_WHITE_EXTERNAL, 2);
//     }
}

static void appLedDisarmedFlashTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
#ifdef APP_ENABLE_CW_TEST_MODE
    app_led_control(1, APP_LED_PURPLE, 4);
#else
//     if (app_eeprom_read_connect_wanted() && (!app_lwmesh_is_comm_working()))
//     {
//         // If we WANT Connect, but the network is down, we use yellow.
//         app_led_control(1, APP_LED_YELLOW_EXTERNAL, 4);
//     }
//     else
//     {
//         app_led_control(1, APP_LED_WHITE_EXTERNAL, 4);
//     }
#endif
}

////////////////////////////////////////////////////////////////
//
// app_led_control()
// iDuration in seconds, APP_LED_DURATION_FOREVER (0xFFFF) = forever
// iColor = from a table of ENUMs
// iMode  = flashes per second, with 0 = steady on FOREVER
//
// Connect calls into this for the IDENTIFY function.
// It only has two hex digits for the duration (0x00-0xFF),
// and one each for color and mode.
// Within the product, we can use a wider range of values.
//
void app_led_control(uint16_t iDuration, uint8_t iColor, uint8_t iMode)
{
//     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);

    SYS_TimerStop(&appLedFlashTimer);
    SYS_TimerStop(&appLedDurationTimer);
    SYS_TimerStop(&appLedDisarmedFlashTimer);

    appLedDuration = iDuration;
    appLedColor    = iColor;
    appLedMode     = iMode;

    if (appLedDuration != 0)
    {
        if (appLedMode != 0)
        {
            switch (appLedColor)
            {
//                 case APP_LED_BLUE:
//                     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_ON);
//                     break;
//                 case APP_LED_GREEN:
//                     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_ON);
//                     break;
//                 case APP_LED_RED:
//                 case APP_LED_ALARM:
//                 case APP_LED_ALARM_EXTERNAL:  // Setup the same as normal alarm
//                     app_ext_gpio_set_cache(LED_RED_PIN, LED_ON);
//                     break;
             }

            appLedFlashTimer.interval = 1000 / appLedMode;
            appLedFlashTimer.mode     = SYS_TIMER_PERIODIC_MODE;
            appLedFlashTimer.handler  = appLedFlashTimerHandler;
            SYS_TimerStart(&appLedFlashTimer);

            if (APP_LED_DURATION_FOREVER != appLedDuration)
            {
                // We do NOT set up the duration timer for duration of 0xFFFF.
                // That's an indication that we want to keep going forever.
                appLedDurationTimer.interval = appLedDuration * 1000;
                appLedDurationTimer.mode     = SYS_TIMER_INTERVAL_MODE;
                appLedDurationTimer.handler  = appLedDurationTimerHandler;
                SYS_TimerStart(&appLedDurationTimer);
            }
        }
        else
        {
            // Solid LED
            appLedFlashTimerHandler(NULL);
        }
    }
    else
    {
        // 0 duration
        app_led_update();
    }

//    app_ext_gpio_update();
}

void app_LED_init(void)
{
    // LED IO are configured in app_shift_register_init() or app_ext_gpio_init() depending on project
    // Turn off LEDs
//     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);

    appLedDisarmedFlashTimer.interval = 5000;
    appLedDisarmedFlashTimer.mode     = SYS_TIMER_PERIODIC_MODE;
    appLedDisarmedFlashTimer.handler  = appLedDisarmedFlashTimerHandler;

    appArmedUnpoweredFlashTimer.interval = 8000;
    appArmedUnpoweredFlashTimer.mode     = SYS_TIMER_PERIODIC_MODE;
    appArmedUnpoweredFlashTimer.handler  = appArmedUnpoweredFlashTimerHandler;
}

void app_led_update(void)
{
//    PuckStatus_t puckStatus;
//     puckStatus.sPuck = app_gen_io_get_puck_status();
// 
//     // Generally, we'll stop this timer, as it only runs during the disabled state.
//     SYS_TimerStop(&appLedDisarmedFlashTimer);
//     SYS_TimerStop(&appArmedUnpoweredFlashTimer);
// 
//     if (app_arm_is_any_alarm_active())
//     {
//         app_led_control(0xff, APP_LED_ALARM_EXTERNAL, 4);
//     }
//     else if (puckStatus.notPowered)
//     {
//         // No power, turn off ALL the LEDs.
//         app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//         app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//         app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//         app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);
// 
//         SYS_TimerStop(&appLedFlashTimer);
//         SYS_TimerStop(&appLedDurationTimer);
//         if (SYSTEM_ARMED == app_arm_is_disarmed())
//         {
//             // But if we're armed, we have a once-per-eight-seconds flash to maintain.
//             SYS_TimerRestart(&appArmedUnpoweredFlashTimer);
//         }
//     }
//     else if (app_gen_io_is_usb_over_current())
//     {
//         app_led_control(1, APP_LED_BLUE, 0);
//     }
//     else if (app_join_get_provisioning())
//     {
//         app_led_control(0xFF, APP_LED_BLUE, 2);
//     }
//     else if (appLedState == APP_LED_STATE_IDENTIFYING)
//     {
//         // Just continue with existing identifying LED pattern
//     }
//     else if (SYSTEM_ARMED == app_arm_is_disarmed())
//     {
//         if ((APP_EEPROM_MODEL_TYPE_CONNECTED == app_eeprom_read_connect_wanted()) && (!app_lwmesh_is_comm_working()))
//         {
//             // If we WANT to connect, but the network is down, we use yellow.
//             app_led_control(1, APP_LED_YELLOW_EXTERNAL, 0);
//         }
//         else
//         {
//             app_led_control(1, APP_LED_WHITE_EXTERNAL, 0);
//         }
//     }
//     else if (RFID_STATE_DISARMED != app_rfid_state_get())
//     {
//         app_rfid_state_set_LEDs();
//     }
//     else
//     {
//         SYS_TimerStop(&appLedFlashTimer);
//         SYS_TimerStop(&appLedDurationTimer);
// 
//         // Turn off all LEDs as the base state...pho
//         app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//         app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//         app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//         app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);
// 
//         // And start the special timer for the disarmed flash.
//         SYS_TimerRestart(&appLedDisarmedFlashTimer);
//     }
//     app_ext_gpio_update();
}

void app_LED_emit_RGBCause_pattern(void)
{
    // Start sequence
//     app_ext_gpio_set_cache(LED_RED_PIN, LED_ON);
//     app_ext_gpio_update();
//     delay_cycles_ms(500);
// 
//     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_ON);
//     app_ext_gpio_update();
//     delay_cycles_ms(500);
// 
//     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_ON);
//     app_ext_gpio_update();
//     delay_cycles_ms(500);
// 
//     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//     app_ext_gpio_update();
// 
//     switch (system_get_reset_cause())
//     {
//         case SYSTEM_RESET_CAUSE_SOFTWARE:
//             app_ext_gpio_set_cache(LED_RED_PIN, LED_ON);
//             break;
//         case SYSTEM_RESET_CAUSE_WDT:
//             app_ext_gpio_set_cache(LED_BLUE_PIN, LED_ON);
//             break;
//         case SYSTEM_RESET_CAUSE_EXTERNAL_RESET:
//             app_ext_gpio_set_cache(LED_GREEN_PIN, LED_ON);
//             break;
//         case SYSTEM_RESET_CAUSE_BOD33:
//             app_ext_gpio_set_cache(LED_BLUE_PIN, LED_ON);
//             app_ext_gpio_set_cache(LED_GREEN_PIN, LED_ON);
//             break;
//         case SYSTEM_RESET_CAUSE_BOD12:
//             app_ext_gpio_set_cache(LED_GREEN_PIN, LED_ON);
//             app_ext_gpio_set_cache(LED_RED_PIN, LED_ON);
//             break;
//         case SYSTEM_RESET_CAUSE_POR:
//             app_ext_gpio_set_cache(LED_WHITE_PIN, LED_ON);
//             break;
//     }
//     app_ext_gpio_update();
//     delay_cycles_ms(1000);
// 
//     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
//     app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);
//     app_ext_gpio_update();
    delay_cycles_ms(1000);
}

void app_led_identify(uint16_t iDuration, uint8_t iColor, uint8_t iMode)
{
    if (iDuration > 0)
    {
        appLedState = APP_LED_STATE_IDENTIFYING;
    }
    else
    {
        appLedState = APP_LED_STATE_IDLE;
    }
    app_led_control(iDuration, iColor, iMode);
}

bool app_led_is_identifying(void)
{
    return (appLedState == APP_LED_STATE_IDENTIFYING);
}


// *****************************************************************************

static SYS_Timer_t appLedDisarmedMultiportFlashTimer;
static void appLedDisarmedMultiportFlashTimerHandler(SYS_Timer_t *timer)
uint8_t multiportFlashCounter;

void app_LED_multiport_init(void)
{
    // LED IO are configured in app_shift_register_init() or app_ext_gpio_init() depending on project
    // Turn off LEDs
    //     app_ext_gpio_set_cache(LED_RED_PIN, LED_OFF);
    //     app_ext_gpio_set_cache(LED_GREEN_PIN, LED_OFF);
    //     app_ext_gpio_set_cache(LED_BLUE_PIN, LED_OFF);
    //     app_ext_gpio_set_cache(LED_WHITE_PIN, LED_OFF);

    appLedDisarmedMultiportFlashTimer.interval = 250;
    appLedDisarmedMultiportFlashTimer.mode     = SYS_TIMER_PERIODIC_MODE;
    appLedDisarmedMultiportFlashTimer.handler  = appLedDisarmedMultiportFlashTimerHandler;
    
    multiportFlashCounter = 0;
    
    SYS_TimerStart(&appLedDisarmedMultiportFlashTimer);
}

static void appLedDisarmedMultiportFlashTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    multiportFlashCounter = multiportFlashCounter + 1;
    if(multiportFlashCounter >= 8)
    {
        multiportFlashCounter = 0;
    }
    
    app_gen_io_multiport_blink(multiportFlashCounter);
}
