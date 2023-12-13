/*
 * conf_timer.h
 *
 * Created: 10/18/2023 1:56:41 PM
 *  Author: dmcdougall
 */ 

#ifndef CONF_TIMER_H_
#define CONF_TIMER_H_

// Adjust HW_TIMER to cause interrupt every HW_TIMER_INTERVAL
// 8MHz * 1ms / 64 = 125
#define HW_TIMER_INTERVAL      1ul /* ms */
#define HW_TIMER_PERIOD        125
#define HW_TIMER_SOURCE        GCLK_GENERATOR_5
#define HW_TIMER_PRESCALER     TC_CLOCK_PRESCALER_DIV64
#define HW_TIMER_MODULE        TC3
#define HW_TIME_RUN_IN_STANDBY true

// If timers are started or stopped from interrupt this must be defined
#define HW_TIMER_ENTER_CRITICAL cpu_irq_enter_critical();
#define HW_TIMER_LEAVE_CRITICAL cpu_irq_leave_critical();

// Define if external hardware timer expiration callback is required
#include "config.h"

/*
 * - SYS timer handlers are called from the main loop
 * - SLP timer handlers are called from interrupt and will operate in all sleep modes
 *
 * - Example usage
 * - SYS and/or SLP timers can be used
 *
 * - SYS Timer usage
 * - #include "sysTimer.h"
 * - SYS_TimerInit(); // Call during main initialization code
 * - SYS_TimerTaskHandler(); // Call in main loop
 *
 * - static SYS_Timer_t testTimer; //  Global variable for specific timer
 * - static void testTimerHandler(SYS_Timer_t *timer) // Timer expiration callback handler
 *   {
 *		UNUSED(timer); // To avoid compiler warning
 *		// Do whatever timer needs to do
 *   }
 *
 *   testTimer.interval = 1000; // Initialize specific timer
 *   testTimer.mode = SYS_TIMER_INTERVAL_MODE;
 *   testTimer.handler = testTimerHandler;
 *   SYS_TimerStart(&testTimer); // Start timer for specific timer
 *
 *
 *
 * - SLP Timer usage
 * - #include "slpTimer.h"
 * - SLP_TimerInit(); // Call during main initialization code
 *
 * - static SLP_Timer_t testTimer; //  Global variable for specific timer
 * - static void testTimerHandler(SLP_Timer_t *timer) // Timer expiration callback handler
 *   {
 *		UNUSED(timer); // To avoid compiler warning
 *		// Do whatever timer needs to do
 *   }
 *
 *   testTimer.interval = 1000; // Initialize specific timer
 *   testTimer.mode = SYS_TIMER_INTERVAL_MODE;
 *   testTimer.handler = testTimerHandler;
 *   SLP_TimerStart(&testTimer); // Start timer for specific timer
 *
 */

#endif /* CONF_TIMER_H_ */

