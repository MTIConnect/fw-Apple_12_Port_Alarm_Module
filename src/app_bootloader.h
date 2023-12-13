/*
 * app_bootloader.h
 *
 * Created: 10/23/2023 12:01:46 PM
 *  Author: dmcdougall
 */ 


#ifndef APP_BOOTLOADER_H_
#define APP_BOOTLOADER_H_

#include "conf_bootloader.h"

#define APP_MAX_SIZE      ((FLASH_SIZE - BOOTLOADER_MAX_SIZE - EEPROM_MAX_SIZE) / 2)
#define APP_MAX_BLOCK_NUM (APP_MAX_SIZE / FLASH_PAGE_SIZE)

#define APP_START_ADDRESS    (FLASH_ADDR + BOOTLOADER_MAX_SIZE)
#define APP_STORE_ADDRESS    (APP_START_ADDRESS + APP_MAX_SIZE)
#define EEPROM_START_ADDRESS (FLASH_ADDR + FLASH_SIZE - EEPROM_MAX_SIZE)

// Update Command
#define UPDATE_COMMAND_START  0
#define UPDATE_COMMAND_PACKET 1
#define UPDATE_COMMAND_ABORT  2
#define UPDATE_COMMAND_SWITCH 3

// Update Pending
#define UPDATE_PENDING_NONE            0
#define UPDATE_PENDING_PERFORM_UPDATE  1
#define UPDATE_PENDING_UPDATE_COMPLETE 2
#define UPDATE_PENDING_FILE_VALID      3

// Update Status
#define UPDATE_STATUS_BUSY                   0x0
#define UPDATE_STATUS_READY                  0x1
#define UPDATE_STATUS_FILE_TRANSFER_COMPLETE 0x2
#define UPDATE_STATUS_SWITCH_COMPLETE        0x3
#define UPDATE_STATUS_ERROR_BLOCK_CRC        0x4
#define UPDATE_STATUS_ERROR_FILE_CRC         0x5
#define UPDATE_STATUS_ERROR_TIMEOUT          0x6
#define UPDATE_STATUS_ERROR_ABORTED          0x7
#define UPDATE_STATUS_ERROR_LOW_BATTERY      0x8
#define UPDATE_STATUS_ERROR_BLOCK_NUMBER     0x9
#define UPDATE_STATUS_ERROR_DUPLICATE_BLOCK  0xA
#define UPDATE_STATUS_ERROR_DEVICE_TYPE      0xB
#define UPDATE_STATUS_ERROR_FILE_SIZE        0xC
#define UPDATE_STATUS_ERROR_SWITCH           0xD

#define UPDATE_KEY_STOP_FLAG       0xff
#define UPDATE_KEY_START_FLAG_SIZE 16
#define UPDATE_KEY_START_FLAG      0x4f, 0x54, 0x41, 0x55, 0x4b, 0x45, 0x59, 0x53, 0x54, 0x41, 0x52, 0x54, 0x46, 0x4c, 0x41, 0x47

void app_bootloader_data_handler(uint8_t updateCMD, uint16_t updateBlockNumber, uint16_t updateCRC, uint8_t *updateData);
void app_bootloader_is_update_complete(void);

#define OTAU_PACKET_LENGTH 84  // does not include start flag (0xaa)

#endif /* APP_BOOTLOADER_H_ */
