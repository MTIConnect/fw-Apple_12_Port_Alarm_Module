/*
 * app_gen_io.c
 *
 * Created: 10/20/2023 5:00:34 PM
 *  Author: dmcdougall
 */ 


#include <asf.h>
#include "app_gen_io.h"
#include "app_arm.h"
#include "app_bbu.h"
#include "app_buzzer.h"
#include "app_uart.h"
#include "slpTimer.h"
#include "sysTimer.h"


#define BATTERY_LEVEL_STORAGE (3900)
#define BATTERY_LEVEL_DEAD    (3000)

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//      VARIABLES
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

volatile AlarmModuleStatus_t amStatus;
volatile bool ShelfStorageMessageSent;                           // Over debug? 

static SYS_Timer_t debouncePowerGoodTimer;
static SYS_Timer_t debounce_nMASTERTimer; 
static SLP_Timer_t shelfStorageConditionTimer;
static SYS_Timer_t debounceCh_00_SwitchTimer;
static SYS_Timer_t debounceCh_01_SwitchTimer;
static SYS_Timer_t debounceCh_02_SwitchTimer;
static SYS_Timer_t debounceCh_03_SwitchTimer;
static SYS_Timer_t debounceCh_04_SwitchTimer;
static SYS_Timer_t debounceCh_05_SwitchTimer;
static SYS_Timer_t debounceCh_06_SwitchTimer;
static SYS_Timer_t debounceCh_07_SwitchTimer;
static SYS_Timer_t debounceCh_08_SwitchTimer;
static SYS_Timer_t debounceCh_09_SwitchTimer;
static SYS_Timer_t debounceCh_10_SwitchTimer;
static SYS_Timer_t debounceCh_11_SwitchTimer;
static SYS_Timer_t debounce_nDISARM_Timer;


// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//      Prototypes
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

void app_gen_io_init_cables(void);
static void extint_callback_power_good(void);
// static void exting_callback_nMASTER(void);           // Read on boot and power loss, don't need Interrupt
static void extint_callback_debounceCh_00(void);
static void extint_callback_debounceCh_01(void);
static void extint_callback_debounceCh_02(void);
static void extint_callback_debounceCh_03(void);
static void extint_callback_debounceCh_04(void);
static void extint_callback_debounceCh_05(void);
static void extint_callback_debounceCh_06(void);
static void extint_callback_debounceCh_07(void);
static void extint_callback_debounceCh_08(void);
static void extint_callback_debounceCh_09(void);
static void extint_callback_debounceCh_10(void);
static void extint_callback_debounceCh_11(void);
static void extint_callback_debounce_nDISARM(void);


static void shelfStorageConditionTimerHandler(SLP_Timer_t *timer);
static void debouncePowerGoodTimerHandler(SYS_Timer_t *timer);
static void debounce_nMASTERTimerHandler(SYS_Timer_t *timer);
static void debounceCh_00_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_01_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_02_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_03_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_04_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_05_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_06_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_07_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_08_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_09_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_10_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounceCh_11_SwitchTimerHandler(SYS_Timer_t *timer);
static void debounce_nDISARM_TimerHandler(SYS_Timer_t *timer);


struct ChannelStatus_t Channel[CH_COUNT] = 
{
    //   gpio_pin,      gpio_eic_mux,      gpio_eic_line,  timer,                     timerHandler_function,        PortStatus
    {CHANNEL_0_PIN, CHANNEL_0_EIC_MUX, CHANNEL_0_EIC_LINE, &debounceCh_00_SwitchTimer, debounceCh_00_SwitchTimerHandler, 0},
    {CHANNEL_1_PIN, CHANNEL_1_EIC_MUX, CHANNEL_1_EIC_LINE, &debounceCh_01_SwitchTimer, debounceCh_01_SwitchTimerHandler, 0},
    {CHANNEL_2_PIN, CHANNEL_2_EIC_MUX, CHANNEL_2_EIC_LINE, &debounceCh_02_SwitchTimer, debounceCh_02_SwitchTimerHandler, 0},
    {CHANNEL_3_PIN, CHANNEL_3_EIC_MUX, CHANNEL_3_EIC_LINE, &debounceCh_03_SwitchTimer, debounceCh_03_SwitchTimerHandler, 0},
    {CHANNEL_4_PIN, CHANNEL_4_EIC_MUX, CHANNEL_4_EIC_LINE, &debounceCh_04_SwitchTimer, debounceCh_04_SwitchTimerHandler, 0},
    {CHANNEL_5_PIN, CHANNEL_5_EIC_MUX, CHANNEL_5_EIC_LINE, &debounceCh_05_SwitchTimer, debounceCh_05_SwitchTimerHandler, 0},
    {CHANNEL_6_PIN, CHANNEL_6_EIC_MUX, CHANNEL_6_EIC_LINE, &debounceCh_06_SwitchTimer, debounceCh_06_SwitchTimerHandler, 0},
    {CHANNEL_7_PIN, CHANNEL_7_EIC_MUX, CHANNEL_7_EIC_LINE, &debounceCh_07_SwitchTimer, debounceCh_07_SwitchTimerHandler, 0},
    {CHANNEL_8_PIN, CHANNEL_8_EIC_MUX, CHANNEL_8_EIC_LINE, &debounceCh_08_SwitchTimer, debounceCh_08_SwitchTimerHandler, 0},
    {CHANNEL_9_PIN, CHANNEL_9_EIC_MUX, CHANNEL_9_EIC_LINE, &debounceCh_09_SwitchTimer, debounceCh_09_SwitchTimerHandler, 0},
    {CHANNEL_10_PIN, CHANNEL_10_EIC_MUX, CHANNEL_10_EIC_LINE, &debounceCh_10_SwitchTimer, debounceCh_10_SwitchTimerHandler, 0},
    {CHANNEL_11_PIN, CHANNEL_11_EIC_MUX, CHANNEL_11_EIC_LINE, &debounceCh_11_SwitchTimer, debounceCh_11_SwitchTimerHandler, 0},
};

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//      SETUP FUNCTIONS
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

void app_gen_io_init(void)
{
    struct port_config pin_conf;
    port_get_config_defaults(&pin_conf);
    
    // Set BATT DEADMAN SW PIN (KILL switch) to Output
    pin_conf.direction = PORT_PIN_DIR_OUTPUT;
    port_pin_set_config(BATT_DEADMAN_SW_PIN, &pin_conf);
    port_pin_set_output_level(BATT_DEADMAN_SW_PIN, BATT_DEADMAN_STAY_POWERED);
    
    port_get_config_defaults(&pin_conf);                            // Re-fetch defaults
    
    app_gen_io_init_cables(); // Configure cables as Input Pull-None and read logic level to determine cable status
    
    struct extint_chan_conf config_extint_chan;
    extint_chan_get_config_defaults(&config_extint_chan);

    debouncePowerGoodTimer.interval = STANDARD_DEBOUNCE_INTERVAL_MS;  // delay
    debouncePowerGoodTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    debouncePowerGoodTimer.handler  = debouncePowerGoodTimerHandler;
    
    debounce_nMASTERTimer.interval = STANDARD_DEBOUNCE_INTERVAL_MS;  // delay
    debounce_nMASTERTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    debounce_nMASTERTimer.handler  = debounce_nMASTERTimerHandler;
    
    shelfStorageConditionTimer.interval = SHELF_STORAGE_MESSAGE_INTERVAL_MS;  // delay
    shelfStorageConditionTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    shelfStorageConditionTimer.handler  = shelfStorageConditionTimerHandler;
    
    debounce_nDISARM_Timer.interval = STANDARD_DEBOUNCE_INTERVAL_MS;  // delay
    debounce_nDISARM_Timer.mode     = SYS_TIMER_INTERVAL_MODE;
    debounce_nDISARM_Timer.handler  = debounce_nDISARM_TimerHandler;

    for(int num; num < CH_COUNT; num++)
    {
        Channel[num].timer->interval = STANDARD_DEBOUNCE_INTERVAL_MS;  // delay
        Channel[num].timer->mode     = SYS_TIMER_INTERVAL_MODE;
        Channel[num].timer->handler  = Channel[num].timerHandler;
    }

    // Setup Power Good input. Be sure NOT to have the pull-up enabled,
    // as it's stronger than the lower resistor in the divider.
    config_extint_chan.gpio_pin           = POWER_GOOD_EIC_PIN;
    config_extint_chan.gpio_pin_mux       = POWER_GOOD_EIC_MUX;
    config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;  // <<<===
    config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
    extint_chan_set_config(POWER_GOOD_EIC_LINE, &config_extint_chan);

    config_extint_chan.gpio_pin           = nDISARM_PIN;
    config_extint_chan.gpio_pin_mux       = nDISARM_EIC_MUX;
    config_extint_chan.gpio_pin_pull      = EXTINT_PULL_NONE;  // <<<===
    config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
    extint_chan_set_config(nDISARM_EIC_LINE, &config_extint_chan);

    for(int num = 0; num < CH_COUNT; num++)
    {
        config_extint_chan.gpio_pin           = Channel[num].gpio_pin;
        config_extint_chan.gpio_pin_mux       = Channel[num].gpio_eic_mux;
        config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;
        config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
        extint_chan_set_config(Channel[num].gpio_eic_line, &config_extint_chan);
    }

    pin_conf.direction  = PORT_PIN_DIR_INPUT;
    pin_conf.input_pull = PORT_PIN_PULL_UP;
    port_pin_set_config(POWER_GOOD_PIN, &pin_conf);     // Input Pull UP
    port_pin_set_config(nMASTER_PIN, &pin_conf);        // Input Pull-UP
    
    pin_conf.input_pull = PORT_PIN_PULL_NONE;
    port_pin_set_config(BAT_MON_PIN, &pin_conf);        // Input Pull-None
   // port_pin_set_config(nDISARM_PIN, &pin_conf);   // Input Pull-None
    
    // Delay to allow hardware debounce components to stabilize after configuration
    delay_cycles_ms(100);
    
    if (port_pin_get_input_level(POWER_GOOD_PIN))  // High = Power Good, Low = No Power
    {
        amStatus.notCharging = false;
        amStatus.Powered  = POWER_GOOD;
    }
    else
    {
        amStatus.notCharging = true;  // Charging refers to MTI product internal battery, not device being secured.
        amStatus.Powered  = POWER_NOT_GOOD;
    }
    
    // Setup default Status'
    amStatus.deepSleep = false;
    amStatus.shutDown  = false;
    amStatus.switchLifted    = false;
        
    if( port_pin_get_input_level(nMASTER_PIN) )
    {
        // HIGH - Not Master AM - This unit will be a slave AM
        amStatus.isMaster = AM_NOT_MASTER;
        app_arm_set_PowerTamper_armed(SYSTEM_DISARMED);
        app_arm_clear_PowerTamper_alarm();
    }
    else
    {
        // LOW - IS Master AM - This unit will be in charge of the daisy chain & Power Tamper Alarm
        amStatus.isMaster = AM_IS_MASTER;
        app_arm_set_PowerTamper_armed(SYSTEM_ARMED);
        app_arm_clear_PowerTamper_alarm();
    }

    extint_register_callback(extint_callback_power_good, POWER_GOOD_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_chan_enable_callback(POWER_GOOD_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    
    extint_register_callback(extint_callback_debounce_nDISARM, nDISARM_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_chan_enable_callback(nDISARM_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    
    // Setup Cable interrupts
    extint_register_callback(extint_callback_debounceCh_00, CHANNEL_0_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_01, CHANNEL_1_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_02, CHANNEL_2_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_03, CHANNEL_3_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_04, CHANNEL_4_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_05, CHANNEL_5_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_06, CHANNEL_6_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_07, CHANNEL_7_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_08, CHANNEL_8_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_09, CHANNEL_9_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_10, CHANNEL_10_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_register_callback(extint_callback_debounceCh_11, CHANNEL_11_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    
    for (int num; num < CH_COUNT; num++)
    {
        extint_chan_enable_callback(Channel[num].gpio_eic_line, EXTINT_CALLBACK_TYPE_DETECT);  
    }
    
    // Set up analog input pins
    pin_conf.input_pull = PORT_PIN_PULL_NONE;
    port_pin_set_config(BAT_MON_PIN, &pin_conf);
    
    ShelfStorageMessageSent = false;
    
}


void app_gen_io_init_cables(void)
{
    struct port_config pin_conf;
    port_get_config_defaults(&pin_conf);
    pin_conf.input_pull = PORT_PIN_PULL_NONE;
    
    for(int num; num < CH_COUNT; num++)
    {
        port_pin_set_config(Channel[num].gpio_pin, &pin_conf);  // Inputs with no pull
    }        
    
    delay_cycles_ms(100);                                       // Allow pins to stabilize
    
    for(int num; num < CH_COUNT; num++)
    {
        // Read value and assign
        if(port_pin_get_input_level(Channel[num].gpio_pin) == LOW)
        {
            Channel[num].portStat.cablePresent = CABLE_PRESENT;
        }
        else
        {
            Channel[num].portStat.cablePresent = CABLE_ABSENT; 
        }
        
        Channel[num].portStat.armed = PORT_DISARMED;
        Channel[num].portStat.alarming = PORT_NOT_ALARMING;
    }   
}

void app_gen_io_kill_switch_task(void)
{
    //     if (puckStatus.notPowered)
    //     {
    //         if (SYS_Timer_Time() > 60000)  // Booted for at least 60 seconds
    //         {
    //             if ((SYSTEM_DISARMED == app_arm_is_disarmed()) ||
    //             (app_bbu_TimeLimitTimerComplete() && (BASE_UNPOWERED == app_puckToBase_get_baseNotPowered())))
    //             {
    //                 // If we're disarmed, we turn off with the battery at a good
    //                 // voltage for long-term storage.
    //                 uint16_t temp_batlevel = app_bbu_get_battery_level();
    //
    //                 if (temp_batlevel < BATTERY_LEVEL_STORAGE)
    //                 {
    //                     UART_TX("\napp_bbu_get_battery_level reading is: %d\n", temp_batlevel);
    //                     // 2 seconds to send lwmesh message, we've just passed app_bbu_task()
    //                     // so we should make it through the main loop again, if power is restored
    //                     // before timer expires then status is updated and timer stops. This is
    //                     // handled in
    //                     puckStatus.shutDown = true;
    //
    //                     if ((!SLP_TimerStarted(&shelfStorageConditionTimer)) && (ShelfStorageMessageSent == false))
    //                     {
    //                         // Send the lwmesh message only once
    //                         ShelfStorageMessageSent = true;
    //                         app_lwmesh_send_status();
    //                     }
    //
    //                     SLP_TimerStart(&shelfStorageConditionTimer);
    //                 }
    //             }
    //         }
    //     }
}

////////////////////////////////////////////////////////////////
void app_gen_io_set_status_deepSleep(uint16_t sleepState)
{
    
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//      Callbacks
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

static void extint_callback_power_good(void)  // Power Good indicator - now just a voltage divider
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debouncePowerGoodTimer);
    amStatus.Powered = POWER_NOT_GOOD;
}


static void extint_callback_debounceCh_00(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_00_SwitchTimer);
}


static void extint_callback_debounceCh_01(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_01_SwitchTimer);
}


static void extint_callback_debounceCh_02(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_02_SwitchTimer);
}


static void extint_callback_debounceCh_03(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_03_SwitchTimer);
}


static void extint_callback_debounceCh_04(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_04_SwitchTimer);
}


static void extint_callback_debounceCh_05(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_05_SwitchTimer);
}


static void extint_callback_debounceCh_06(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_06_SwitchTimer);
}


static void extint_callback_debounceCh_07(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_07_SwitchTimer);
}


static void extint_callback_debounceCh_08(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_08_SwitchTimer);
}


static void extint_callback_debounceCh_09(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_09_SwitchTimer);
}


static void extint_callback_debounceCh_10(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_10_SwitchTimer);
}


static void extint_callback_debounceCh_11(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounceCh_11_SwitchTimer);
}


static void extint_callback_debounce_nDISARM(void)
{
    app_bbu_sleep_on_exit(false);
    SYS_TimerRestart(&debounce_nDISARM_Timer);
}

// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------
//      Timer Handlers
// ----------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------

static void shelfStorageConditionTimerHandler(SLP_Timer_t *timer)
{
    UNUSED(timer);
    #ifdef ENABLE_GENIO_DEBUG_MSGS
    UART_DBG_TX("Set Kill to LOW\n");
    #endif
    port_pin_set_output_level(BATT_DEADMAN_SW_PIN, BATT_DEADMAN_POWER_DOWN);
}



static void debouncePowerGoodTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    if (port_pin_get_input_level(POWER_GOOD_PIN))  // Low = No Power, High = Power Good
    {
        amStatus.Powered = POWER_GOOD;
        amStatus.shutDown   = false;
        SLP_TimerStop(&shelfStorageConditionTimer);  // Don't issue kill command
        app_arm_reset_auto_arm_timer();
        app_buzzer_stop_pattern(BUZ_PAT_PUCK_DEEP_SLEEP);
        
        if( app_arm_only_powerTamper_alarming() )
        {
            // Only the master can cause this alarm to trigger, so checking it's the only active alarm also checks this
            app_arm_set_PowerTamper_armed(SYSTEM_ARMED);
            app_buzzer_alarm_stop();
        }
        
        app_arm_clear_PowerTamper_alarm();

        ShelfStorageMessageSent = false;
        app_bbu_request_active();
    }
    else
    {
        amStatus.Powered = POWER_NOT_GOOD;
        if( (AM_IS_MASTER == amStatus.isMaster) && (SYSTEM_ARMED == app_arm_get_system_armed() ) )
        {
            // Power Tamper Alarm is only on the Master Unit & Only if a Channel is armed
            // Check nMASTER if its gone high with the power loss we might alarm.  
            SYS_TimerStart(&debounce_nMASTERTimer);   
        }
    }

    //app_led_update();
}


static void debounce_nMASTERTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    // To get here we are the Master AM have seen a loss of power and waited 2 debounce time units
   if( port_pin_get_input_level(nMASTER_PIN) )
   {
       app_arm_alarmEvent(POWER_TAMPER_nMASTER_ALARM);  // Alarm for 5 minutes, unless power is restored
   } 
}


static void debounceCh_00_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_0_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_00].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 0 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_00].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_00].portStat.alarming))
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_0_SWITCH_WAS_OPENED);
            Channel[CH_00].portStat.alarming = PORT_ALARMING;
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_00].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 0 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_01_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_1_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_01].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 1 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_01].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_01].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_1_SWITCH_WAS_OPENED);
            Channel[CH_01].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_01].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 1 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_02_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_2_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_02].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 2 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_02].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_02].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_2_SWITCH_WAS_OPENED);
            Channel[CH_02].portStat.alarming = PORT_ALARMING;
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_02].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 2 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_03_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_3_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_03].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 3 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_03].portStat.armed)  &&\
             (PORT_NOT_ALARMING == Channel[CH_03].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_3_SWITCH_WAS_OPENED);
            Channel[CH_03].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_03].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 3 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_04_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_4_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_04].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 4 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_04].portStat.armed)  &&\
             (PORT_NOT_ALARMING == Channel[CH_04].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_4_SWITCH_WAS_OPENED);
            Channel[CH_04].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_04].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 4 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_05_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_5_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_05].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 5 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_05].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_05].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_5_SWITCH_WAS_OPENED);
            Channel[CH_05].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_05].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 5 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_06_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_6_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_06].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 6 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_06].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_06].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_6_SWITCH_WAS_OPENED);
            Channel[CH_06].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_06].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 6 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_07_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_7_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_07].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 7 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_07].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_07].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_7_SWITCH_WAS_OPENED);
            Channel[CH_07].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_07].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 7 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_08_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_8_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_08].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 8 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_08].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_08].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_8_SWITCH_WAS_OPENED);
            Channel[CH_08].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_08].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 8 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}



static void debounceCh_09_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_9_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_09].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 9 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_09].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_09].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_9_SWITCH_WAS_OPENED);
            Channel[CH_09].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_09].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 9 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}


static void debounceCh_10_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_10_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_10].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 10 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_10].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_10].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_10_SWITCH_WAS_OPENED);
            Channel[CH_10].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_10].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 10 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}


static void debounceCh_11_SwitchTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    if (port_pin_get_input_level(CHANNEL_11_PIN))
    {
        // Primary Switch - High = Open
        Channel[CH_11].portStat.cablePresent = CABLE_ABSENT;
        UART_DBG_TX("\n CHANNEL 11 SWITCH OPENED\n");
        
        if ( (PORT_ARMED == Channel[CH_11].portStat.armed) &&\
             (PORT_NOT_ALARMING == Channel[CH_11].portStat.alarming) )
        {
            // Switch was closed but has opened
            app_arm_alarmEvent(CHANNEL_11_SWITCH_WAS_OPENED);
            Channel[CH_11].portStat.alarming = PORT_ALARMING; 
        }
    }
    else
    {
        // Primary Switch - Low = Closed
        // Cable Switch was open or absent, but has closed
        Channel[CH_11].portStat.cablePresent = CABLE_PRESENT;
        UART_DBG_TX("\n CHANNEL 11 SWITCH CLOSED\n");
        
        app_arm_reset_auto_arm_timer();     
    }
}


static void debounce_nDISARM_TimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    // Driving this signal low disarms the unit.  
    if( port_pin_get_input_level(nDISARM_PIN) == ARM_CMD )
    {
        // intelli-key removed - arm attempt should be made
        // auto arm after 1 minute is CM workflow
        // app_arm_reset_auto_arm_timer(); 
        app_arm_request(false, ARM_IGNORE_NONE);
        UART_DBG_TX("nDISARM: Arm Requested");
    }
    else
    {
        // intelli-key inserted - remain disarmed
        app_arm_disarm(0);
        UART_DBG_TX("nDISARM: DISARM!");
    }
    
}

// LOW = REMAIN DISARMED
// HIGH = CAN ARM
bool app_gen_io_get_nDISARM(void)
{
    return port_pin_get_input_level(nDISARM_PIN);
}

////////////////////////////////////////////////////////////////
// This function returns the UNIT status bits
uint16_t app_gen_io_get_AM_status(void)
{
    return amStatus.sAlarmModule;
}


////////////////////////////////////////////////////////////////
uint16_t app_gen_io_get_Channel_Status(uint16_t num)
{
    return Channel[num].portStat.sPort;
}


////////////////////////////////////////////////////////////////
bool app_gen_io_is_power_good(void)
{
    if( !SYS_TimerStarted(&debouncePowerGoodTimer) )
    {
        // The de-bounce timer is not running. Check to see if we need to start it.
        if(port_pin_get_input_level(POWER_GOOD_PIN)) // Low = No Power, High = Power Good
        {
            if( POWER_NOT_GOOD == amStatus.Powered )
            {
                // The de-bounced status and the pin don't agree. Start the debounce timer.
                SYS_TimerStart(&debouncePowerGoodTimer);
            }
        }
        else
        {
            if( POWER_NOT_GOOD == amStatus.Powered )
            {
                // The de-bounced status and the pin don't agree. Start the debounce timer.
                SYS_TimerStart(&debouncePowerGoodTimer);
            }
        }
    }
    
    // Regardless of whether we (re)start the timer, we use the de-bounced value.
    return (amStatus.Powered);
}


////////////////////////////////////////////////////////////////
bool app_gen_io_is_cable_present(uint8_t portNum)
{
    if(portNum >= CH_COUNT)
    {
        // Invalid Channel
        return false;
    }
    
    return (Channel[portNum].portStat.cablePresent);
}

////////////////////////////////////////////////////////////////
bool app_gen_io_is_port_armed(uint8_t portNum)
{
    if(portNum >= CH_COUNT)
    {
        // Invalid Channel
        return false;
    }
    
    return (Channel[portNum].portStat.armed);
}


////////////////////////////////////////////////////////////////
void app_gen_io_set_port_armed(uint8_t portNum, bool desiredArmState)
{
    if(portNum >= CH_COUNT)
    {
        return;
    }
    
    Channel[portNum].portStat.alarming = DIDNT_ALARM;               // Only true when actually alarming or silent alarming 
    
    if( desiredArmState )
    {
        Channel[portNum].portStat.armed = SENSOR_ARMED; 
        Channel[portNum].portStat.cablePresent = CABLE_PRESENT; 
    }
    else
    {
        Channel[portNum].portStat.armed = SENSOR_DISARMED;
    }
}

////////////////////////////////////////////////////////////////
bool app_gen_io_is_any_port_armed(void)
{
    bool portArmed = PORT_DISARMED; 
    
    for (int num = 0 ; num < CH_COUNT ; num++ )
    {
        if(PORT_ARMED == Channel[num].portStat.armed)
        {
           portArmed = PORT_ARMED;
           continue;
        }
    }

    return portArmed;
}

bool app_gen_io_is_port_alarming(uint8_t portNum)
{
    return Channel[portNum].portStat.alarming;
}


void app_gen_io_multiport_blink(uint8_t multiportFlashCounter)
{
    for(int num = 0 ; num < CH_COUNT ; num++)
    {
        if(PORT_DISARMED == Channel[num].portStat.armed)
        {
            if((multiportFlashCounter == 5) || (multiportFlashCounter == 7))
            {
                // Reconfigure pin to be output
                // Turn off Disarmed Channel's LED
                
            }  
            else
            {
                // Turn on Disarmed Channel's LED
                // Change to input & measure level to see if a cable is plugged in
                // If there is a cable plugged in Call Debounce function
            }              
        }
    }  
}

void app_gen_io_multiport_reconfigIO(uint8_t gpioDirection)
{
    struct port_config pin_conf;
    port_get_config_defaults(&pin_conf);
}