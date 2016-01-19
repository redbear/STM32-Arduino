#ifndef DEVICE_NAME_H
#define	DEVICE_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DEVICE_NAME_SIZE 20

typedef struct
{
    uint8_t length;
    uint8_t value[ MAX_DEVICE_NAME_SIZE ];
} device_name_t;

void HAL_Device_Name(device_name_t *dev_name);

#ifdef __cplusplus
}
#endif

#endif
