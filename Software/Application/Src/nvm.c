#include "nvm.h"
#include "stm32f7xx_hal.h"
#include "crc.h"
#include <string.h>

#define NVM_CRC_INIT_VALUE   (0xFFFFFFFFU)
#define FLASH_SECTOR_INVALID (0xFFFFFFFFU)

/**
 * @brief Retrieves the base address of the specified flash sector.
 *
 * @param[in] sector Flash sector number.
 * 
 * @return The base address of the flash sector, or `FLASH_SECTOR_INVALID` if invalid.
 */
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

/**
 * @brief Writes data to the specified flash memory address.
 *
 * @param[in] baseAddress Starting address in flash memory for the write operation.
 * @param[in] pData Pointer to the data to be written.
 * @param[in] size Number of bytes to write.
 *
 * @return
 * - true if the write operation is successful.
 * - false if the write operation fails.
 */
static bool NVM_FlashWrite(uint32_t baseAddress, uint8_t *pData, uint32_t size)
{
    HAL_FLASH_Unlock();

    while (size > 0)
    {
        uint64_t programData;
        uint32_t increment, programType;

        if (size >= 4U)
        {
            programType = FLASH_TYPEPROGRAM_WORD;
            programData = *(uint32_t *)pData;
            increment = 4U;
        }
        else if (size >= 2U)
        {
            programType = FLASH_TYPEPROGRAM_HALFWORD;
            programData = *(uint16_t *)pData;
            increment = 2U;
        }
        else
        {
            programType = FLASH_TYPEPROGRAM_BYTE;
            programData = *pData;
            increment = 1U;
        }

        if (HAL_FLASH_Program(programType, baseAddress, programData) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return false;
        }

        baseAddress += increment;
        pData += increment;
        size -= increment;
    }

    HAL_FLASH_Lock();

    return true;
}

/**
 * @brief Initializes the NVM instance.
 *
 * @param[in,out] nvm Pointer to the NVM instance.
 *
 * @return
 * - 0 on success.
 * - -1 if the initialization fails.
 */
int NVM_Init(Nvm_Instance_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL || nvm->sector >= FLASH_SECTOR_TOTAL || nvm->size == 0U)
    {
        return -1;
    }

    nvm->lastCrc = NVM_CRC_INIT_VALUE;

    return 0;
}

/**
 * @brief Reads data from flash memory into the NVM buffer.
 *
 * @param[in,out] nvm Pointer to the NVM instance.
 *
 * @return
 * - 0 if the data is read and validated successfully.
 * - -1 if the read operation fails.
 */
int NVM_Read(Nvm_Instance_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL)
    {
        return -1;
    }

    uint32_t baseAddress = GetSectorBaseAddress(nvm->sector);
    if (baseAddress == FLASH_SECTOR_INVALID)
    {
        return -1;
    }

    /* Read data from the flash memory into the buffer */
    for (uint32_t i = 0U; i < nvm->size; i++)
    {
        nvm->data[i] = *(__IO uint8_t *)(baseAddress + i);
    }

    /* Read the CRC value from the flash memory */
    uint32_t crcAddress = baseAddress + nvm->size;
    uint32_t storedCrc = *(__IO uint32_t *)crcAddress;
    /* Calculate the CRC value of the data */
    uint32_t calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t *)nvm->data, nvm->size);

    if (calculatedCrc != storedCrc)
    {
        if (nvm->defaultData != NULL)
        {
            /* If the calculated CRC value is different, load the default data */
            memcpy(nvm->data, nvm->defaultData, nvm->size);
            calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t *)nvm->data, nvm->size);
        }
        else
        {
            return -1;
        }
    }

    nvm->lastCrc = calculatedCrc;

    return 0;
}

/**
 * @brief Writes data from the NVM buffer to flash memory.
 *
 * @param[in] nvm Pointer to the NVM instance.
 *
 * @return
 * - 0 if the data is written successfully.
 * - -1 if the write operation fails.
 */
int NVM_Write(Nvm_Instance_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL)
    {
        return -1;
    }

    uint32_t size = nvm->size;
    uint8_t *pData = nvm->data;
    uint32_t currentAddress = GetSectorBaseAddress(nvm->sector);

    if (currentAddress == FLASH_SECTOR_INVALID)
    {
        return -1;
    }

    /* Calculate the CRC value of the data */
    uint32_t calculatedCrc = HAL_CRC_Calculate(&hcrc, (uint32_t *)nvm->data, nvm->size);

    /* If the calculated CRC value is different from the last CRC value, write the data to the flash memory */
    if (calculatedCrc != nvm->lastCrc)
    {
        if (NVM_Erase(nvm) == -1)
        {
            return -1;
        }
        if (NVM_FlashWrite(currentAddress, pData, size) == -1)
        {
            return -1;
        }

        /* Write the calculated CRC value after the data */
        uint32_t crcAddress = currentAddress + size;
        if (NVM_FlashWrite(crcAddress, (uint8_t *)&calculatedCrc, sizeof(calculatedCrc)) == -1)
        {
            return -1;
        }

        /* Update the last CRC value */
        nvm->lastCrc = calculatedCrc;
    }

    return 0;
}

/**
 * @brief Erases the specified flash sector.
 *
 * @param[in] nvm Pointer to the NVM instance.
 *
 * @return
 * - 0 if the sector is erased successfully.
 * - -1 if the erase operation fails.
 */
int NVM_Erase(Nvm_Instance_T *const nvm)
{
    if (nvm == NULL || nvm->data == NULL)
    {
        return -1;
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
        return -1;
    }

    HAL_FLASH_Lock();

    nvm->lastCrc = NVM_CRC_INIT_VALUE;

    return 0;
}
