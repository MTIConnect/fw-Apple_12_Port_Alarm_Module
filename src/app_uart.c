/*
 * app_uart.c
 *
 * Created: 10/18/2023 1:48:01 PM
 *  Author: dmcdougall
 */ 

// # Get Versions
// #python dbgOTWU.py -p COM3 -d
//
// # Update Device
// #python dbgOTWU.py -p COM3 -m 150-00219 Connect_2-0_FM_Puck_Rev_00008_Dev.bin 00008
// #python gwOTAU.py -m 150-00219 038-00085_Rev00014.bin 00014
//

// ****************************************************************************
//					#includes
// ****************************************************************************

#include <asf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>  // printf()
#include <stdio_serial.h>
#include <stdlib.h>  //
#include <string.h>  // strlen() strcmp()
#include <vpi/circBuf.h>
#include "app_uart.h"
//#include "app_LED.h"
//#include "app_adc.h"  // For adc value printing
#include "app_arm.h"
//#include "app_bbu.h"
//#include "app_bootloader.h"
#include "app_buzzer.h"
//#include "app_eeprom.h"  // For HW Model Number (150-00XXX), HW Version No, DMA
#include "app_gen_io.h"  // Functions and ISRs
//#include "app_rfid_state.h"
#include "conf_board.h"  // #defines
#include "config.h"      // For Firmware Version
#include "sysTimer.h"

#warning "TODO: re-enable uart includes"

// ****************************************************************************
//					#defines
// ****************************************************************************

#define MODEL_NUMBER_LEN  9
#define MODEL_VERSION_LEN 7
#define SERIAL_NUMBER_LEN 32

#define UART_DMA_BUFFER_SIZE  (OTAU_PACKET_LENGTH + 1)
#define UART_CIRC_BUFFER_SIZE (UART_DMA_BUFFER_SIZE * 2)
#define UART_RX_BUFFER_SIZE   UART_DMA_BUFFER_SIZE
#define UART_COMMAND_LENGTH   2
#define UART_TX_BUFFER_LENGTH UART_CIRC_BUFFER_SIZE

// ****************************************************************************
//					Typedefs
// ****************************************************************************

typedef void (*CommmandHandler)(char* msg);

typedef struct
{
    const char* command;
    uint8_t len;
    const char* commandError;
    CommmandHandler handler;

} Command;

// ****************************************************************************
//					Local static variables
// ****************************************************************************

char tx_buffer[TX_BUFFER_LENGTH];
static bool printDebugData;
static bool useTetherMissedPingMaxAlt;
static bool allowTempDebugData;

static uint8_t dmaBuff[DMA_BUFFER_SIZE];
static VPCircBuf_Element rxBuff[CIRC_BUFFER_SIZE];
static VPCircBuf rxCircBuff;
static SYS_Timer_t dmaTimer;

static struct usart_module usart_instance;
struct dma_resource usart_dma_resource_rx;

COMPILER_ALIGNED(16)
DmacDescriptor usart_dma_descriptor_rx;

static uint8_t rx_data_index = 0;
static char rx_data[RX_BUFFER_SIZE];
static bool update_packet     = false;
static bool alt_update_packet = false;

// ****************************************************************************
//                 References to variables elsewhere, for status printing
// ****************************************************************************

#warning "TODO: add in device status"
//extern volatile DeviceStatus_t deviceStatus;

// ****************************************************************************
//					Function Prototypes
// ****************************************************************************

static void dmaTimerHandler(SYS_Timer_t* timer);
void app_uart_tx(void);
void usart_error_callback(struct usart_module* const usart_module);
void usart_read_callback(struct usart_module* const usart_module);
void usart_write_callback(struct usart_module* const usart_module);
static void app_uart_printResetReason(uint8_t resetCause);


static void handleBV(char* msg);  // Get Battery Voltage
static void handleCR(char* msg);  // Print Debug data to uart
static void handleFF(char* msg);  // Free Function
static void handleGS(char* msg);  // Get Sensor Status
static void handleGV(char* msg);  // Get Version Request
static void handlePT(char* msg);  // Play Tune
static void handleRB(char* msg);  // Reboot Primary or Secondary Nodes
static void handleSA(char* msg);  // Set Arm
static void handleSH(char* msg);  // Set userConfig
static void handleSU(char* msg);  // Start Update
static void handleSW(char* msg);  // Switch Update
static void handleQM(char* msg);  // Get HELP

// ****************************************************************************
//					Messages
// ****************************************************************************

// clang-format off
static Command commands[] = {
    // ID  len Error                                                Handler
    {"BV", 2,  "NG Error - BV\n",                                   handleBV},
    {"CR", 2,  "NG Error - CR\n",                                   handleCR},
    {"FF", 2,  "NG Error - FF\n",                                   handleFF},
    {"GS", 2,  "NG Error - GS\n",                                   handleGS},
    {"GV", 2,  "NG Error - GV\n",                                   handleGV},
    {"PT", 5,  "NG Error - PT <NN>\n",                              handlePT},
    {"RB", 4,  "NG Error - RB <N>\n",                               handleRB},
    {"SA", 4,  "NG Error - SA <N>\n",                               handleSA},
    {"SH", 9,  "NG Error - SH <VV><AA><SS>\n",                      handleSH},
    {"SU", 10, "NG Error - SU<BBBB><CCCC>\r",                       handleSU},
    {"SW", 2,  "NG Error - SW\r",                                   handleSW},
    {"??", 2,  "NG Error - ??\n",                                   handleQM},
    {NULL, 0, NULL, NULL}};
// clang-format on

// ****************************************************************************
//					INITILIZATIONS
// ****************************************************************************

static void dmaTimerHandler(SYS_Timer_t* timer)
{
    UNUSED(timer);

    if ((usart_instance.hw->USART.CTRLA.reg & SERCOM_USART_CTRLA_ENABLE))  // Only do this if I have a UART setup
    {
        cpu_irq_enter_critical();

        dma_abort_job(&usart_dma_resource_rx);
        if (usart_dma_resource_rx.transfered_size != 0)
        {
            vpCircBuf_putAll(&rxCircBuff, (const VPCircBuf_Element*)dmaBuff, usart_dma_resource_rx.transfered_size);
        }
        // dma_start_transfer_job(&usart_dma_resource_rx);

        uint8_t dmaStartStatus = dma_start_transfer_job(&usart_dma_resource_rx);

        cpu_irq_leave_critical();

        if (dmaStartStatus != 0x00)  // if status is not returned as "Okay"
        {
            UART_TX("UART DMA Start Transfer Status in dmaTimerHandler: %02X \n",
                    dmaStartStatus);  // printf could cause a reboot
        }
    }
}

static void transfer_done_rx(struct dma_resource* const resource)
{
    if (SYS_TimerStarted(&dmaTimer))
    {
        SYS_TimerStop(&dmaTimer);
    }

    if ((usart_instance.hw->USART.CTRLA.reg & SERCOM_USART_CTRLA_ENABLE))  // Only do this if I have a UART setup
    {
        vpCircBuf_putAll(&rxCircBuff, (const VPCircBuf_Element*)dmaBuff, DMA_BUFFER_SIZE);

        uint8_t dmaStartStatus = dma_start_transfer_job(&usart_dma_resource_rx);
        if (dmaStartStatus != 0x00)  // if status is not returned as "Okay"
        {
            UART_TX("UART DMA Start Transfer Status in transfer_done_rx: %02X \n",
                    dmaStartStatus);  // printf could cause a reboot
        }
    }
}

void usart_read_callback(struct usart_module* const usart_module)
{
    // Set RX Start interrupt
    usart_instance.hw->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXS;
    // Gets cleared in interrupt handler

    SYS_TimerRestart(&dmaTimer);

    #warning "TODO: add BBU"
//    app_bbu_sleep_on_exit(false);
}

void app_uart_enable(void)
{
    printDebugData            = false;
    useTetherMissedPingMaxAlt = false;

    vpCircBuf_init(&rxCircBuff, rxBuff, CIRC_BUFFER_SIZE);

    dmaTimer.interval = 25;
    dmaTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    dmaTimer.handler  = dmaTimerHandler;

    struct usart_config config_usart;
    struct dma_resource_config config_dma_resource_rx;
    struct dma_descriptor_config config_dma_descriptor;


    // Configure the USART settings and initialize the standard I/O library
    usart_get_config_defaults(&config_usart);
    config_usart.generator_source             = GCLK_GENERATOR_5;
    config_usart.start_frame_detection_enable = true;
    config_usart.baudrate                     = DEBUG_UART_BAUDRATE;
    config_usart.mux_setting                  = DEBUG_UART_SERCOM_MUX_SETTING;
    config_usart.pinmux_pad0                  = DEBUG_UART_SERCOM_PINMUX_PAD0;
    config_usart.pinmux_pad1                  = DEBUG_UART_SERCOM_PINMUX_PAD1;
    config_usart.pinmux_pad2                  = DEBUG_UART_SERCOM_PINMUX_PAD2;
    config_usart.pinmux_pad3                  = DEBUG_UART_SERCOM_PINMUX_PAD3;
    while (usart_init(&usart_instance, DEBUG_UART_MODULE, &config_usart) != STATUS_OK)
    {
    }

    stdio_serial_init(&usart_instance, DEBUG_UART_MODULE, &config_usart);
    usart_enable(&usart_instance);

    usart_register_callback(&usart_instance, usart_read_callback, USART_CALLBACK_START_RECEIVED);
    usart_enable_callback(&usart_instance, USART_CALLBACK_START_RECEIVED);

    usart_register_callback(&usart_instance, usart_error_callback, USART_CALLBACK_ERROR);
    usart_enable_callback(&usart_instance, USART_CALLBACK_ERROR);

    // Set RX Start interrupt
    usart_instance.hw->USART.INTFLAG.reg  = SERCOM_USART_INTFLAG_RXS;
    usart_instance.hw->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXS;
    // Gets cleared in interrupt handler

    // Configure DMA
    dma_get_config_defaults(&config_dma_resource_rx);
    config_dma_resource_rx.peripheral_trigger = DEBUG_UART_SERCOM_DMAC_ID_RX;
    config_dma_resource_rx.trigger_action     = DMA_TRIGGER_ACTION_BEAT;  // DMA_TRIGGER_ACTION_BEAT;
    dma_allocate(&usart_dma_resource_rx, &config_dma_resource_rx);

    dma_descriptor_get_config_defaults(&config_dma_descriptor);
    config_dma_descriptor.beat_size            = DMA_BEAT_SIZE_BYTE;
    config_dma_descriptor.src_increment_enable = false;
    config_dma_descriptor.block_transfer_count = DMA_BUFFER_SIZE;
    config_dma_descriptor.destination_address  = (uint32_t)dmaBuff + sizeof(dmaBuff);
    config_dma_descriptor.source_address       = (uint32_t)(&usart_instance.hw->USART.DATA.reg);
    dma_descriptor_create(&usart_dma_descriptor_rx, &config_dma_descriptor);
    dma_add_descriptor(&usart_dma_resource_rx, &usart_dma_descriptor_rx);

    dma_register_callback(&usart_dma_resource_rx, transfer_done_rx, DMA_CALLBACK_TRANSFER_DONE);
    dma_enable_callback(&usart_dma_resource_rx, DMA_CALLBACK_TRANSFER_DONE);

    uint8_t dmaStartStatus = dma_start_transfer_job(&usart_dma_resource_rx);
    if (dmaStartStatus != STATUS_OK)
    {
        UART_TX("UART DMA Start Transfer Status in app_uart_enable: %02X \n", dmaStartStatus);  // printf could cause a reboot
    }

    UART_TX("NG Debug Port Enabled\n");
}

// ****************************************************************************
//					Functions
// ****************************************************************************

static void messageHandler(char* data, size_t size)
{
    Command* command = commands;

    while (command->command != NULL)
    {
        if (memcmp_code2ram(command->command, data, COMMAND_LENGTH) == 0)
        {
            volatile uint8_t cmdLength = strlen(data);
            if (cmdLength == command->len)
            {
                command->handler(data);
            }
            else
            {
                UART_TX(command->commandError);
            }
            return;
        }
        command++;
    }
    UART_TX("NG Invalid command received\n");
}

void app_uart_task(void)
{
    if ((usart_instance.hw->USART.CTRLA.reg & SERCOM_USART_CTRLA_ENABLE))
    {
        uint16_t rx_data_counter = vpCircBuf_count(&rxCircBuff);

        while (rx_data_counter > 0)
        {
            rx_data_counter--;

            vpCircBuf_getElement(&rxCircBuff, &rx_data[rx_data_index]);

            if (update_packet && (rx_data_index < UPDATE_PACKET_LENGTH))
            {  // Check for Update packet data
                rx_data_index++;

                if (rx_data_index >= UPDATE_PACKET_LENGTH)
                {
                    rx_data_index   = 0;
                    rx_data_counter = 0;

                #warning "TODO: add in bootloader"
//                     app_bootloader_data_handler(UPDATE_COMMAND_PACKET, (rx_data[1] << 8) + rx_data[0],
//                                                 (rx_data[UPDATE_PACKET_LENGTH - 1] << 8) + rx_data[UPDATE_PACKET_LENGTH - 2],
//                                                 (uint8_t*)&rx_data[2]);

                    update_packet = false;
                    memset(rx_data, 0, RX_BUFFER_SIZE);  // Clear buffer
                }
            }
            else if (alt_update_packet && (rx_data_index < UPDATE_PACKET_LENGTH))
            {  // Check for Alt Update packet data
                rx_data_index++;

                if (rx_data_index >= UPDATE_PACKET_LENGTH)
                {
                    rx_data_index   = 0;
                    rx_data_counter = 0;
                    
                    #warning "TODO: add in bootloader"
//                     app_puckToBase_update_handler(UPDATE_COMMAND_PACKET, (rx_data[1] << 8) + rx_data[0],
//                                                   (rx_data[UPDATE_PACKET_LENGTH - 1] << 8) + rx_data[UPDATE_PACKET_LENGTH - 2],
//                                                   (uint8_t*)&rx_data[2]);

                    alt_update_packet = false;
                    memset(rx_data, 0, RX_BUFFER_SIZE);  // Clear buffer
                }
            }
            else if (rx_data[rx_data_index] == 0xaa)
            {  // Check for OTAU packet start flag
                rx_data_index = 0;
                memset(rx_data, 0, RX_BUFFER_SIZE);  // Clear buffer
                update_packet = true;
            }
            else if (rx_data[rx_data_index] == 0xab)
            {  // Check for Alt Update packet start flag
                rx_data_index = 0;
                memset(rx_data, 0, RX_BUFFER_SIZE);  // Clear buffer
                alt_update_packet = true;
            }
            else if (rx_data[rx_data_index] == 0x0D)
            {  // Check for CR character
                rx_data[rx_data_index] = 0;
                rx_data_index          = 0;
                size_t messageSize     = rx_data_counter;
                rx_data_counter        = 0;
                messageHandler(rx_data, messageSize);
                memset(rx_data, 0, RX_BUFFER_SIZE);  // Clear buffer

                break;
            }
            else if (rx_data[rx_data_index] == 0x08)
            {  // Check for backspace
                if (rx_data_index > 0)
                {
                    rx_data_index--;
                }
            }
            else if (rx_data_index == (RX_BUFFER_SIZE - 1))
            {  // Buffer overflow
                rx_data_index = 0;
                memset(rx_data, 0, RX_BUFFER_SIZE);  // Clear buffer
                UART_TX("NG Buffer overflow\n");
            }
            else
            {
                rx_data_index++;
            }
        }
    }
}

void app_arraySN_to_strSN(uint8_t* strSN, uint8_t* arraySN)
{
    char tempString[3];
    memset(strSN, 0, 33);

    for (uint8_t index1 = 0; index1 < 16; index1++)
    {
        sprintf(tempString, "%02X", arraySN[index1]);
        strcat((char*)strSN, tempString);
    }
}

// ****************************************************************************
//					UART Message Functions
// ****************************************************************************

static void handleBV(char* msg)  // Get Battery Voltage
{
//     UART_TX("\n\nBattery Voltage: %d \n", app_bbu_get_battery_level());
//     if (STATUS_BUSY == app_adc_configure(BAT_MON_AIN, app_bbu_battery_check_ADC_complete_callback))
//     {
//         UART_TX("\tADC Busy\n\n");
//     }
}

void app_uart_printResetReason(uint8_t resetCause)
{
    if (resetCause == SYSTEM_RESET_CAUSE_SOFTWARE)
    {
        UART_TX("\tReset Cause: Software\n");
    }
    else if (resetCause == SYSTEM_RESET_CAUSE_WDT)
    {
        UART_TX("\tReset Cause: Watchdog Timer\n");
    }
    else if (resetCause == SYSTEM_RESET_CAUSE_BOD33)
    {
        UART_TX("\tReset Cause: BOD33\n");
    }
    else if (resetCause == SYSTEM_RESET_CAUSE_BOD12)
    {
        UART_TX("\tReset Cause: BOD12\n");
    }
    else if (resetCause == SYSTEM_RESET_CAUSE_POR)
    {
        UART_TX("\tReset Cause: Power On Reset\n");
    }
    else
    {
        UART_TX("\tReset Cause: External Reset\n");
    }
}

static void handleCR(char* msg)  
{
    if (printDebugData)
    {
        printDebugData = false;
    }
    else
    {
        printDebugData = true;
    }

    UART_TX("CR: %d\n", printDebugData);
}


static void handleGS(char* msg)
{
    UART_TX("\n\nGET STATUS:\r\r");
    app_uart_printResetReason(system_get_reset_cause());

    AlarmModuleStatus_t AM_Stat; 
    PortStatus_t chanStat[CH_COUNT];
    AlarmStatus_t alarmStat;

 // System Status
    AM_Stat.sAlarmModule = app_gen_io_get_AM_status();
    
    UART_TX("\r\rSYSTEM STATUS:\r");
    UART_TX("\tAM Status: 0x%02x\r", AM_Stat.sAlarmModule);
    UART_TX("\tAM.isMaster: %c\r", (AM_Stat.isMaster ? '1' : '0') );
    UART_TX("\tAM.Powered: %c\r", (AM_Stat.Powered ? '1' : '0') );
    UART_DBG_TX("\tAM.notCharging: %c\r", (AM_Stat.notCharging ? '1' : '0') );
    UART_DBG_TX("\tAM.deepSleep: %c\r", (AM_Stat.deepSleep ? '1' : '0') );
    UART_DBG_TX("\tAM.shutDown: %c\r", (AM_Stat.shutDown ? '1' : '0') );
    UART_TX("\tAM.switchLifted: %c\r", (AM_Stat.switchLifted ? '1' : '0') );
    
    if(app_gen_io_get_nDISARM() == CAN_ARM)
    {
        UART_TX("\tnDISARM 1: Can Arm\r");
    }
    else
    {
        UART_TX("\tnDISARM 0: Can't Arm\r");
    }
    
 // Daisy Chain Status   
    UART_TX("\rDAISY CHAIN STATUS:\r");
   
    UART_TX("\rCR: %d\r", printDebugData);
    
// Arm Status 
    alarmStat.sAlarm = app_arm_get_alarm_status();
        
    UART_TX("\rARM STATUS:\r");
    UART_TX("\r\tarmed: %c\r", (alarmStat.armed ? '1' : '0') );
    UART_TX("\tsilentAlarm: %c\r", (alarmStat.silentAlarm ? '1' : '0') );
    UART_TX("\tchannel_Alarm: %c\r", (alarmStat.channel_Alarm ? '1' : '0') );
    UART_TX("\tpowerTamper_Armed: %c\r", (alarmStat.powerTamper_Armed ? '1' : '0') );
    UART_TX("\tpowerTamper_Alarm: %c\r", (alarmStat.powerTamper_Alarm ? '1' : '0') );
    UART_TX("\tdaisyChainTamper_Alarm: %c\r", (alarmStat.daisyChainTamper_Alarm ? '1' : '0') );
    UART_TX("\tdaisyChainTamper_Armed: %c\r", (alarmStat.daisyChainTamper_Armed ? '1' : '0') );
        
    
// Fetch and display the Channel Data
    UART_TX("\rCHANNEL STATUS:\r");
    for(int num = 0; num < CH_COUNT; num++)
    {
        chanStat[num].sPort  = app_gen_io_get_Channel_Status(num);
        UART_TX("\r\tChannel %d Status : 0x%02x\r", num, chanStat[num].sPort);  
        UART_TX("\tcablePresent: %c\r", ( chanStat[num].cablePresent ? '1' : '0') );
        //UART_TX("\tswitchClosed: %c\r", ( chanStat[num].switchClosed ? '1' : '0') );
        UART_TX("\tarmed: %c\r", (chanStat[num].armed ? '1' : '0') );
        UART_TX("\talarming: %c\r", (chanStat[num].alarming ? '1' : '0') );      
    }
    
    //Battery
    UART_TX("\rBATTERY STATUS:\r");
    
    //Other IO -
    

}

static void handleGV(char* msg)
{
    char modelNumber[MODEL_NUMBER_LEN + 1];
    char modelVersion[MODEL_VERSION_LEN + 1];
    uint8_t strSN[SERIAL_NUMBER_LEN + 1];

//     app_arraySN_to_strSN(strSN, app_eeprom_read_serial_number());
//     memset(modelNumber, 0, sizeof(modelNumber));
//     strncpy(modelNumber, app_eeprom_read_model_number(), MODEL_NUMBER_LEN);
//     memset(modelVersion, 0, sizeof(modelVersion));
//     strncpy(modelVersion, app_eeprom_read_model_version(), MODEL_VERSION_LEN);

    UART_TX("NN%s%s\t%s\t%s\r", (char*)strSN, modelNumber, modelVersion, APP_VERSION);
}

static void handleFF(char* msg)
{
    

    port_pin_toggle_output_level(DISARMED_FLASH_PIN);

    if(port_pin_get_input_level(DISARMED_FLASH_PIN))
    {
        UART_TX("\nDisarmed_Flash_Pin: HIGH\n");  
    }
    else
    {
        UART_TX("\nDisarmed_Flash_Pin: Low\n");
    }


    //     app_internalTemp_readTemp();
    //     app_internalTemp_print();

    //     // For forcing Standard mode to test auth replace with only 1 auth card.
    //     app_eeprom_write_connect_wanted(APP_EEPROM_MODEL_TYPE_NON_CONNECTED);

    // 		if(useTetherMissedPingMaxAlt)
    // 		{
    // 			useTetherMissedPingMaxAlt = false;
    // 		}
    // 		else
    // 		{
    // 			useTetherMissedPingMaxAlt = true;
    // 		}
    //
    // 		app_cutdetect_set_tether_missed_ping_max();
    //
    // 		UART_TX("useTetherMissedPingMaxAlt: %d\n", useTetherMissedPingMaxAlt);
}

static void handlePT(char* msg)
{
    char tempStr[3];
    memset(tempStr, '\0', sizeof(tempStr));
    strncpy(tempStr, &msg[3], 2);
    enum app_buzzer_pattern_t activeTune = (enum app_buzzer_pattern_t)(strtoul(tempStr, 0, 16) & 0xFF);

    UART_TX("\n\nPLAY TUNE: 0x%02X\n", activeTune);

    if (!app_arm_is_any_alarm_active())
    {
        app_buzzer_stop_pattern(app_buzzer_pattern_playing());
        app_buzzer_start_pattern(activeTune);
    }
}

static void handleRB(char* msg)
{
    enum
    {
        puck = 0,
        base = 1
    };

    char tempStr[2];
    memset(tempStr, '\0', sizeof(tempStr));
    strncpy(tempStr, &msg[3], 1);
    uint8_t node = strtoul(tempStr, 0, 16) & 0x0F;

    UART_TX("\n\nREBOOT REQUEST:\t");

    if (puck == node)
    {
        UART_DBG_TX("Software Resetting Puck\n");
        system_reset();
    }
}

static void handleSA(char* msg)
{
    // 	PuckStatus_t puckStatus;
    // 	puckStatus.sPuck = app_gen_io_get_puck_status();

    enum
    {
        disarm  = 0,
        arm     = 1,
        silence = 2
    };

    uint8_t ctrlByte = msg[3] - 48;  // Convert from ASCII

    UART_TX("\n\nSET ARM/DISARM:\n");

    if ((ctrlByte == arm))
    {
        UART_TX("\nRequesting product ARM...\n");
        if (app_arm_request(false, ARM_IGNORE_NONE))
        {
            UART_TX("SUCCESS, Armed!\n");
        }
        else
        {
            UART_TX("Failure, still disarmed!\n");
            app_arm_check_why_arm_failed();
        }
    }
    else if ((ctrlByte == disarm))
    {
        app_arm_disarm(0);
        UART_TX("\nDISARMED!\n\n");
    }
    else if ((ctrlByte == silence))
    {
        if (app_arm_silence_alarm())
        {
            UART_TX("\nSilenced!\n\n");
        }
        else
        {
            UART_TX("\nDISARMED!\n\n");
        }
    }
    else
    {
        UART_TX("\n\tInvalid ARM/DISARM command\n");
        UART_TX("\tSA 0 = DISARM\n");
        UART_TX("\tSA 1 = ARM\n");
        UART_TX("\tSA 2 = SILENCE\n");
    }
}

static void handleSH(char* msg)
{
    UART_TX("\n\nSET USER CONFIG:\n");
// 
//     char tempStr[3];
// 
//     memset(tempStr, '\0', sizeof(tempStr));
//     strncpy(tempStr, &msg[3], 2);
//     uint8_t volumeSetting = strtoul(tempStr, 0, 16) & 0xFF;
// 
//     memset(tempStr, '\0', sizeof(tempStr));
//     strncpy(tempStr, &msg[5], 2);
//     uint8_t alarmSetting = strtoul(tempStr, 0, 16) & 0xFF;
// 
//     memset(tempStr, '\0', sizeof(tempStr));
//     strncpy(tempStr, &msg[7], 2);
//     uint8_t securitySetting = strtoul(tempStr, 0, 16) & 0xFF;
// 
//     app_user_options_set_config(volumeSetting, alarmSetting, securitySetting);
}

static void handleSU(char* msg)
{
//     char tempStr[17];
//     memset(tempStr, '\0', sizeof(tempStr));
//     strncpy(tempStr, &msg[2], 4);
//     uint16_t len = strtoul(tempStr, 0, 16) & 0xFFFF;
//     memset(tempStr, '\0', sizeof(tempStr));
//     strncpy(tempStr, &msg[6], 4);
//     uint16_t cs = strtoul(tempStr, 0, 16) & 0xFFFF;
// 
//     app_bootloader_data_handler(UPDATE_COMMAND_START, len, cs, NULL);
}

static void handleSW(char* msg)
{
 //   app_bootloader_data_handler(UPDATE_COMMAND_SWITCH, 0, 0, NULL);
}




static void handleQM(char* msg)  // HELP
{
    UART_TX("\n");
    UART_TX("BV - Battery Voltage\n");
    UART_TX("CR - Print debug data to UART\n");
    UART_TX("FF - Free Function (placeholder)\n");
    UART_TX("GS - Get Status\n");
    UART_TX("GV - Get Version\n");
    UART_TX("PT <NN> - Play Tune\n");
    UART_TX("RB <N> - Reboot");
    UART_TX("SA <N> - Set Arm/Disarm\n");
    UART_TX("SH <VV><AA><SS> - Set User Config\n");
    UART_TX("?? - Help\n");
    UART_TX("\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void app_uart_tx(void)
{
    usart_write_buffer_wait(&usart_instance, (uint8_t*)tx_buffer, strnlen(tx_buffer, TX_BUFFER_LENGTH));
}

bool UART_TX(const char* transmitString, ...)
{
    if ((usart_instance.hw->USART.CTRLA.reg & SERCOM_USART_CTRLA_ENABLE))
    {
        va_list args;
        char UART_TX_tx_buffer[TX_BUFFER_LENGTH];
        va_start(args, transmitString);
        vsnprintf(UART_TX_tx_buffer, TX_BUFFER_LENGTH, transmitString, args);
        usart_write_buffer_wait(&usart_instance, (uint8_t*)UART_TX_tx_buffer,
                                strnlen(UART_TX_tx_buffer, TX_BUFFER_LENGTH));  // Transmit the buffer
        va_end(args);

        return true;
    }

    return false;
}

// *********************************************************************************************************************************
//	 USE THIS FUNCTION FOR PERIODIC DATA REQUESTS OR BATTERY VOLTAGE PRINTS
// *********************************************************************************************************************************

void UART_DBG_TX(const char* transmitString, ...)
{
    if (printDebugData || allowTempDebugData)
    {
        if ((usart_instance.hw->USART.CTRLA.reg & SERCOM_USART_CTRLA_ENABLE))
        {
            va_list args;
            char UART_TX_tx_buffer[TX_BUFFER_LENGTH];
            va_start(args, transmitString);
            vsnprintf(UART_TX_tx_buffer, TX_BUFFER_LENGTH, transmitString, args);
            usart_write_buffer_wait(&usart_instance, (uint8_t*)UART_TX_tx_buffer,
                                    strnlen(UART_TX_tx_buffer, TX_BUFFER_LENGTH));  // Transmit the buffer
            va_end(args);
            allowTempDebugData = false;
        }
    }
}

bool app_uartDebugRunning(void)
{
    if ((usart_instance.hw->USART.CTRLA.reg & SERCOM_USART_CTRLA_ENABLE))
    {
        return true;
    }

    return false;
}

void app_uart_disable(void)
{
    printDebugData = false;

    usart_disable(&usart_instance);
    usart_disable_callback(&usart_instance, USART_CALLBACK_BUFFER_RECEIVED);
    usart_disable_callback(&usart_instance, USART_CALLBACK_ERROR);

    SYS_TimerStop(&dmaTimer);
    dma_abort_job(&usart_dma_resource_rx);
    dma_disable_callback(&usart_dma_resource_rx, DMA_CALLBACK_TRANSFER_DONE);
    dma_free(&usart_dma_resource_rx);
}

void usart_error_callback(struct usart_module* const usart_module)
{
    app_uart_disable();  //
    app_uart_enable();   // preserve rx_buffer contents
}

void app_uart_print_flash_key(int16_t index, uint8_t keyType, uint8_t* keySerial)
{
    int i;
    UART_TX("Key %03d, type 0x%02X: SN:", index, keyType);
    for (i = 0; i < 4; i++)
    {
        UART_TX(" %02X", keySerial[i]);
    }
    UART_TX("\n");
}
