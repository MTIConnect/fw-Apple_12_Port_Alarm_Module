/**
 * @file hw_timer.c
 *
 */

#include "tc.h"
#include "tc_interrupt.h"
#include "hw_timer.h"
#include "sysTimer.h"
#include "slpTimer.h"

struct tc_config timer_config;
struct tc_module module_inst;


/*! \brief  hw timer compare callback
 */
static void hw_timer_callback(struct tc_module *const module_instance)
{
	SYS_HwExpiry_Cb();
	SLP_HwExpiry_Cb();
	
#ifdef EXT_HW_EXPIRY_CB
	EXT_HW_EXPIRY_CB();
#endif
}

/*! \brief  initialize hw timer to cause interrupt every HW_TIMER_INTERVAL
 */
void hw_timer_init(void)
{
	tc_get_config_defaults(&timer_config);
	timer_config.run_in_standby = HW_TIME_RUN_IN_STANDBY;
	timer_config.wave_generation = TC_WAVE_GENERATION_MATCH_FREQ;
	timer_config.clock_source = HW_TIMER_SOURCE;
	timer_config.clock_prescaler = HW_TIMER_PRESCALER;
	timer_config.counter_16_bit.compare_capture_channel[0] = HW_TIMER_PERIOD;
	tc_init(&module_inst, HW_TIMER_MODULE, &timer_config);
	tc_register_callback(&module_inst, hw_timer_callback, TC_CALLBACK_CC_CHANNEL0);
	tc_enable_callback(&module_inst, TC_CALLBACK_CC_CHANNEL0);

	tc_enable(&module_inst);
}
