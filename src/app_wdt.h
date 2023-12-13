/*
 * app_wdt.h
 *
 * Created: 10/18/2023 1:37:03 PM
 *  Author: dmcdougall
 */ 



#ifndef APP_WDT_H_
#define APP_WDT_H_

void app_wdt_enable(void);
void app_wdt_disable(void);
void app_wdt_kick(void);

#endif /* APP_WDT_H_ */