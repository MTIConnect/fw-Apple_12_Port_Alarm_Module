/*
 * conf_adc.h
 *
 * Created: 10/20/2023 5:29:06 PM
 *  Author: dmcdougall
 */ 


#ifndef CONF_ADC_H
#define CONF_ADC_H

#define CONF_ADC_CLOCK_SOURCE  GCLK_GENERATOR_5
#define CONF_ADC_CLK_PRESCALER ADC_CLOCK_PRESCALER_DIV8;

// Internal Reference = 1/1.48 * VDDANA = 2.23V
#define CONF_ADC_V_REFERENCE ADC_REFERENCE_INTVCC0
#define CONF_ADC_RESOLUTION  ADC_RESOLUTION_12BIT;

// Maximize sample length to reduce noise from ADC
#define CONF_ADC_SAMPLE_LEN  63
#define CONF_ADC_NUM_SAMPLES 128

// ADC Reference is 1/1.48 * VDDANA = 2.230V
// 12 bit resolution ADC = 4095 counts
// Voltage (mV) = averageADCvalue * 1000 * 2.230 / 4095
//              = averageADCvalue * 2230 / 4095
#define CONF_ADC_VOLTAGE_CONVERSION (2230.0 / 4095)
#define CONF_ADC_CHK_TMR_INTERVAL   500

// Run in Standby
#define CONF_ADC_RUN_IN_STANDBY true

// ****************************************************************
//	NAMES & LOCATIONS OF ALL ADC CALLBACKS USED FOR THIS PROJECT
// ****************************************************************
//
//	app_bbu_battery_check_ADC_complete_callback();
//	app_cableID_dir_ADC_complete_callback();
//	app_usb_current_ADC_complete_callback();
//  app_internalTemp_ADC_complete_callback()

#endif  // CONF_ADC_H