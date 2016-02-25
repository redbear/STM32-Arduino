
#ifndef BTSTACK_HAL_H_
#define BTSTACK_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

// MARK: Attribute Property Flags
#define PROPERTY_BROADCAST                  0x01
#define PROPERTY_READ                       0x02
#define PROPERTY_WRITE_WITHOUT_RESPONSE     0x04
#define PROPERTY_WRITE                      0x08
#define PROPERTY_NOTIFY                     0x10
#define PROPERTY_INDICATE                   0x20
#define PROPERTY_AUTHENTICATED_SIGNED_WRITE 0x40
#define PROPERTY_EXTENDED_PROPERTIES        0x80

// MARK: Attribute Property Flag, BTstack extension
// value is asked from client
#define PROPERTY_DYNAMIC                    0x100
// 128 bit UUID used
#define PROPERTY_UUID128                    0x200
// Authentication required
#define PROPERTY_AUTHENTICATION_REQUIRED    0x400
// Authorization from user required
#define PROPERTY_AUTHORIZATION_REQUIRED     0x800


typedef struct hal_linked_item{
    struct hal_linked_item *next;
    void *user_data;
}hal_linked_item_t;

typedef struct hal_timer{
    hal_linked_item_t item;
    uint32_t timeout;
    void (*process)(struct hal_timer *ts);
}hal_timer_source_t;

/**@brief BLE status */
typedef enum BLEStatus {
    BLE_STATUS_OK,
    BLE_STATUS_DONE,    // e.g. for service or characteristic discovery done
    BLE_STATUS_CONNECTION_TIMEOUT,
    BLE_STATUS_CONNECTION_ERROR,
    BLE_STATUS_OTHER_ERROR
} BLEStatus_t;


/**@brief BD address */
#define ADDR_LEN 6
typedef uint8_t addr_t[ADDR_LEN];

/**@brief BLE advertising report data. */
typedef struct{
    uint8_t peerAddrType;
    addr_t  peerAddr;
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
	uint8_t dir_addr[ADDR_LEN];
	uint8_t channel_map;
	uint8_t filter_policy;
}advParams_t;

/**@brief Device API */
void hal_btstack_init(void);
void hal_btstack_deInit(void);
void hal_btstack_loop_execute(void);

void     hal_btstack_setTimer(hal_timer_source_t *ts, uint32_t timeout_in_ms);
void     hal_btstack_setTimerHandler(hal_timer_source_t *ts, void (*process)(void *_ts));
void     hal_btstack_addTimer(hal_timer_source_t *timer);
int      hal_btstack_removeTimer(hal_timer_source_t *timer);
uint32_t hal_btstack_getTimeMs(void);

void hal_btstack_debugLogger(uint8_t flag);
void hal_btstack_debugError(uint8_t flag);
void hal_btstack_enablePacketLogger(void);

/**@brief Gap API */
void hal_btstack_getAdvertisementAddr(uint8_t *addr_type, addr_t addr);
void hal_btstack_setRandomAddressMode(uint8_t random_address_type);
void hal_btstack_setRandomAddr(addr_t addr);
void hal_btstack_setPublicBdAddr(addr_t addr);
void hal_btstack_setLocalName(const char *local_name);
void hal_btstack_setAdvParams(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t adv_type, uint8_t dir_addr_type, addr_t dir_addr, uint8_t channel_map, uint8_t filter_policy);
void hal_btstack_setAdvData(uint16_t size, uint8_t *data);

void hal_btstack_startAdvertising(void);
void hal_btstack_stopAdvertising(void);

void hal_btstack_setConnectedCallback(void (*callback)(BLEStatus_t status, uint16_t handle));
void hal_btstack_setDisconnectedCallback(void (*callback)(uint16_t handle));

void hal_btstack_disconnect(uint16_t handle);

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
