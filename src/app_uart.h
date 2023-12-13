/*
 * app_uart.h
 *
 * Created: 10/18/2023 1:36:36 PM
 *  Author: dmcdougall
 */ 

#ifndef APP_UART_H_
#define APP_UART_H_

#define UPDATE_PACKET_LENGTH 68  // does not include start flag (0xaa)
#define DMA_BUFFER_SIZE      128
#define CIRC_BUFFER_SIZE     (DMA_BUFFER_SIZE * 2)
#define RX_BUFFER_SIZE       DMA_BUFFER_SIZE
#define COMMAND_LENGTH       2
#define TX_BUFFER_LENGTH     CIRC_BUFFER_SIZE

void app_uart_task(void);
void app_arraySN_to_strSN(uint8_t *strSN, uint8_t *arraySN);
void app_uart_store_usbdeviceid(uint16_t idVendor, uint16_t idProduct, uint16_t bcdDevice, const char *serial);
void app_uart_disable(void);
void app_uart_enable(void);
void app_uart_re_enable(void);
bool app_uartDebugRunning(void);  // check if UART Debug port is enabled
void app_uart_print_flash_key(int16_t index, uint8_t keyType, uint8_t *keySerial);
char *app_uart_cable_type_to_string(uint8_t type);

bool UART_TX(const char *transmitString, ...) __attribute__((format(gnu_printf, 1, 2)));
void UART_DBG_TX(const char *transmitString, ...) __attribute__((format(gnu_printf, 1, 2)));  // Variatic Function Prototype

#ifdef INCLUDE_ALL_DEBUG_FUNCTIONS
bool app_uart_get_useTetherMissedPingMaxAlt(void);
#endif

#endif  // APP_UART_H_
