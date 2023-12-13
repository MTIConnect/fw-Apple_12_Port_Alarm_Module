/*
 * app_wdt.c
 *
 * Created: 10/18/2023 1:38:44 PM
 *  Author: dmcdougall
 */ 


#include <asf.h>
#include "app_wdt.h"
//#include "app_eeprom.h"
#include "config.h"  // So that this function sees the definition of APP_DISABLE_WDT
#include "wdt.h"

void app_wdt_early_warning_callback(void);

void app_wdt_kick(void)
{
    #ifndef APP_DISABLE_WDT
    if (!wdt_is_syncing())
    {
        Wdt *const WDT_module = WDT;

        // Kick WDT
        WDT_module->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
    }
    #endif
}

void app_wdt_enable(void)
{
    #ifndef APP_DISABLE_WDT
    // Wait for synchronization
    while (wdt_is_syncing())
    {
    }

    struct wdt_conf config_wdt;
    wdt_get_config_defaults(&config_wdt);

    config_wdt.enable               = true;
    config_wdt.always_on            = false;
    config_wdt.clock_source         = GCLK_GENERATOR_3;
    config_wdt.timeout_period       = WDT_PERIOD_16384CLK;
    config_wdt.early_warning_period = WDT_PERIOD_8192CLK;

    wdt_register_callback(app_wdt_early_warning_callback, WDT_CALLBACK_EARLY_WARNING);
    wdt_enable_callback(WDT_CALLBACK_EARLY_WARNING);

    // ULP32K/4 =  8192Hz Clock Source
    // 16384/8192 = 2s to trip WDT

    wdt_set_config(&config_wdt);
    #else
    #warning "WDT Disabled"
    #endif
}

void app_wdt_disable(void)
{
    #ifndef APP_DISABLE_WDT
    // Wait for synchronization
    while (wdt_is_syncing())
    {
    }

    struct wdt_conf config_wdt;
    wdt_get_config_defaults(&config_wdt);

    config_wdt.enable = false;

    wdt_set_config(&config_wdt);
    #endif
}

void app_wdt_early_warning_callback(void)
{
    // String can be up to 60 characters
    #warning "TODO: Apple add in eeprom write"
    //app_eeprom_write_error((char *)"Early Warning WDT");
}
