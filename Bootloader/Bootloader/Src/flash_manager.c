/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "flash_manager.h"
#include "stm32f7xx_hal.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/
static int FlashManager_ToggleFlashLock(bool lock);

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
/**
 * @brief Toggles the flash lock state.
 *
 * @param lock True to lock the flash; false to unlock.
 * @return 0 on success; -1 on failure.
 */
static int FlashManager_ToggleFlashLock(bool lock)
{
    HAL_StatusTypeDef status;

    if (lock)
    {
        status = HAL_FLASH_Lock();
    }
    else
    {
        status = HAL_FLASH_Unlock();
    }

    return (status == HAL_OK) ? 0 : -1;
}

/**
 * @brief Erases the application sectors in flash memory.
 *
 * @param manager Pointer to the Flash_Manager_T instance.
 * @return 0 on success; -1 on failure.
 */
int FlashManager_Erase(Flash_Manager_T *const manager)
{
    FLASH_EraseInitTypeDef eraseData = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Sector = manager->appSectorRange.startSector,
        .NbSectors = manager->appSectorRange.sectorCount,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };

    if (FlashManager_ToggleFlashLock(false) != 0)
    {
        return -1;
    }

    uint32_t sectorError = 0;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&eraseData, &sectorError);
    FlashManager_ToggleFlashLock(true);

    return (status == HAL_OK) ? 0 : -1;
}

/**
 * @brief Writes data to flash memory.
 *
 * @param manager Pointer to the Flash_Manager_T instance.
 * @param startAddress The start address to write to in flash memory.
 * @param data Pointer to the data to be written.
 * @param size The size of the data to write in bytes.
 * @return 0 on success; -1 on failure.
 */
int FlashManager_Write(Flash_Manager_T *const manager, uint32_t startAddress, const void *data, size_t size)
{
    const uint8_t *pData = (const uint8_t *)data;
    uint32_t currentAddress = startAddress;

    if (FlashManager_ToggleFlashLock(false) != 0)
    {
        return -1;
    }

    uint32_t programType;
    uint64_t programData;
    size_t increment;

    while (size > 0)
    {
        if (size >= 4 && currentAddress % 4 == 0)
        {
            programType = FLASH_TYPEPROGRAM_WORD;
            programData = *(uint32_t *)pData;
            increment = 4;
        }
        else if (size >= 2 && currentAddress % 2 == 0)
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
            FlashManager_ToggleFlashLock(true);
            return -1;
        }

        currentAddress += increment;
        pData += increment;
        size -= increment;
    }

    FlashManager_ToggleFlashLock(true);

    return 0;
}

/**
 * @brief Reads data from flash memory.
 *
 * @param manager Pointer to the Flash_Manager_T instance.
 * @param startAddress The start address to read from.
 * @param buffer Pointer to the buffer to store the read data.
 * @param size The size of the data to read.
 * @return 0 on success; -1 on failure.
 */
int FlashManager_Read(Flash_Manager_T *const manager, uint32_t startAddress, void *buffer, size_t size)
{
    memcpy(buffer, (const void *)startAddress, size);

    return 0;
}
