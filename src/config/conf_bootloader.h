/*
 * conf_bootloader.h
 *
 * Created: 10/23/2023 11:58:04 AM
 *  Author: dmcdougall
 */ 
/*
 * conf_bootloader.h
 *
 * Created: 3/29/2019 3:14:04 PM
 *  Author: jcollier
 */



#ifndef CONF_BOOTLOADER_H_
#define CONF_BOOTLOADER_H_

// Include wired interface (Comment out if not in use)
#include "app_uart.h"

// Include wireless interface (Comment out if not in use)
//#include "app_lwmesh.h"

// For MICRO_2_TYPE definition,
#include "config.h"

// Update to send status through the desired port
// Gateway (RPI Port Only)
/*
#define SEND_UPDATE_STATUS(blockNumber, sts) {\
                                uint8_t strSN[33];\
                                app_arraySN_to_strSN(strSN, app_eeprom_read_serial_number());\
                                printf("NU%s%04X%X\r", strSN, blockNumber, sts);\
                        }
*/

// Node (LWMesh and Debug Port)
#define SEND_UPDATE_STATUS(blockNumber, sts)           \
    {                                                  \
        UART_TX("NU%04X%X\r", blockNumber, sts);       \
    }

// Debug Port Only
/*#define SEND_UPDATE_STATUS(blockNumber, sts) {\
                                printf("NU%04X%X\r", blockNumber, sts);
                        }
*/

// Update Key Definition Table
#define UPDATE_KEY_GATEWAY              0x00
#define UPDATE_KEY_FM_PUCK              0x01
#define UPDATE_KEY_AP_PUCK              0x02
#define UPDATE_KEY_PWR_LOCK             0x03
#define UPDATE_KEY_BAT_LOCK             0x04
#define UPDATE_KEY_REPEATER             0x05
#define UPDATE_KEY_USB_KEY              0x06
#define UPDATE_KEY_UNIFIED_KEY          0x07
#define UPDATE_KEY_AP_AM                0x08
#define UPDATE_KEY_DRAWER_LOCK          0x09
#define UPDATE_KEY_GLASS_LOCK           0x0A
#define UPDATE_KEY_RAM                  0x0B
#define UPDATE_KEY_COREIII              0x0C
#define UPDATE_KEY_SP                   0x0D
#define UPDATE_KEY_PLUNGER_LOCK         0x0E
#define UPDATE_KEY_SP_RFID              0x0F
#define UPDATE_KEY_FM2_PUCK             0x10  // 150-00219 (Daughter board), 150-00282 (integrated)
#define UPDATE_KEY_FM2_BASE             0x11  // 150-00285 (Standard HS base)
#define UPDATE_KEY_CXFLEX1A             0x12
#define UPDATE_KEY_CXFLEX1P             0x13
#define UPDATE_KEY_POE_REPEATER         0x14
#define UPDATE_KEY_CXFLEX4A             0x15
#define UPDATE_KEY_NXDI_PUCK            0x16
#define UPDATE_KEY_NXDI_BASE            0x17
#define UPDATE_KEY_NXDI_SHELL           0x18
#define UPDATE_KEY_VERSA_EX             0x19
#define UPDATE_KEY_FM2_BASE_INT         0x1A  // 150-00285 (High Security)
#define UPDATE_KEY_FM2_LITE_PUCK        0x1B  // 150-00312 (Lite Puck - Depricated)
#define UPDATE_KEY_FM2_VP_PUCK          0x1C  // 150-00307 (VP Puck)
#define UPDATE_KEY_FM2_BASE_VP          0x1D  // 150-00308 (VP Base 038-00131)
#define UPDATE_KEY_CAM_LOCK             0x1E
#define UPDATE_KEY_PADLOCK              0x1F
#define UPDATE_KEY_FM2_BASE_NO_LOCK     0x20  // 150-00323 (FM2 Base Standard)
#define UPDATE_KEY_POE_REPEATER_II      0x21
#define UPDATE_KEY_FM2_PUCK_ALT         0x22  // 150-00330 (SAMR21G17A no Shift Reg no USB)
#define UPDATE_KEY_DRAWER_LOCK_ALT      0x23
#define UPDATE_KEY_GLASS_LOCK_ALT       0x24
#define UPDATE_KEY_PLUNGER_LOCK_ALT     0x25
#define UPDATE_KEY_CAM_LOCK_ALT         0x26
#define UPDATE_KEY_PAD_LOCK_ALT         0x27
#define UPDATE_KEY_FM2_PUCK_NO_SR       0x28  // 150-00331 (SAMR21G18A no Shift Reg no USB)
#define UPDATE_KEY_FM2_BASE_NO_LOCK_ALT 0x29  // 150-00332 (SAMD21E16C & SPI Flash) Standard
#define UPDATE_KEY_FM2_BASE_INT_ALT     0x2A  // 150-00334 (SAMD21E16C & SPI FLash) High Security
#define UPDATE_KEY_MAN_ACCESS           0x2B
#define UDATE_KEY_DRAWER_LOCK_BLE       0x2C
#define UDATE_KEY_GLASS_LOCK_BLE        0x2D
#define UDATE_KEY_PLUNGER_LOCK_BLE      0x2E
#define UPDATE_KEY_APPLE_12P_AM         0x2F  // No OTAU, only OTWU Alarm Module 
#define UPDATE_KEY_APPLE_12P_CM         0x30  // No OTAU, only OTWU Control Module


// MUST BE UNIQUE FOR EACH DEVICE TYPE
#define UPDATE_KEY_DEVICE_TYPE      UPDATE_KEY_APPLE_12P_AM

// Set pseudo EEPROM Reserved Size
#define EEPROM_MAX_SIZE 0x00002000

// Set Bootloader Reserved Size
#define BOOTLOADER_MAX_SIZE 0x00001000

#endif /* CONF_BOOTLOADER_H_ */
