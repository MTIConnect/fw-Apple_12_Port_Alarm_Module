/*
 * app_daisychain.c
 *
 * Created: 11/16/2023 3:46:09 PM
 *  Author: dmcdougall
 */ 


#include <asf.h>
#include "app_arm.h"
#include "app_daisychain.h"
#include "app_gen_io.h"
#include "app_uart.h"
#include "sysTimer.h"


#define DAISY_CHAIN_ALARM_TIME          3000    // Milli-seconds till alarm
#define DAISY_CHAIN_PULSE_TIME          500     // Milli-seconds till pulse


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//      PROTOTYPES & Variables
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

AlarmModuleStatus_t alarmModuleStatus;

static SYS_Timer_t daisyChainDebounceTimer;
static SYS_Timer_t daisyChainCountdownTimer;

static void daisyChainDebounceTimerHandler(SYS_Timer_t *timer);
static void daisyChainCountdownTimerHandler(SYS_Timer_t *timer);
static void extint_callback_debounce_daisyChain(void);


// --------------------------------------------------------------------------
// --------------------------------------------------------------------------
//      FUNCTIONS
// --------------------------------------------------------------------------
// --------------------------------------------------------------------------

void app_daisychain_init(void)
{

     alarmModuleStatus.sAlarmModule = app_gen_io_get_AM_status(); 
     
     
     if(AM_IS_MASTER == alarmModuleStatus.isMaster)
     {
         // We're the master and generate the heartbeat on the daisychain, ignore daisy chain alarms. 
         
         struct port_config pin_conf;
         port_get_config_defaults(&pin_conf);
         
         pin_conf.direction = PORT_PIN_DIR_OUTPUT;
         port_pin_set_config(ARM_PIN, &pin_conf);
         port_pin_set_output_level(ARM_PIN, LOW);
         
         daisyChainCountdownTimer.interval = DAISY_CHAIN_PULSE_TIME;            // delay
     }
     else
     {daisyChainCountdownTimer.interval = DAISY_CHAIN_PULSE_TIME;            // delay
        // We're a Slave on the daisy chain and listening for a heartbeat
        // if a channel is armed and we loose heartbeat we alarm.
         
        struct extint_chan_conf config_extint_chan;
        extint_chan_get_config_defaults(&config_extint_chan);
        
        daisyChainDebounceTimer.interval = STANDARD_DEBOUNCE_INTERVAL_MS;        // delay
        daisyChainDebounceTimer.mode     = SYS_TIMER_INTERVAL_MODE;
        daisyChainDebounceTimer.handler  = daisyChainDebounceTimerHandler;
         
        config_extint_chan.gpio_pin           = ARM_PIN;
        config_extint_chan.gpio_pin_mux       = ARM_EIC_MUX;
        config_extint_chan.gpio_pin_pull      = EXTINT_PULL_UP;  // <<<===
        config_extint_chan.detection_criteria = EXTINT_DETECT_BOTH;
        extint_chan_set_config(ARM_EIC_LINE, &config_extint_chan);
        
        extint_register_callback(extint_callback_debounce_daisyChain, ARM_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
        extint_chan_enable_callback(ARM_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
        
        daisyChainCountdownTimer.interval = DAISY_CHAIN_ALARM_TIME;             // delay
     }
     
      
      daisyChainCountdownTimer.mode     = SYS_TIMER_PERIODIC_MODE;
      daisyChainCountdownTimer.handler  = daisyChainCountdownTimerHandler;
      
      
      #warning "TODO: Slave should wait to find daisyChain first"
      SYS_TimerStart(&daisyChainCountdownTimer);
}


static void extint_callback_debounce_daisyChain(void)
{
    SYS_TimerRestart(&daisyChainDebounceTimer);
}


static void daisyChainDebounceTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
    
    #warning "TODO: Detect several toggles before arming?"
    app_arm_set_daisyChainTamper_armed(SYSTEM_ARMED);  
}


static void daisyChainCountdownTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);
    
     if(AM_IS_MASTER == alarmModuleStatus.isMaster)
     {
         // Daisy Chain Master - Toggle the heartbeat level
         port_pin_toggle_output_level(ARM_PIN);
     }
     else
     {
        // Daisy Chain Slave - if we didn't see the heartbeat and a channel is armed we alarm
        
        if( app_gen_io_is_any_port_armed() && ( SYSTEM_ARMED == app_arm_get_daisyChainTamper_armed() ) )
        {
            app_arm_alarmEvent(DAISY_CHAIN_TAMPER_ALARM);
            app_arm_set_daisyChainTamper_armed(SYSTEM_DISARMED); 
        }
     }         
}

