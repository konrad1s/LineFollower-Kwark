#include "boot.h"
#include "stm32f722xx.h"

void Boot_JumpToApp(uint32_t address)
{
    __disable_irq();

    /* Disable all interrupts */
    const uint32_t nvicRegsNum = sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]);

    for (unsigned int i = 0U; i < nvicRegsNum; ++i)
    {
        NVIC->ICER[i] = 0xFFFFFFFFU;
        NVIC->ICPR[i] = 0xFFFFFFFFU;
    }

    SysTick->CTRL = 0U;
    SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;

    SCB->SHCSR &= ~(SCB_SHCSR_USGFAULTENA_Msk | SCB_SHCSR_BUSFAULTENA_Msk | SCB_SHCSR_MEMFAULTENA_Msk);

    /* Set the vector table offset register */
    SCB->VTOR = address;

    /* Set the MSP to the value at the reset vector */
    __set_MSP(*(uint32_t *)address);

    /* Jump to the application */
    void (*appEntry)(void) = (void (*)(void))(*(uint32_t *)(address + 4U));

    __DSB();
    __ISB();

    appEntry();

    /* Should never reach here */
    while (1)
    {
    }
}
