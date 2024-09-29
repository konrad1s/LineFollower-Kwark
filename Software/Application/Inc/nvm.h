#ifndef __NVM__H__
#define __NVM__H__

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint8_t *data;              /* Pointer to the RAM buffer */
    const uint8_t *defaultData; /* Pointer to the default data */
    uint32_t size;              /* Size of the EEPROM data */
    uint32_t crc;               /* CRC value of the EEPROM data */
    uint32_t sector;            /* Sector number of the Flash memory, where NVM data is stored */
    uint32_t lastCrc;           /* Last CRC value of the EEPROM data */
} Nvm_Instance_T;

int NVM_Init(Nvm_Instance_T *const nvm);
int NVM_Read(Nvm_Instance_T *const nvm);
int NVM_Write(Nvm_Instance_T *const nvm);
int NVM_Erase(Nvm_Instance_T *const nvm);

#endif /* __NVM__H__ */
