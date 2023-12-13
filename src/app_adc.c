/*
 * app_adc.c
 *
 * Created: 10/20/2023 5:27:08 PM
 *  Author: dmcdougall
 */ 


#include <asf.h>
#include "app_adc.h"
#include "conf_adc.h"
#include "config.h"
#include "slpTimer.h"

// ****************************************************************************
//		Prototypes
static void adcCheckTimerHandler(SLP_Timer_t *timer);
void adc_complete_callback(struct adc_module *const module);

// ****************************************************************************
//		Variables
uint16_t adc_result_buffer[CONF_ADC_NUM_SAMPLES];
uint8_t currentChannel         = 0;
app_adc_callback_t adcCallback = NULL;
bool adcBusy                   = false;
struct adc_module adc_instance;
static SLP_Timer_t adcCheckTimer;

// ****************************************************************************
//		ADC Check Timer
static void adcCheckTimerHandler(SLP_Timer_t *timer)
{
    UNUSED(timer);

    // This timer should only expire if something went wrong setting up an
    // ADC read.  The callback is supposed to stop this timer, but the code
    // seems never to have gotten called.  We'll start the timer again, and
    // hope that clears everything up!
    if (adcBusy)
    {
        adcBusy = false;
        app_adc_configure(currentChannel, adcCallback);
    }
}

// ****************************************************************************
//		ADC Sample Complete Callback
void adc_complete_callback(struct adc_module *const module)
{
    SLP_TimerStop(&adcCheckTimer);

    uint32_t averageADCvalue = 0;

    // First sample after changing channels is invalid
    for (uint8_t i = 1; i < CONF_ADC_NUM_SAMPLES; i++)
    {
        averageADCvalue += adc_result_buffer[i];
    }

    averageADCvalue = averageADCvalue / (CONF_ADC_NUM_SAMPLES - 1);

    uint16_t voltage = averageADCvalue * CONF_ADC_VOLTAGE_CONVERSION;

    // Send voltage to registered callback
    adcCallback(voltage);

    adc_disable(&adc_instance);
    adcBusy = false;
}

//////////////////////////////////////////////////////////////////
// Reserve ADC for external sample
enum status_code app_adc_reserve(uint8_t ADC_channel)
{
    cpu_irq_enter_critical();
    if (adcBusy)
    {
        cpu_irq_leave_critical();
        return STATUS_BUSY;
    }
    currentChannel = ADC_channel;
    adcBusy        = true;
    cpu_irq_leave_critical();
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////
// Free ADC
enum status_code app_adc_free(uint8_t ADC_channel)
{
    cpu_irq_enter_critical();
    if (adcBusy && (currentChannel != ADC_channel))
    {
        cpu_irq_leave_critical();
        return STATUS_BUSY;
    }
    adcBusy = false;
    cpu_irq_leave_critical();
    return STATUS_OK;
}

// ***********************************************************************************************************************************
//		Configure ADC and start job
enum status_code app_adc_configure(uint8_t ADC_channel, app_adc_callback_t callback_func)
{
    cpu_irq_enter_critical();

    if (adcBusy)
    {
        cpu_irq_leave_critical();
        return STATUS_BUSY;
    }
    adcBusy = true;
    cpu_irq_leave_critical();

    if (STATUS_BUSY == adc_get_job_status(&adc_instance, ADC_JOB_READ_BUFFER))
    {
        adc_abort_job(&adc_instance, ADC_JOB_READ_BUFFER);
    }

    currentChannel = ADC_channel;
    adcCallback    = callback_func;

    struct adc_config config_adc;
    adc_get_config_defaults(&config_adc);

    config_adc.clock_source    = CONF_ADC_CLOCK_SOURCE;
    config_adc.clock_prescaler = CONF_ADC_CLK_PRESCALER;
    config_adc.reference       = CONF_ADC_V_REFERENCE;  // Internal Reference = 1/1.48 * VDDANA = 2.23V
    config_adc.resolution      = CONF_ADC_RESOLUTION;
    config_adc.sample_length   = CONF_ADC_SAMPLE_LEN;  // Maximize sample length to reduce noise from ADC
    config_adc.run_in_standby  = CONF_ADC_RUN_IN_STANDBY;
    config_adc.positive_input  = currentChannel;

    adc_init(&adc_instance, ADC, &config_adc);
    adc_enable(&adc_instance);
    adc_register_callback(&adc_instance, adc_complete_callback, ADC_CALLBACK_READ_BUFFER);

    adc_enable_callback(&adc_instance, ADC_CALLBACK_READ_BUFFER);
    adc_read_buffer_job(&adc_instance, adc_result_buffer, CONF_ADC_NUM_SAMPLES);

    adcCheckTimer.interval = CONF_ADC_CHK_TMR_INTERVAL;
    adcCheckTimer.mode     = SLP_TIMER_INTERVAL_MODE;
    adcCheckTimer.handler  = adcCheckTimerHandler;
    SLP_TimerRestart(&adcCheckTimer);

    return STATUS_OK;
}
