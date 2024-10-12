#ifndef __FLASH_MANAGER_H__
#define __FLASH_MANAGER_H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct
{
    uint32_t startSector;
    uint32_t sectorCount;
} Flash_SectorRange_T;

typedef struct
{
    Flash_SectorRange_T appSectorRange;
    uint32_t appStartAddress;
    uint32_t appEndAddress;
} Flash_Manager_T;

int FlashManager_Erase(Flash_Manager_T *const manager);
int FlashManager_Write(Flash_Manager_T *const manager, uint32_t startAddress, const void *data, size_t size);
int FlashManager_Read(Flash_Manager_T *const manager, uint32_t startAddress, void *buffer, size_t size);

#endif /* __FLASH_MANAGER_H__ */
