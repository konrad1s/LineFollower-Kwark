#include "tb6612_motor.h"

/**
 * @brief Initializes driver by setting it to stop mode.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 */
void TB6612Motor_Init(TB6612MotorDriver_T *const driver)
{
    if ((driver == NULL) || (driver->pwmTimer == NULL) || (driver->in1.port == NULL) || (driver->in2.port == NULL))
    {
        /* TODO: Handle error*/
        return;
    }
    TB6612Motor_Stop(driver);
}

/**
 * @brief Changes the motor direction.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 * @param direction Motor direction.
 */
void TB6612Motor_ChangeDirection(TB6612MotorDriver_T *const driver, TB6612MotorDirection_T direction)
{
    switch (direction)
    {
    case MOTOR_FORWARD:
        HAL_GPIO_WritePin(driver->in1.port, driver->in1.pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(driver->in2.port, driver->in2.pin, GPIO_PIN_RESET);
        break;
    case MOTOR_BACKWARD:
        HAL_GPIO_WritePin(driver->in1.port, driver->in1.pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(driver->in2.port, driver->in2.pin, GPIO_PIN_SET);
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
void TB6612Motor_SetSpeed(TB6612MotorDriver_T *const driver, uint16_t speed)
{
    __HAL_TIM_SET_COMPARE(driver->pwmTimer, driver->pwmChannel, speed);
}

/**
 * @brief Stops the motor by.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 */
void TB6612Motor_Stop(TB6612MotorDriver_T *const driver)
{
    TB6612Motor_ChangeDirection(driver, MOTOR_STOP);
    TB6612Motor_SetSpeed(driver, 0);
}

/**
 * @brief Brakes the motor.
 * 
 * @param driver Pointer to the TB6612MotorDriver_T instance.
 */
void TB6612Motor_Brake(TB6612MotorDriver_T *const driver)
{
    TB6612Motor_ChangeDirection(driver, MOTOR_BRAKE);
    TB6612Motor_SetSpeed(driver, 0);
}