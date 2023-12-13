/*
 * app_eeprom.c
 *
 * Created: 10/20/2023 4:59:18 PM
 *  Author: dmcdougall
 */ 


/*
 * app_eeprom.c
 *
 * EEPROM EMULATOR
 *
 * Created: 11/6/2015 11:40:42 AM
 *  Author: jcollier
 */

#include <asf.h>
#include "app_eeprom.h"
#include "app_uart.h"
#include "string.h"

#define APP_DEFAULT_ADDRESS      0x02
#define APP_DEFAULT_PANID        0x0001
#define APP_DEFAULT_CHANNEL      0x1A                // 0x0B (2405MHz) to 0x1A (2480MHz) (See ATSAMR21 datasheet Table 38-23)
#define APP_DEFAULT_SECURITY_KEY "MTICONNECTED4321"  // MUST be 16 characters
// Key for Wireshark 4d:54:49:43:4f:4e:4e:45:43:54:45:44:34:33:32:31

#define APP_EEPROM_MODEL_TYPE_DEFAULT APP_EEPROM_MODEL_TYPE_FACTORY

// EEPROM Memory Map
//	8kB EEPROM - Size set in fuse bits
//	60 Logical Pages (due to using the pseudo EEPROM driver)

//	EEPROM_PAGE_SIZE = NVMCTRL_PAGE_SIZE - EEPROM_HEADER_SIZE
//	60 = 64 - 4

#define EEPROM_PAGE_GENERAL                0
#define EEPROM_INDEX_OTAU_PENDING          0
#define EEPROM_INDEX_OTAU_LENGTH           1  // 01-02
#define EEPROM_INDEX_OTAU_CRC              3  // 03-04
#define EEPROM_INDEX_NETWORK_ADDRESS       5  // 05-06
#define EEPROM_INDEX_NETWORK_PANID         7  // 07-08
#define EEPROM_INDEX_NETWORK_CHANNEL       9
#define EEPROM_INDEX_NETWORK_KEY           10  // 10-25
#define EEPROM_INDEX_KEY_TABLE_VERSION     26
#define EEPROM_INDEX_NEXT_ROUTING_ADDR     27  // 27-28
#define EEPROM_INDEX_NEXT_NON_ROUTING_ADDR 29  // 29-30
#define EEPROM_INDEX_CONNECT_WANTED        31  // 31
#define EEPROM_INDEX_GENERAL_RESERVED      32  // 32-59

#define EEPROM_PAGE_KEY_TABLE       1
#define EEPROM_PAGE_KEY_TABLE_PAGES 26  // Support 252 keys
#define EEPROM_KEYS_PER_PAGE        10

#define EEPROM_PAGE_USER_CONFIG_OPTIONS   56  // This is the User Configuration (Volume, Alarms, etc)
#define EEPROM_INDEX_USER_CONFIG_OPTIONS  0   // 00-31
#define EEPROM_INDEX_USER_CONFIG_RESERVED 32  // 32-59

#define EEPROM_PAGE_PROG_KEY_TABLE       57
#define EEPROM_PAGE_PROG_KEY_TABLE_PAGES 1

#define EEPROM_PAGE_USER_SETTING 58  // This is the Mode (Standard/Connect/Default) not User Config!
#define EEPROM_PAGE_MODEL_TYPE   0
#define EEPROM_PAGE_MODE_RECORD  1

#define EEPROM_PAGE_ERROR_LOG  59
#define EEPROM_INDEX_ERROR_LOG 0  // 00-59

// Userpage definitions
#define APP_MODEL_NUMBER_PTR  0x00804040
#define APP_MODEL_VERSION_PTR 0x00804049

// Serial Number definitions
#define APP_SERIAL_NUMBER_0 0x0080A00C
#define APP_SERIAL_NUMBER_1 0x0080A040
#define APP_SERIAL_NUMBER_2 0x0080A044
#define APP_SERIAL_NUMBER_3 0x0080A048

// #define ENABLE_EEPROM_DEBUG_MSGS 1 // Uncomment to print out Debug messages

COMPILER_PACK_SET(1)
typedef struct eepromKey_t
{
    uint8_t keySerialNumber[4];//RFID_LENGTH_UID];
    uint8_t keyType;
    uint8_t unused;
} eepromKey_t;

typedef union eepromKeyPage_t
{
    eepromKey_t eepromKey[EEPROM_KEYS_PER_PAGE];
    uint8_t data[EEPROM_PAGE_SIZE];
} eepromKeyPage_t;
COMPILER_PACK_RESET()

static uint8_t serialNumber[16];
static uint8_t aesKey[16];
static uint8_t errorLog[61];

////////////////////////////////////////////////////////////////
// Functions

void app_eeprom_init(void)
{
    // Setup EEPROM emulator service
    enum status_code error_code = eeprom_emulator_init();
    if (error_code == STATUS_ERR_NO_MEMORY)
    {
        while (true)
        {
            // No EEPROM section has been set in the device's fuses
        }
    }
    else if (error_code != STATUS_OK)
    {
        // Erase the emulated EEPROM memory (assume it is unformatted or irrecoverably corrupt)
        eeprom_emulator_erase_memory();
        eeprom_emulator_init();
        // Bootloader should have already done this
    }
}

uint16_t app_eeprom_read_address(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint16_t tempAddr = ((page_data[EEPROM_INDEX_NETWORK_ADDRESS] << 8) + page_data[EEPROM_INDEX_NETWORK_ADDRESS + 1]);

    // Check to see if the address is valid
    if (tempAddr >= 0xFFFF)
    {
        tempAddr = APP_DEFAULT_ADDRESS;
        app_eeprom_write_address(APP_DEFAULT_ADDRESS);
    }
    return tempAddr;
}

void app_eeprom_write_address(uint16_t nAddr)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint16_t tempAddr = ((page_data[EEPROM_INDEX_NETWORK_ADDRESS] << 8) + page_data[EEPROM_INDEX_NETWORK_ADDRESS + 1]);

    if (tempAddr != nAddr)
    {
        page_data[EEPROM_INDEX_NETWORK_ADDRESS]     = nAddr >> 8;
        page_data[EEPROM_INDEX_NETWORK_ADDRESS + 1] = nAddr & 0xff;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

uint16_t app_eeprom_read_panid(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint16_t tempPanID = ((page_data[EEPROM_INDEX_NETWORK_PANID] << 8) + page_data[EEPROM_INDEX_NETWORK_PANID + 1]);
    if (tempPanID >= 0xffff)
    {
        app_eeprom_write_panid(APP_DEFAULT_PANID);
        tempPanID = APP_DEFAULT_PANID;
    }
    return tempPanID;
}

void app_eeprom_write_panid(uint16_t ePanID)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint16_t tempPanID = ((page_data[EEPROM_INDEX_NETWORK_PANID] << 8) + page_data[EEPROM_INDEX_NETWORK_PANID + 1]);

    if (tempPanID != ePanID)
    {
        page_data[EEPROM_INDEX_NETWORK_PANID]     = ePanID >> 8;
        page_data[EEPROM_INDEX_NETWORK_PANID + 1] = ePanID & 0xff;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

uint8_t app_eeprom_read_channel(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint8_t tempChannel = page_data[EEPROM_INDEX_NETWORK_CHANNEL];

    if ((tempChannel >= 0x1B) || (tempChannel <= 0x0A))
    {
        app_eeprom_write_channel(APP_DEFAULT_CHANNEL);
        tempChannel = APP_DEFAULT_CHANNEL;
    }
    return tempChannel;
}

void app_eeprom_write_channel(uint8_t eChannel)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint8_t tempChannel = page_data[EEPROM_INDEX_NETWORK_CHANNEL];

    if (tempChannel != eChannel)
    {
        page_data[EEPROM_INDEX_NETWORK_CHANNEL] = eChannel;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

uint8_t app_eeprom_read_connect_wanted(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint8_t currentConnectWanted = page_data[EEPROM_INDEX_CONNECT_WANTED];

    if (currentConnectWanted > APP_EEPROM_MODEL_TYPE_FACTORY)
    {
        // app_eeprom_write_connect_wanted(APP_EEPROM_MODEL_TYPE_DEFAULT);
 //       app_rfid_state_set_mode(APP_EEPROM_MODEL_TYPE_FACTORY);
        currentConnectWanted = APP_EEPROM_MODEL_TYPE_FACTORY;
    }

    // return(APP_EEPROM_MODEL_TYPE_NON_CONNECTED != currentConnectWanted);
    return currentConnectWanted;
}

void app_eeprom_write_connect_wanted(uint8_t newConnectWanted)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);
    uint8_t currentConnectWanted = page_data[EEPROM_INDEX_CONNECT_WANTED];

    // Check for valid values
    if ((newConnectWanted != APP_EEPROM_MODEL_TYPE_NON_CONNECTED) && (newConnectWanted != APP_EEPROM_MODEL_TYPE_CONNECTED) &&
        (newConnectWanted != APP_EEPROM_MODEL_TYPE_FACTORY))
    {
        newConnectWanted = APP_EEPROM_MODEL_TYPE_FACTORY;
    }

    // Going from connect to a non-connect mode
    if ((APP_EEPROM_MODEL_TYPE_CONNECTED == currentConnectWanted) &&
        ((APP_EEPROM_MODEL_TYPE_FACTORY == newConnectWanted) || (APP_EEPROM_MODEL_TYPE_NON_CONNECTED == newConnectWanted)))
    {
#ifdef ENABLE_EEPROM_DEBUG_MSGS
        UART_DBG_TX("Leaving Mesh Network\n");
#endif
 //       app_lwmesh_send_error("0x1001 Leaving Mesh Network");
    }

    if (currentConnectWanted != newConnectWanted)
    {
        page_data[EEPROM_INDEX_CONNECT_WANTED] = newConnectWanted;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

uint8_t *app_eeprom_read_key(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    memcpy(aesKey, &page_data[EEPROM_INDEX_NETWORK_KEY], sizeof(aesKey));

    bool keyNotSet = true;
    for (uint8_t i = 0; i < sizeof(aesKey); i++)
    {
        if (aesKey[i] != 0xff)
        {
            keyNotSet = false;
            break;
        }
    }
    if (keyNotSet)
    {
        app_eeprom_write_key((uint8_t *)APP_DEFAULT_SECURITY_KEY);
        memcpy(aesKey, (uint8_t *)APP_DEFAULT_SECURITY_KEY, sizeof(aesKey));
    }
    return aesKey;
}

void app_eeprom_write_key(uint8_t *eKey)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    memcpy(aesKey, &page_data[EEPROM_INDEX_NETWORK_KEY], sizeof(aesKey));

    bool keyMatch = true;
    for (uint8_t iKey = 0; iKey < sizeof(aesKey); iKey++)
    {
        if (eKey[iKey] != aesKey[iKey])
        {
            keyMatch = false;
            break;
        }
    }
    if (!keyMatch)
    {
        memcpy(&page_data[EEPROM_INDEX_NETWORK_KEY], eKey, sizeof(aesKey));
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

char *app_eeprom_read_model_number(void)
{
    return (char *)APP_MODEL_NUMBER_PTR;
}

char *app_eeprom_read_model_version(void)
{
    return (char *)APP_MODEL_VERSION_PTR;
}

uint8_t *app_eeprom_read_serial_number(void)
{
    uint32_t *ptr;

    ptr = (uint32_t *)APP_SERIAL_NUMBER_0;

    serialNumber[0] = (*ptr >> 24) & 0xFF;
    serialNumber[1] = (*ptr >> 16) & 0xFF;
    serialNumber[2] = (*ptr >> 8) & 0xFF;
    serialNumber[3] = (*ptr >> 0) & 0xFF;

    ptr = (uint32_t *)APP_SERIAL_NUMBER_1;

    serialNumber[4] = (*ptr >> 24) & 0xFF;
    serialNumber[5] = (*ptr >> 16) & 0xFF;
    serialNumber[6] = (*ptr >> 8) & 0xFF;
    serialNumber[7] = (*ptr >> 0) & 0xFF;

    ptr = (uint32_t *)APP_SERIAL_NUMBER_2;

    serialNumber[8]  = (*ptr >> 24) & 0xFF;
    serialNumber[9]  = (*ptr >> 16) & 0xFF;
    serialNumber[10] = (*ptr >> 8) & 0xFF;
    serialNumber[11] = (*ptr >> 0) & 0xFF;

    ptr = (uint32_t *)APP_SERIAL_NUMBER_3;

    serialNumber[12] = (*ptr >> 24) & 0xFF;
    serialNumber[13] = (*ptr >> 16) & 0xFF;
    serialNumber[14] = (*ptr >> 8) & 0xFF;
    serialNumber[15] = (*ptr >> 0) & 0xFF;

    return serialNumber;
}

uint8_t app_eeprom_read_update_pending(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    return page_data[EEPROM_INDEX_OTAU_PENDING];
}

void app_eeprom_write_update_pending(uint8_t otauPending)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    if (otauPending != page_data[EEPROM_INDEX_OTAU_PENDING])
    {
        page_data[EEPROM_INDEX_OTAU_PENDING] = otauPending;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

uint16_t app_eeprom_read_update_length(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    return ((page_data[EEPROM_INDEX_OTAU_LENGTH] << 8) + page_data[EEPROM_INDEX_OTAU_LENGTH + 1]);
}

void app_eeprom_write_update_length(uint16_t otauLength)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint16_t storedLength = (page_data[EEPROM_INDEX_OTAU_LENGTH] << 8) + page_data[EEPROM_INDEX_OTAU_LENGTH + 1];

    if (storedLength != otauLength)
    {
        page_data[EEPROM_INDEX_OTAU_LENGTH]     = otauLength >> 8;
        page_data[EEPROM_INDEX_OTAU_LENGTH + 1] = otauLength & 0xff;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

uint16_t app_eeprom_read_update_crc(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    return ((page_data[EEPROM_INDEX_OTAU_CRC] << 8) + page_data[EEPROM_INDEX_OTAU_CRC + 1]);
}

void app_eeprom_write_update_crc(uint16_t otauCrc)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);

    uint16_t storedCrc = (page_data[EEPROM_INDEX_OTAU_CRC] << 8) + page_data[EEPROM_INDEX_OTAU_CRC + 1];

    if (storedCrc != otauCrc)
    {
        page_data[EEPROM_INDEX_OTAU_CRC]     = otauCrc >> 8;
        page_data[EEPROM_INDEX_OTAU_CRC + 1] = otauCrc & 0xff;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

int16_t app_eeprom_key_table_add(uint8_t *kSN, uint8_t kType)
{
    eepromKeyPage_t keyPage;
    int16_t keyIndex = 0;

    // Check if the key is already in the list
    for (uint8_t pageIndex = 0; pageIndex < EEPROM_PAGE_KEY_TABLE_PAGES; pageIndex++)
    {
        eeprom_emulator_read_page(EEPROM_PAGE_KEY_TABLE + pageIndex, keyPage.data);

        for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
        {
//             if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//                 (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//             {
//                 // Reached the end of the list without finding the key
//                 pageIndex = EEPROM_PAGE_KEY_TABLE_PAGES;
//                 break;
//             }

//             if (0 == memcmp(kSN, keyPage.eepromKey[keyPageIndex].keySerialNumber, RFID_LENGTH_UID))
//             {
//                 // Key already in list
//                 return keyIndex;
//             }
            keyIndex++;
        }
    }
    keyIndex = 0;

    // Check if there is an empty slot for the key
    for (uint8_t pageIndex = 0; pageIndex < EEPROM_PAGE_KEY_TABLE_PAGES; pageIndex++)
    {
        eeprom_emulator_read_page(EEPROM_PAGE_KEY_TABLE + pageIndex, keyPage.data);

        for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
        {
//             if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//                 (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//             {
//                 // Reached an empty slot
//                 memcpy(keyPage.eepromKey[keyPageIndex].keySerialNumber, kSN, RFID_LENGTH_UID);
//                 keyPage.eepromKey[keyPageIndex].keyType = kType;
//                 eeprom_emulator_write_page(EEPROM_PAGE_KEY_TABLE + pageIndex, keyPage.data);
//                 eeprom_emulator_commit_page_buffer();
//                 return keyIndex;
//             }
            keyIndex++;
        }
    }

    // List is full
    return -1;
}

int16_t app_eeprom_key_table_count_valid(void)
{
    eepromKeyPage_t keyPage;
    int16_t keyIndex = 0;

    for (uint8_t pageIndex = 0; pageIndex < EEPROM_PAGE_KEY_TABLE_PAGES; pageIndex++)
    {
        eeprom_emulator_read_page(EEPROM_PAGE_KEY_TABLE + pageIndex, keyPage.data);

        for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
        {
//             if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//                 (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//             {
//                 // Invalid card, we're done!
//                 pageIndex = EEPROM_PAGE_KEY_TABLE_PAGES;
//                 break;
//             }
            keyIndex++;
        }
    }
    // Return the number of keys found.
    return keyIndex;
}

void app_eeprom_key_table_list_valid(void)
{
    eepromKeyPage_t keyPage;
//    int16_t keyIndex = 0;

    for (uint8_t pageIndex = 0; pageIndex < EEPROM_PAGE_KEY_TABLE_PAGES; pageIndex++)
    {
        eeprom_emulator_read_page(EEPROM_PAGE_KEY_TABLE + pageIndex, keyPage.data);

        for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
        {
//             if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//                 (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//             {
//                 // Invalid card, we're done!
//                 return;
//             }
//             app_uart_print_flash_key(keyIndex++, keyPage.eepromKey[keyPageIndex].keyType,
//                                      keyPage.eepromKey[keyPageIndex].keySerialNumber);
        }
    }
}

int16_t app_eeprom_key_table_query(uint8_t *kSN)
{
    eepromKeyPage_t keyPage;
    int16_t keyIndex = 0;

    for (uint8_t pageIndex = 0; pageIndex < EEPROM_PAGE_KEY_TABLE_PAGES; pageIndex++)
    {
        eeprom_emulator_read_page(EEPROM_PAGE_KEY_TABLE + pageIndex, keyPage.data);

        for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
        {
//             if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//                 (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//             {
//                 // Reached end of table and didn't find the key
//                 return -1;
//             }

//             if (0 == memcmp(kSN, keyPage.eepromKey[keyPageIndex].keySerialNumber, RFID_LENGTH_UID))
//             {
//                 return keyIndex;
//             }
            keyIndex++;
        }
    }
    return -1;
}

void app_eeprom_key_table_clear(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    bool writePage;
    bool commitBuffer = false;

    app_eeprom_key_table_version_clear();

    for (uint8_t i = 0; i < EEPROM_PAGE_KEY_TABLE_PAGES; i++)
    {
        writePage = false;
        eeprom_emulator_read_page(i + EEPROM_PAGE_KEY_TABLE, page_data);
        for (uint8_t index = 0; index < EEPROM_PAGE_SIZE; index++)
        {
            if (page_data[index] != 0xff)
            {
                page_data[index] = 0xff;
                writePage        = true;
                commitBuffer     = true;
            }
        }
        if (writePage)
        {
            eeprom_emulator_write_page(i + EEPROM_PAGE_KEY_TABLE, page_data);
        }
    }
    if (commitBuffer)
    {
        eeprom_emulator_commit_page_buffer();
    }
}

int16_t app_eeprom_progKey_table_add(uint8_t *kSN, uint8_t kType)
{
    eepromKeyPage_t keyPage;
    int16_t keyIndex = 0;

    // Check if the key is already in the list
    eeprom_emulator_read_page(EEPROM_PAGE_PROG_KEY_TABLE, keyPage.data);

    for (keyIndex = 0; keyIndex < EEPROM_KEYS_PER_PAGE; keyIndex++)
    {
//         if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyIndex].keyType) ||
//             (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyIndex].keyType))
//         {
//             // Reached the end of the list without finding the key
//             break;
//         }
//         if (0 == memcmp(kSN, keyPage.eepromKey[keyIndex].keySerialNumber, RFID_LENGTH_UID))
//         {
//             // Key already in list
//             return keyIndex;
//         }
        keyIndex++;
    }

    if (keyIndex > 2)
    {
        // The table is full of cards that don't match.  We shouldn't ever get
        // here.  Let the calling party figure out what to do.
        return -1;
    }

    // Save Key
 //   memcpy(keyPage.eepromKey[keyIndex].keySerialNumber, kSN, RFID_LENGTH_UID);
//    keyPage.eepromKey[keyIndex].keyType = kType;

    eeprom_emulator_write_page(EEPROM_PAGE_PROG_KEY_TABLE, keyPage.data);
    eeprom_emulator_commit_page_buffer();

    // Indicate our success.
    return keyIndex;
}

int16_t app_eeprom_progKey_table_count_valid(void)
{
    eepromKeyPage_t keyPage;
    int16_t keyIndex = 0;

    eeprom_emulator_read_page(EEPROM_PAGE_PROG_KEY_TABLE, keyPage.data);

    for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
    {
//         if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//             (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//         {
//             // Invalid card, we're done!
//             break;
//         }
        keyIndex++;
    }
    // Return the number of keys found.
    return keyIndex;
}

void app_eeprom_progKey_table_list_valid(void)
{
    eepromKeyPage_t keyPage;
//    int16_t keyIndex = 0;

    eeprom_emulator_read_page(EEPROM_PAGE_PROG_KEY_TABLE, keyPage.data);

//     for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
//     {
//         if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//             (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//         {
//             // Invalid card, we're done!
//             return;
//         }
// 
//         app_uart_print_flash_key(keyIndex++, keyPage.eepromKey[keyPageIndex].keyType,
//                                  keyPage.eepromKey[keyPageIndex].keySerialNumber);
//     }
}

int16_t app_eeprom_progKey_table_query(uint8_t *kSN)
{
    eepromKeyPage_t keyPage;
    int16_t keyIndex = 0;

    eeprom_emulator_read_page(EEPROM_PAGE_PROG_KEY_TABLE, keyPage.data);

    for (uint8_t keyPageIndex = 0; keyPageIndex < EEPROM_KEYS_PER_PAGE; keyPageIndex++)
    {
//         if ((RFID_CARD_TYPE_UNKNOWN == keyPage.eepromKey[keyPageIndex].keyType) ||
//             (RFID_CARD_TYPE_ERASED == keyPage.eepromKey[keyPageIndex].keyType))
//         {
//             // Reached end of table and didn't find the key
//             return -1;
//         }
// 
//         if (0 == memcmp(kSN, keyPage.eepromKey[keyPageIndex].keySerialNumber, RFID_LENGTH_UID))
//         {
//             return keyIndex;
//         }
        keyIndex++;
    }

    // Not found
    return -1;
}

void app_eeprom_progKey_table_clear(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    bool writePage;
    bool commitBuffer = false;

    writePage = false;
    eeprom_emulator_read_page(EEPROM_PAGE_PROG_KEY_TABLE, page_data);
    for (uint8_t index = 0; index < EEPROM_PAGE_SIZE; index++)
    {
        // Got the page of data. Make sure that every byte is erased.
        if (page_data[index] != 0xff)
        {
            page_data[index] = 0xff;
            writePage        = true;
            commitBuffer     = true;
        }
    }
    if (writePage)
    {
        eeprom_emulator_write_page(EEPROM_PAGE_PROG_KEY_TABLE, page_data);
    }
    if (commitBuffer)
    {
        eeprom_emulator_commit_page_buffer();
    }
}

uint8_t app_eeprom_read_key_table_version(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);
    uint8_t tableVersion = page_data[EEPROM_INDEX_KEY_TABLE_VERSION];
    return tableVersion;
}

void app_eeprom_write_key_table_version(uint8_t tableVersion)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_GENERAL, page_data);
    if (tableVersion != page_data[EEPROM_INDEX_KEY_TABLE_VERSION])
    {
        page_data[EEPROM_INDEX_KEY_TABLE_VERSION] = tableVersion;
        eeprom_emulator_write_page(EEPROM_PAGE_GENERAL, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

void app_eeprom_key_table_version_clear(void)
{
    app_eeprom_write_key_table_version(0xFF);
}

char *app_eeprom_read_error(void)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_ERROR_LOG, page_data);
    memset(errorLog, 0, sizeof(errorLog));

    if (page_data[0] != 0xFF)
    {
        strncpy((char *)errorLog, (char *)page_data, EEPROM_PAGE_SIZE);

        // Clear log
        memset(page_data, 0xFF, EEPROM_PAGE_SIZE);
        eeprom_emulator_write_page(EEPROM_PAGE_ERROR_LOG, page_data);
        eeprom_emulator_commit_page_buffer();
    }
    return (char *)errorLog;
}

void app_eeprom_write_error(char *errorMsg)
{
    uint8_t page_data[EEPROM_PAGE_SIZE];
    eeprom_emulator_read_page(EEPROM_PAGE_ERROR_LOG, page_data);
    if (0 != strncmp((char *)page_data, (char *)errorMsg, EEPROM_PAGE_SIZE))
    {
        memset(page_data, 0, EEPROM_PAGE_SIZE);
        strncpy((char *)page_data, errorMsg, EEPROM_PAGE_SIZE);
        eeprom_emulator_write_page(EEPROM_PAGE_ERROR_LOG, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}

/**  User Setting  **/
void app_eeprom_read_modelType(uint8_t *ul_modeType)
{
    uint8_t page_data[EEPROM_PAGE_SIZE] = {0};

    eeprom_emulator_read_page(EEPROM_PAGE_USER_SETTING, page_data);

    if (page_data[EEPROM_PAGE_MODEL_TYPE] != 0xFF)
        *ul_modeType = page_data[EEPROM_PAGE_MODEL_TYPE];
    else
        *ul_modeType = APP_EEPROM_MODEL_TYPE_CONNECTED;
}

void app_eeprom_write_modelType(uint8_t ul_modelType)
{
    uint8_t page_data[EEPROM_PAGE_SIZE] = {0};

    eeprom_emulator_read_page(EEPROM_PAGE_USER_SETTING, page_data);
    page_data[EEPROM_PAGE_MODEL_TYPE] = ul_modelType;
    eeprom_emulator_write_page(EEPROM_PAGE_USER_SETTING, page_data);
    eeprom_emulator_commit_page_buffer();
}

// ********************************************************************
// This is the User Configuration (Volume, Alarms, Latch State etc)
// #define EEPROM_PAGE_USER_CONFIG_OPTIONS		56
// #define EEPROM_INDEX_USER_CONFIG_OPTIONS		0
// #define EEPROM_BYTES_USER_CONFIG_OPTIONS		32
// ********************************************************************

void app_eeprom_read_userConfig(uint8_t *userConfig)
{
    uint8_t page_data[EEPROM_PAGE_SIZE] = {0};
    eeprom_emulator_read_page(EEPROM_PAGE_USER_CONFIG_OPTIONS, page_data);

    memcpy(userConfig, page_data, EEPROM_BYTES_USER_CONFIG_OPTIONS);
}

void app_eeprom_write_userConfig(uint8_t *userConfiguration)
{
    uint8_t page_data[EEPROM_PAGE_SIZE] = {0};
    eeprom_emulator_read_page(EEPROM_PAGE_USER_CONFIG_OPTIONS, page_data);

    if (0 == memcmp(userConfiguration, page_data, EEPROM_BYTES_USER_CONFIG_OPTIONS))
    {
#ifdef ENABLE_EEPROM_DEBUG_MSGS
        UART_DBG_TX("No Change Detected - User Config not overwritten\n");
#endif
    }
    else
    {
#ifdef ENABLE_EEPROM_DEBUG_MSGS
        UART_DBG_TX("Change Detected - Writing User Config\n");
#endif
        memcpy(page_data, userConfiguration, EEPROM_BYTES_USER_CONFIG_OPTIONS);
        eeprom_emulator_write_page(EEPROM_PAGE_USER_CONFIG_OPTIONS, page_data);
        eeprom_emulator_commit_page_buffer();
    }
}
