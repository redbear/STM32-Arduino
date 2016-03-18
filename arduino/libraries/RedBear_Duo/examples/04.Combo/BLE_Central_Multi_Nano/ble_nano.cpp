
#include "ble_nano.h"
#include "application.h"
//#include "spark_wiring_btstack.h"
//#include "spark_wiring_constants.h"

/******************************************************
 *                 Type Definitions
 ******************************************************/

typedef struct{
	gatt_client_characteristic_t              chars;
	gatt_client_characteristic_descriptor_t   use_descriptor;
	gatt_client_characteristic_descriptor_t   cccd;
}characteristic_t;

typedef struct{

	uint16_t         connect_handle;
	bd_addr_type_t   addr_type;
	bd_addr_t        peer_addr;

	struct{
		gatt_client_service_t service;
		characteristic_t      tx_chars;
		characteristic_t      rx_chars;
	}rbl_service;

	NanoDiscoveryState_t discoveryState;

}nano_t;

/******************************************************
 *               Variable Definitions
 ******************************************************/
static uint8_t service_uuid[16]  = {0x71,0x3d,0x00,0x00,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t chars_tx_uuid[16] = {0x71,0x3d,0x00,0x03,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};
static uint8_t chars_rx_uuid[16] = {0x71,0x3d,0x00,0x02,0x50,0x3e,0x4c,0x75,0xba,0x94,0x31,0x48,0xf1,0x8d,0x94,0x1e};

static nano_t  nano[NANO_NUM];

/******************************************************
 *             Function Definitions
 ******************************************************/
void nano_init(uint8_t num)
{
	if(num >= NANO_NUM)
		return;

	nano[num].connect_handle	= INVALID_CONN_HANDLE;
	nano[num].discoveryState    = NANO_DISCOVERY_IDLE;
}

void nano_setConnectID(uint8_t num, uint16_t handle)
{
	if(num >= NANO_NUM)
		return;

	nano[num].connect_handle = handle;
}

uint16_t nano_getConnectID(uint8_t num)
{
	if(num >= NANO_NUM)
		return INVALID_CONN_HANDLE;

	return nano[num].connect_handle;
}

void nano_setPeerAddr(uint8_t num, bd_addr_t addr, bd_addr_type_t addr_type)
{
	if(num >= NANO_NUM)
		return;

	nano[num].addr_type = addr_type;
	memcpy(nano[num].peer_addr, addr, 0x06);
}

//0 success.
uint8_t nano_connect(uint8_t num)
{
	if(num >= NANO_NUM)
		return 1;

	return ble.connect(nano[num].peer_addr, nano[num].addr_type);
}


uint8_t nano_discoverService(uint8_t num)
{
	if(num >= NANO_NUM)
		return 0xFF;

	nano[num].discoveryState = NANO_DISCOVERY_SERVICES;
	return ble.discoverPrimaryServices(nano[num].connect_handle);
}

void nano_discoveredServiceResult(uint8_t num, gatt_client_service_t *service)
{
	if(num >= NANO_NUM)
		return;

    if(0x00 == memcmp(service->uuid128, service_uuid, 16))
    {
    	nano[num].rbl_service.service = *service;
    }
}

uint8_t nano_discoverCharsOfService(uint8_t num)
{
	if(num >= NANO_NUM)
		return 0xFF;

	nano[num].discoveryState = NANO_DISCOVERY_CHARS_OF_RBL_SERVICE;
	return ble.discoverCharacteristics(nano[num].connect_handle, &nano[num].rbl_service.service);
}

void nano_discoveredCharsResult(uint8_t num, gatt_client_characteristic_t *chars)
{
	if(num >= NANO_NUM)
		return;

    if(0x00 == memcmp(chars->uuid128, chars_tx_uuid, 16))
    {
    	Serial.println("Discovered characteristic txxxx");
    	nano[num].rbl_service.tx_chars.chars = *chars;
    }
    else if(0x00 == memcmp(chars->uuid128, chars_rx_uuid, 16))
    {

    	Serial.println("Discovered characteristic rxxxx");
    	nano[num].rbl_service.rx_chars.chars = *chars;
    }
}

uint8_t nano_discoverDescriptor(uint8_t num)
{
	if(num >= NANO_NUM)
		return 0xFF;


	nano[num].discoveryState = NANO_DISCOVERY_DESCRIPTOR_OF_RX_CHARS;
	return ble.discoverCharacteristicDescriptors(nano[num].connect_handle, &nano[num].rbl_service.rx_chars.chars);

	return 0;
}

void nano_discoverDescriptorResult(uint8_t num, gatt_client_characteristic_descriptor_t *descriptor)
{
	if(num >= NANO_NUM)
		return;

	if(descriptor->uuid16 == 0x2901)
	{
		nano[num].rbl_service.rx_chars.use_descriptor = *descriptor;
	}
	else if(descriptor->uuid16 == 0x2902)
	{
		nano[num].rbl_service.rx_chars.cccd = *descriptor;
	}
}

uint8_t nano_startNotify(uint8_t num)
{
	if(num >= NANO_NUM)
		return 0xFF;

	return ble.writeClientCharsConfigDescritpor(nano[num].connect_handle, &nano[num].rbl_service.rx_chars.chars, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION);
}

uint8_t nano_stopNotify(uint8_t num)
{
	if(num >= NANO_NUM)
		return 0xFF;

	return ble.writeClientCharsConfigDescritpor(nano[num].connect_handle, &nano[num].rbl_service.rx_chars.chars, GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NONE);
}

uint8_t nano_write(uint8_t num, uint8_t *buf, uint8_t len)
{ 
    if(num >= NANO_NUM)
      return 0xFF;
      
    return ble.writeValueWithoutResponse(nano[num].connect_handle, nano[num].rbl_service.tx_chars.chars.value_handle, len, buf);
}

uint8_t nano_read(uint8_t num)
{ 
    if(num >= NANO_NUM)
      return 0xFF;
      
    return ble.readValue(nano[num].connect_handle, &nano[num].rbl_service.rx_chars.chars);
}

