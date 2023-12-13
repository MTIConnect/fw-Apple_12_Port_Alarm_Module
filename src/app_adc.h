/*
 * app_adc.h
 *
 * Created: 10/20/2023 5:27:20 PM
 *  Author: dmcdougall
 */ 


#ifndef APP_ADC_H_
#define APP_ADC_H_

/** Type of the callback functions. */
typedef void (*app_adc_callback_t)(uint16_t voltage);

enum status_code app_adc_configure(uint8_t ADC_channel, app_adc_callback_t callback_func);
enum status_code app_adc_reserve(uint8_t ADC_channel);
enum status_code app_adc_free(uint8_t ADC_channel);

#endif /* APP_ADC_H_ */