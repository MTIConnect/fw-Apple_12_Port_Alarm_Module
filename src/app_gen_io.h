/*
 * app_gen_io.h
 *
 * Created: 10/20/2023 5:00:45 PM
 *  Author: dmcdougall
 */ 


/*
 * app_gen_io.h
 *
 * Created: 8/24/2015 1:41:01 PM
 *  Author: jcollier
 */

#ifndef APP_GEN_IO_H_
#define APP_GEN_IO_H_

#include "sysTimer.h"

// Debounce and vibration constants
#define STANDARD_DEBOUNCE_INTERVAL_MS       250
#define SHELF_STORAGE_MESSAGE_INTERVAL_MS   2000
#define CABLE_ALARM_COUNTDOWN_INTERVAL_MS   10000

enum
{
    POWER_NOT_GOOD = 0,
    POWER_GOOD     = 1,
};


enum
{
    AM_NOT_MASTER  = 0,
    AM_IS_MASTER   = 1,    
};

enum
{
    CABLE_ABSENT  = 0,
    CABLE_PRESENT = 1,
};

enum
{
    PORT_DISARMED  = 0,
    PORT_ARMED     = 1,
};

enum
{
    PORT_NOT_ALARMING = 0,
    PORT_ALARMING = 1,
};

enum
{
    DISARM_CMD = 0,
    ARM_CMD = 1,
};

// enum
// {
//     PORT_STATUS_NO_PRIMARY_DEVICE    = 1 << 0,
//     PORT_STATUS_NO_SECOND_DEVICE     = 1 << 1,
//     PORT_STATUS_NOT_CHARGING         = 1 << 2,
//     PORT_STATUS_DISARMED             = 1 << 3,
//     PORT_STATUS_PRIMARY_DEVICE_ALARM = 1 << 4,
//     PORT_STATUS_SECOND_DEVICE_ALARM  = 1 << 5,
//     PORT_STATUS_CURRENT_OKAY         = 1 << 6,
//     PORT_STATUS_LIFTED               = 1 << 7,
// };

enum channel_num
{
    CH_00 = 0,         // 0
    CH_01,
    CH_02,
    CH_03,
    CH_04,
    CH_05,
    CH_06,
    CH_07,
    CH_08,
    CH_09,
    CH_10,
    CH_11,
    CH_COUNT,			// 12
};


typedef void (*timerHandler_function)(SYS_Timer_t *timer);

COMPILER_PACK_SET(1)

typedef struct PortStatus_t
{
    union
    {
        struct
        {
            uint16_t cablePresent   : 1;                // 1 =
            uint16_t armed          : 1;                // 1 = Armed
            uint16_t alarming       : 1;                // 1 = alarming
            uint16_t reserved       : 13;
        };
        uint16_t sPort;
    };
} PortStatus_t;

typedef struct AlarmModuleStatus_t
{
    union
    {
        struct
        {
            uint16_t isMaster         : 1;
            uint16_t Powered       : 1;
            uint16_t notCharging      : 1;
            uint16_t deepSleep        : 1;
            uint16_t shutDown         : 1;
            uint16_t switchLifted     : 1;
            uint16_t reserved         : 10;
        };
        uint16_t sAlarmModule;
    };
} AlarmModuleStatus_t;

// These bits reflect hardware status
typedef struct ChannelStatus_t
{
    const uint32_t          gpio_pin;               // The uC pin number of this Stud (ex: port A8)
    const uint32_t          gpio_eic_mux;           // EIC MUX
    const uint32_t          gpio_eic_line;          // EIC LINE
    SYS_Timer_t             *timer; 
    timerHandler_function   timerHandler;                   // pointer to the Channels timerHandler Function                  
    volatile PortStatus_t   portStat;               // Cable
} ChannelStatus_t;

COMPILER_PACK_RESET()

void app_gen_io_init(void);



bool app_gen_io_get_nDISARM(void);
uint16_t app_gen_io_get_AM_status(void);
uint16_t app_gen_io_get_port_status(uint8_t portNum);
void app_gen_io_kill_switch_task(void);
void app_gen_io_set_status_deepSleep(uint16_t sleepState);
uint16_t app_gen_io_get_Channel_Status(uint16_t num);
bool app_gen_io_is_power_good(void);
bool app_gen_io_is_cable_present(uint8_t portNum);
bool app_gen_io_is_port_armed(uint8_t portNum);
void app_gen_io_set_port_armed(uint8_t portNum, bool desiredArmState);
bool app_gen_io_is_any_port_armed(void);
bool app_gen_io_is_port_alarming(uint8_t portNum);

#endif /* APP_GEN_IO_H_ */
