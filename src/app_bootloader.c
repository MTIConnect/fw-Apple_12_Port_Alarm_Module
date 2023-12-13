/*
 * app_bootloader.c
 *
 * Created: 10/23/2023 12:01:35 PM
 *  Author: dmcdougall
 */ 
// # Get Versions
// #python dbgOTWU.py -p COM3 -d
//
// # Update Device
// #python dbgOTWU.py -p COM6 -m 150-00219 038-00085_Rev00018.bin 00008
// #python gwOTAU.py -m 150-00219 038-00085_Rev00014.bin 00014

#include <asf.h>
#include "app_bootloader.h"
#include "app_eeprom.h"
#include "string.h"
#include "sysTimer.h"

// Prototypes
static void updateRebootTimerHandler(SYS_Timer_t *timer);
static void updateTimeoutTimerHandler(SYS_Timer_t *timer);
void app_bootloader_start_loader(void);
uint8_t app_bootloader_load(uint16_t blockNumber, uint16_t crc, uint8_t *data);
static bool app_bootloader_verify_device_type(void);

// Variables
static volatile uint16_t currentBlockNumber;
static volatile uint16_t totalBlockNumber;
static volatile uint16_t fileCRC;
uint32_t curr_prog_addr = APP_STORE_ADDRESS;
struct nvm_config config;
static SYS_Timer_t updateTimeoutTimer;
static SYS_Timer_t updateRebootTimer;
bool firstBlock = true;

// ************************************************************************************************
// This function is called when updateRebootTimer expires
// ************************************************************************************************
static void updateRebootTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    system_reset();
}

// ************************************************************************************************
// This function is called when updateTimeoutTimer expires
// ************************************************************************************************
static void updateTimeoutTimerHandler(SYS_Timer_t *timer)
{
    UNUSED(timer);

    // TODO: what to do when it times out???
    // Use to reset which doesn't make sense now
}

void app_bootloader_start_loader(void)
{
    // Application Address
    curr_prog_addr = APP_STORE_ADDRESS;

    // Get NVM default configuration and load the same
    nvm_get_config_defaults(&config);
    config.manual_page_write = false;
    nvm_set_config(&config);

    // Get the number of blocks to be programmed
    totalBlockNumber   = app_eeprom_read_update_length();
    currentBlockNumber = 0;
    fileCRC            = 0;

    updateTimeoutTimer.interval = 5000;
    updateTimeoutTimer.mode     = SYS_TIMER_INTERVAL_MODE;
    updateTimeoutTimer.handler  = updateTimeoutTimerHandler;
    SYS_TimerStart(&updateTimeoutTimer);

    firstBlock = false;
}

uint8_t app_bootloader_load(uint16_t blockNumber, uint16_t crc, uint8_t *data)
{
    if (firstBlock)
    {
        app_bootloader_start_loader();
    }

    SYS_TimerRestart(&updateTimeoutTimer);

    if (currentBlockNumber > blockNumber)
    {
        // Duplicate block
        return UPDATE_STATUS_ERROR_DUPLICATE_BLOCK;
    }
    else if (currentBlockNumber < blockNumber)
    {
        // Missed a block
        return UPDATE_STATUS_ERROR_BLOCK_NUMBER;
    }

    // Check CRC
    uint16_t cs = 0;
    uint16_t blockCRC;
    uint8_t i = 0;
    while (i < NVMCTRL_PAGE_SIZE)
    {
        cs = cs + data[i];
        i++;
    }
    blockCRC = (1 + ~cs) & 0xffff;
    if (crc != blockCRC)
    {
        return UPDATE_STATUS_ERROR_BLOCK_CRC;
    }
    fileCRC = fileCRC + cs;

    // Check if it is first page of a row
    if ((curr_prog_addr & 0xFF) == 0)
    {
        /* Erase row */
        nvm_erase_row(curr_prog_addr);
    }
    // Write page to flash
    nvm_write_buffer(curr_prog_addr, data, NVMCTRL_PAGE_SIZE);

    // Increment the current programming address
    curr_prog_addr += NVMCTRL_PAGE_SIZE;

    currentBlockNumber++;

    if (currentBlockNumber >= totalBlockNumber)
    {
        fileCRC = (1 + ~fileCRC) & 0xffff;
        if (fileCRC != app_eeprom_read_update_crc())
        {
            return UPDATE_STATUS_ERROR_FILE_CRC;
        }

        if (!app_bootloader_verify_device_type())
        {
            return UPDATE_STATUS_ERROR_DEVICE_TYPE;
        }

        // Set Update Pending flag
        app_eeprom_write_update_pending(UPDATE_PENDING_FILE_VALID);

        if (SYS_TimerStarted(&updateTimeoutTimer))
        {
            SYS_TimerStop(&updateTimeoutTimer);
        }

        return UPDATE_STATUS_FILE_TRANSFER_COMPLETE;
    }

    return UPDATE_STATUS_READY;
}

static bool app_bootloader_verify_device_type(void)
{
    // Search Storage memory space to see if it contains a file with the correct device type key
    uint8_t updateDeviceTypeKey[] = {UPDATE_KEY_START_FLAG, UPDATE_KEY_DEVICE_TYPE, UPDATE_KEY_STOP_FLAG};
    uint8_t *memPtr               = (uint8_t *)(APP_STORE_ADDRESS);
    uint8_t *ptrEnd               = (uint8_t *)(APP_STORE_ADDRESS + APP_MAX_SIZE - UPDATE_KEY_START_FLAG_SIZE);

    while (memPtr < ptrEnd)
    {
        if (0 == memcmp(updateDeviceTypeKey, memPtr, UPDATE_KEY_START_FLAG_SIZE))
        {
            // Start flag found
            memPtr = memPtr + UPDATE_KEY_START_FLAG_SIZE;
            while (UPDATE_KEY_STOP_FLAG != *memPtr)
            {
                if (UPDATE_KEY_DEVICE_TYPE == *memPtr)
                {
                    // Valid key found
                    return true;
                }
                memPtr++;
            }

            // Invalid key found
            return false;
        }
        memPtr++;
    }

    // Start flag not found
    return false;
}

void app_bootloader_data_handler(uint8_t updateCMD, uint16_t updateBlockNumber, uint16_t updateCRC, uint8_t *updateData)
{
    uint8_t tempSts;

    switch (updateCMD)
    {
        case UPDATE_COMMAND_START:

#ifdef ALT_HW_BUILD  // This is the alternate bootloader where we have the file transfer in the bootloader itself instead of the
                     // appspace
            app_eeprom_write_update_pending(UPDATE_PENDING_PERFORM_UPDATE);
            system_reset();
#endif

            // Verify file size is not too large
            if ((updateBlockNumber * NVMCTRL_PAGE_SIZE) > APP_MAX_SIZE)
            {
                SEND_UPDATE_STATUS(0, UPDATE_STATUS_ERROR_FILE_SIZE);
            }
            else
            {
                // Set Update Length, CRC, Pending
                app_eeprom_write_update_length(updateBlockNumber);
                app_eeprom_write_update_crc(updateCRC);
                app_eeprom_write_update_pending(UPDATE_PENDING_NONE);
                SEND_UPDATE_STATUS(0, UPDATE_STATUS_READY);
                firstBlock = true;
            }
            break;
        case UPDATE_COMMAND_PACKET:
            tempSts = app_bootloader_load(updateBlockNumber, updateCRC, updateData);
            SEND_UPDATE_STATUS(updateBlockNumber, tempSts);
            break;
        case UPDATE_COMMAND_ABORT:
            SEND_UPDATE_STATUS(updateBlockNumber, UPDATE_STATUS_ERROR_ABORTED);
            break;
        case UPDATE_COMMAND_SWITCH:
            if (UPDATE_PENDING_FILE_VALID == app_eeprom_read_update_pending())
            {
                app_eeprom_write_update_pending(UPDATE_PENDING_PERFORM_UPDATE);

                // Reset to bootloader to switch in new app
                updateRebootTimer.interval = 500;
                updateRebootTimer.mode     = SYS_TIMER_INTERVAL_MODE;
                updateRebootTimer.handler  = updateRebootTimerHandler;
                SYS_TimerStart(&updateRebootTimer);
            }
            else
            {
                SEND_UPDATE_STATUS(0, UPDATE_STATUS_ERROR_SWITCH);
            }
            break;
    }
}

void app_bootloader_is_update_complete(void)
{
    if (UPDATE_PENDING_UPDATE_COMPLETE == app_eeprom_read_update_pending())
    {
        SEND_UPDATE_STATUS(0, UPDATE_STATUS_SWITCH_COMPLETE);
        app_eeprom_write_update_pending(UPDATE_PENDING_NONE);
    }
}