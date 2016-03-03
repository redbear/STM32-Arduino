
#ifndef BTSTACK_HAL_H_
#define BTSTACK_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "btstack.h"
#include "btstack_chipset_bcm.h"
#include "btstack_config.h"
#include "hci_dump.h"

/**@brief BLE status */
typedef enum BLEStatus {
    BLE_STATUS_OK,
    BLE_STATUS_DONE,    // e.g. for service or characteristic discovery done
    BLE_STATUS_CONNECTION_TIMEOUT,
    BLE_STATUS_CONNECTION_ERROR,
    BLE_STATUS_OTHER_ERROR
} BLEStatus_t;


/**@brief BLE advertising report data. */
typedef struct{
    uint8_t peerAddrType;
    bd_addr_t  peerAddr;
    int     rssi;
    uint8_t advEventType;
    uint8_t advDataLen;
    uint8_t advData[31];
}advertisementReport_t;

typedef struct{
    uint16_t adv_int_min;
    uint16_t adv_int_max;
    uint8_t adv_type;
    uint8_t dir_addr_type;
    uint8_t dir_addr[BD_ADDR_LEN];
    uint8_t channel_map;
    uint8_t filter_policy;
}advParams_t;

/**@brief Device API */
void hal_btstack_init(void);
void hal_btstack_deInit(void);
void hal_btstack_loop_execute(void);

void     hal_btstack_setTimer(btstack_timer_source_t *ts, uint32_t timeout_in_ms);
void     hal_btstack_setTimerHandler(btstack_timer_source_t *ts, void (*process)(btstack_timer_source_t *_ts));
void     hal_btstack_addTimer(btstack_timer_source_t *timer);
int      hal_btstack_removeTimer(btstack_timer_source_t *timer);
uint32_t hal_btstack_getTimeMs(void);

void hal_btstack_debugLogger(uint8_t flag);
void hal_btstack_debugError(uint8_t flag);
void hal_btstack_enablePacketLogger(void);

/**@brief Gap API */
void hal_btstack_getAdvertisementAddr(uint8_t *addr_type, bd_addr_t addr);
void hal_btstack_setRandomAddressMode(gap_random_address_type_t random_address_type);
void hal_btstack_setRandomAddr(bd_addr_t addr);
void hal_btstack_setPublicBdAddr(bd_addr_t addr);
void hal_btstack_setLocalName(const char *local_name);
void hal_btstack_setAdvParams(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t adv_type, uint8_t dir_addr_type, bd_addr_t dir_addr, uint8_t channel_map, uint8_t filter_policy);
void hal_btstack_setAdvData(uint16_t size, uint8_t *data);

void hal_btstack_startAdvertising(void);
void hal_btstack_stopAdvertising(void);

void hal_btstack_setConnectedCallback(void (*callback)(BLEStatus_t status, uint16_t handle));
void hal_btstack_setDisconnectedCallback(void (*callback)(uint16_t handle));

void hal_btstack_disconnect(uint16_t handle);
uint8_t hal_btstack_connect(bd_addr_t addr, bd_addr_type_t type);

void hal_btstack_setConnParamsRange(le_connection_parameter_range_t range);

/**@brief Gatt server API */
int  hal_btstack_attServerCanSend(void);
int  hal_btstack_attServerSendNotify(uint16_t value_handle, uint8_t *value, uint16_t length);
int  hal_btstack_attServerSendIndicate(uint16_t value_handle, uint8_t *value, uint16_t length);

void hal_btstack_setGattCharsRead(uint16_t (*cb)(uint16_t handle, uint8_t *buffer, uint16_t buffer_size));
void hal_btstack_setGattCharsWrite(int (*cb)(uint16_t handle, uint8_t *buffer, uint16_t buffer_size));

void hal_btstack_addServiceUUID16bits(uint16_t uuid);
void hal_btstack_addServiceUUID128bits(uint8_t *uuid);

uint16_t hal_btstack_addCharsUUID16bits(uint16_t uuid, uint16_t flags, uint8_t *data, uint16_t data_len);
uint16_t hal_btstack_addCharsDynamicUUID16bits(uint16_t uuid, uint16_t flags, uint8_t *data, uint16_t data_len);
uint16_t hal_btstack_addCharsUUID128bits(uint8_t *uuid, uint16_t flags, uint8_t *data, uint16_t data_len);
uint16_t hal_btstack_addCharsDynamicUUID128bits(uint8_t *uuid, uint16_t flags, uint8_t *data, uint16_t data_len);


/**@brief Gatt client API */
void hal_btstack_startScanning(void);
void hal_btstack_stopScanning(void);

void hal_btstack_setBLEAdvertisementCallback(void (*cb)(advertisementReport_t *advertisement_report));


#ifdef __cplusplus
}
#endif

#endif
