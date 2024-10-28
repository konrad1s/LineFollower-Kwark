/******************************************************************************************
 *                                        INCLUDES                                        *
 ******************************************************************************************/
#include "tb6612_motor.h"

/******************************************************************************************
 *                                         DEFINES                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                        TYPEDEFS                                        *
 ******************************************************************************************/

/******************************************************************************************
 *                                   FUNCTIONS PROTOTYPES                                 *
 ******************************************************************************************/

/******************************************************************************************
 *                                        VARIABLES                                       *
 ******************************************************************************************/

/******************************************************************************************
 *                                        FUNCTIONS                                       *
 ******************************************************************************************/
/**
 * @brief Initializes driver by setting it to stop mode.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 */
int TB6612Motor_Init(const TB6612MotorDriver_T *const driver)
{
    if ((driver == NULL) || (driver->pwmTimer == NULL) || (driver->in1.port == NULL) || (driver->in2.port == NULL))
    {
        return -1;
    }

    HAL_TIM_PWM_Start(driver->pwmTimer, driver->pwmChannel);
    TB6612Motor_Stop(driver);

    return 0;
}

/**
 * @brief Changes the motor direction.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 * @param direction Motor direction.
 */
void TB6612Motor_ChangeDirection(const TB6612MotorDriver_T *const driver, TB6612MotorDirection_T direction)
{
    switch (direction)
    {
    case MOTOR_FORWARD:
        HAL_GPIO_WritePin(driver->in1.port, driver->in1.pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(driver->in2.port, driver->in2.pin, GPIO_PIN_SET);
        break;
    case MOTOR_BACKWARD:
        HAL_GPIO_WritePin(driver->in1.port, driver->in1.pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(driver->in2.port, driver->in2.pin, GPIO_PIN_RESET);
        break;
    case MOTOR_STOP:
        HAL_GPIO_WritePin(driver->in1.port, driver->in1.pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(driver->in2.port, driver->in2.pin, GPIO_PIN_RESET);
        break;
    case MOTOR_BRAKE:
        HAL_GPIO_WritePin(driver->in1.port, driver->in1.pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(driver->in2.port, driver->in2.pin, GPIO_PIN_SET);
        break;
    }
}

/**
 * @brief Sets the PWM speed for the motor.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 * @param speed The speed value (0-999 for PWM).
 */
void TB6612Motor_SetSpeed(const TB6612MotorDriver_T *const driver, uint16_t speed)
{
    __HAL_TIM_SET_COMPARE(driver->pwmTimer, driver->pwmChannel, speed);
}

/**
 * @brief Stops the motor by.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 */
void TB6612Motor_Stop(const TB6612MotorDriver_T *const driver)
{
    TB6612Motor_ChangeDirection(driver, MOTOR_STOP);
    TB6612Motor_SetSpeed(driver, 0);
}

/**
 * @brief Brakes the motor.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 */
void TB6612Motor_Brake(const TB6612MotorDriver_T *const driver)
{
    TB6612Motor_ChangeDirection(driver, MOTOR_BRAKE);
    TB6612Motor_SetSpeed(driver, 0);
}
