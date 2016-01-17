
#ifndef SYSTEM_WIRELESS_UPDATE_H_
#define SYSTEM_WIRELESS_UPDATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (PLATFORM_ID==88)

void Wireless_Update_Begin(uint32_t file_length, uint16_t chunk_size, uint32_t chunk_address);
void Wireless_Update_Save_Chunk(uint8_t *data, uint16_t length);

#endif

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_WIRELESS_UPDATE_H_ */

