/*
 * app_eeprom.h
 *
 * Created: 10/20/2023 4:59:09 PM
 *  Author: dmcdougall
 */ 


/*
 * app_eeprom.h
 *
 * Created: 11/6/2015 11:40:14 AM
 *  Author: jcollier
 */

#ifndef APP_EEPROM_H_
#define APP_EEPROM_H_

#define AES_KEY_BYTE_SIZE         16
#define SERIAL_NUMBER_BYTE_SIZE   16
#define SERIAL_NUMBER_STRING_SIZE 32

#define MODEL_VERSION_SIZE               9
#define HARDWARE_VERSION_SIZE            7
#define FIRMWARE_VERSION_SIZE            7
#define EEPROM_BYTES_USER_CONFIG_OPTIONS 32

enum modelType_t
{
    APP_EEPROM_MODEL_TYPE_NON_CONNECTED = 0,
    APP_EEPROM_MODEL_TYPE_CONNECTED     = 1,
    APP_EEPROM_MODEL_TYPE_FACTORY       = 2
};

void app_eeprom_init(void);
uint16_t app_eeprom_read_address(void);
void app_eeprom_write_address(uint16_t eAddress);
uint16_t app_eeprom_read_panid(void);
void app_eeprom_write_panid(uint16_t ePanID);
uint8_t app_eeprom_read_channel(void);
void app_eeprom_write_channel(uint8_t eChannel);
uint8_t *app_eeprom_read_key(void);
void app_eeprom_write_key(uint8_t *eKey);

char *app_eeprom_read_model_number(void);
char *app_eeprom_read_model_version(void);
uint8_t *app_eeprom_read_serial_number(void);

uint8_t app_eeprom_read_update_pending(void);
void app_eeprom_write_update_pending(uint8_t otauPending);
uint16_t app_eeprom_read_update_length(void);
void app_eeprom_write_update_length(uint16_t otauLength);
uint16_t app_eeprom_read_update_crc(void);
void app_eeprom_write_update_crc(uint16_t otauCrc);

uint8_t app_eeprom_read_connect_wanted(void);
void app_eeprom_write_connect_wanted(uint8_t eConnectWanted);

int16_t app_eeprom_key_table_add(uint8_t *kSN, uint8_t kType);
int16_t app_eeprom_key_table_query(uint8_t *kSN);
void app_eeprom_key_table_clear(void);
int16_t app_eeprom_key_table_count_valid(void);
void app_eeprom_key_table_list_valid(void);

int16_t app_eeprom_progKey_table_add(uint8_t *kSN, uint8_t kType);
int16_t app_eeprom_progKey_table_query(uint8_t *kSN);
void app_eeprom_progKey_table_clear(void);
int16_t app_eeprom_progKey_table_count_valid(void);
void app_eeprom_progKey_table_list_valid(void);

uint8_t app_eeprom_read_key_table_version(void);
void app_eeprom_write_key_table_version(uint8_t tableVersion);
void app_eeprom_key_table_version_clear(void);

char *app_eeprom_read_error(void);
void app_eeprom_write_error(char *error);

/**  User Mode Setting  **/
void app_eeprom_read_modelType(uint8_t *ul_modeType);
void app_eeprom_write_modelType(uint8_t ul_modelType);

// User Config
void app_eeprom_read_userConfig(uint8_t *userConfig);
void app_eeprom_write_userConfig(uint8_t *userConfiguration);

#endif /* APP_EEPROM_H_ */
