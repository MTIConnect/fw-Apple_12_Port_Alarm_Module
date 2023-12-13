/*
 * app_bbu.h
 *
 * Created: 10/20/2023 5:16:55 PM
 *  Author: dmcdougall
 */ 


#ifndef APP_BBU_H_
#define APP_BBU_H_

void app_bbu_init(void);
void app_bbu_task(void);
void app_bbu_battery_check_ADC_complete_callback(uint16_t bat);
uint16_t app_bbu_get_battery_level(void);
bool app_bbu_get_battery_charging(void);
bool app_bbu_sleeping(void);
void app_bbu_sleep_request(void);
void app_bbu_request_active(void);
void app_bbu_sleep_on_exit(bool sleepOnExit);
//void app_bbu_change_main_clock_source(uint8_t clkSrc);
bool app_bbu_TimeLimitTimerComplete(void);

#endif /* APP_BBU_H_ */