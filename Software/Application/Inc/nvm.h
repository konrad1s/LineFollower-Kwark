#ifndef __NVM__H__
#define __NVM__H__

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t *data;    /* Pointer to the RAM buffer */
    uint32_t size;    /* Size of the EEPROM data */
    uint32_t crc;     /* CRC value of the EEPROM data */
    uint32_t sector;  /* Sector number of the Flash memory, where NVM data is stored */
} NVM_T;

void NVM_Init(NVM_T *const nvm, uint8_t *data, uint32_t size, uint32_t sector);
bool NVM_Read(NVM_T *const nvm);
bool NVM_Write(NVM_T *const nvm);
bool NVM_Erase(NVM_T *const nvm);

#endif /* __NVM__H__ */
