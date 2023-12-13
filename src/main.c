/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */
#include <asf.h>
#include "app_LED.h"
#include "app_adc.h"
#include "app_arm.h"
#include "app_bbu.h"
#include "app_buzzer.h"
#include "app_daisychain.h"
#include "app_eeprom.h"
#include "app_gen_io.h"
#include "app_uart.h"
//#include "app_user_options.h"
#include "app_wdt.h"
#include "config.h"
#include "slpTimer.h"
#include "sysTimer.h"

int main (void)
{
	system_init();
    delay_init();
    sleepmgr_init();
    SYS_TimerInit();
    SLP_TimerInit();
 
//    app_eeprom_init();
    app_gen_io_init();  // Need to start the timers before we start the IO
//     app_LED_init();
    app_daisychain_init();
    app_uart_enable();
    app_buzzer_init();
// //    app_user_options_init();
    app_arm_init();
//     app_bbu_init();
    cpu_irq_enable();
    app_wdt_enable();
    // After all the rest of the initialization, start the auto-arm
    // timer, just in case everything is ready.
    app_arm_reset_auto_arm_timer();
    
    while(true)
    {
        app_wdt_kick();
        SYS_TimerTaskHandler();
        app_uart_task();
//         app_bbu_task();
//         app_gen_io_kill_switch_task();
//         app_buzzer_task();
    }
    
    
    
	/* Insert application code here, after the board has been initialized. */
}







