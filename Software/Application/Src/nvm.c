#include "nvm.h"
#include "stm32f7xx_hal.h"

#define FLASH_SECTOR_INVALID (0xFFFFFFFFU)

static uint32_t GetSectorBaseAddress(uint32_t sector)
{
    switch (sector)
    {
    case FLASH_SECTOR_0:
        return 0x08000000U;
    case FLASH_SECTOR_1:
        return 0x08004000U;
    case FLASH_SECTOR_2:
        return 0x08008000U;
    case FLASH_SECTOR_3:
        return 0x0800C000U;
    case FLASH_SECTOR_4:
        return 0x08010000U;
    case FLASH_SECTOR_5:
        return 0x08020000U;
    case FLASH_SECTOR_6:
        return 0x08040000U;
    case FLASH_SECTOR_7:
        return 0x08060000U;
    default:
        return FLASH_SECTOR_INVALID;
    }
}

void NVM_Init(NVM_T *const nvm, uint8_t *data, uint32_t size, uint32_t sector)
{
    if (nvm != NULL && data != NULL && sector < FLASH_SECTOR_TOTAL)
    {
        nvm->data = data;
        nvm->size = size;
        nvm->sector = sector;
    }
}

bool NVM_Read(NVM_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL)
    {
        return false;
    }

    uint32_t baseAddress = GetSectorBaseAddress(nvm->sector);
    if (baseAddress == FLASH_SECTOR_INVALID)
    {
        return false;
    }

    /* Read data from the flash memory into the buffer */
    for (uint32_t i = 0; i < nvm->size; i++)
    {
        nvm->data[i] = *(__IO uint8_t *)(baseAddress + i);
    }

    /* TODO: Implement CRC check here */

    return true;
}

bool NVM_Write(NVM_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL)
    {
        return false;
    }

    if (NVM_Erase(nvm) == false)
    {
        return false;
    }

    uint32_t size = nvm->size;
    uint8_t *pData = nvm->data;
    uint32_t currentAddress = GetSectorBaseAddress(nvm->sector);

    if (currentAddress == FLASH_SECTOR_INVALID)
    {
        return false;
    }

    HAL_FLASH_Unlock();

    while (size > 0)
    {
        uint64_t programData = 0;
        uint32_t increment = 0;
        uint32_t programType = 0;

        if ((size >= 4) && (currentAddress % 4 == 0))
        {
            programType = FLASH_TYPEPROGRAM_WORD;
            programData = *(uint32_t *)pData;
            increment = 4;
        }
        else if ((size >= 2) && (currentAddress % 2 == 0))
        {
            programType = FLASH_TYPEPROGRAM_HALFWORD;
            programData = *(uint16_t *)pData;
            increment = 2;
        }
        else
        {
            programType = FLASH_TYPEPROGRAM_BYTE;
            programData = *pData;
            increment = 1;
        }

        if (HAL_FLASH_Program(programType, currentAddress, programData) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }

        currentAddress += increment;
        pData += increment;
        size -= increment;
    }

    HAL_FLASH_Lock();

    return true;
}

bool NVM_Erase(NVM_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL)
    {
        return false;
    }

    FLASH_EraseInitTypeDef eraseInitStruct;
    uint32_t sectorError = 0U;

    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInitStruct.Sector = nvm->sector;
    eraseInitStruct.NbSectors = 1U;

    HAL_FLASH_Unlock();

    if (HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK)
    {
        HAL_FLASH_Lock();
        return false;
    }

    HAL_FLASH_Lock();

    return true;
}
