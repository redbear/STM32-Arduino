
#ifndef BLE_NANO_H_
#define BLE_NANO_H_

#include <stdint.h>
#include <string.h>

#include "spark_wiring_btstack.h"

#define INVALID_CONN_HANDLE 0xFFFF
#define NANO_NUM            4        //No more than 8, it's limited by btstack library's congfiguration.

typedef enum{

	NANO_DISCOVERY_IDLE,
	NANO_DISCOVERY_SERVICES,
	NANO_DISCOVERY_CHARS_OF_RBL_SERVICE,
	NANO_DISCOVERY_DESCRIPTOR_OF_TX_CHARS,
	NANO_DISCOVERY_DESCRIPTOR_OF_RX_CHARS,

	NANO_DISCOVERY_FINISH,
}NanoDiscoveryState_t;


void     nano_init(uint8_t num);
uint8_t  nano_checkUnconnected(void);
uint8_t  nano_getNumOfUndiscovered(void);

void     nano_setConnectHandle(uint8_t num, uint16_t handle);
uint16_t nano_getConnectHandle(uint8_t num);
uint8_t  nano_getNumAccordingHandle(uint16_t handle);
void     nano_setPeerAddr(uint8_t num, bd_addr_t addr, bd_addr_type_t addr_type);
uint8_t  nano_connect(uint8_t num);

void     nano_setDiscoveredState(uint8_t num, NanoDiscoveryState_t state);
uint8_t  nano_discoverService(uint8_t num);
void     nano_discoveredServiceResult(uint8_t num, gatt_client_service_t *service);
uint8_t  nano_discoverCharsOfService(uint8_t num);
void     nano_discoveredCharsResult(uint8_t num, gatt_client_characteristic_t *chars);
uint8_t  nano_discoverDescriptor(uint8_t num);
void     nano_discoverDescriptorResult(uint8_t num, gatt_client_characteristic_descriptor_t *descriptor);

uint8_t  nano_startNotify(uint8_t num);
uint8_t  nano_stopNotify(uint8_t num);
uint8_t  nano_write(uint8_t num, uint8_t *buf, uint8_t len);
uint8_t  nano_read(uint8_t num);

#endif



