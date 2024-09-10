/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED12_Pin GPIO_PIN_13
#define LED12_GPIO_Port GPIOC
#define LED11_Pin GPIO_PIN_14
#define LED11_GPIO_Port GPIOC
#define LED10_Pin GPIO_PIN_15
#define LED10_GPIO_Port GPIOC
#define LED9_Pin GPIO_PIN_0
#define LED9_GPIO_Port GPIOC
#define LED8_Pin GPIO_PIN_1
#define LED8_GPIO_Port GPIOC
#define LED7_Pin GPIO_PIN_2
#define LED7_GPIO_Port GPIOC
#define SENSOR12_Pin GPIO_PIN_3
#define SENSOR12_GPIO_Port GPIOC
#define SENSOR11_Pin GPIO_PIN_0
#define SENSOR11_GPIO_Port GPIOA
#define SENSOR10_Pin GPIO_PIN_1
#define SENSOR10_GPIO_Port GPIOA
#define SENSOR9_Pin GPIO_PIN_2
#define SENSOR9_GPIO_Port GPIOA
#define SENSOR8_Pin GPIO_PIN_3
#define SENSOR8_GPIO_Port GPIOA
#define SENSOR7_Pin GPIO_PIN_4
#define SENSOR7_GPIO_Port GPIOA
#define SENSOR6_Pin GPIO_PIN_5
#define SENSOR6_GPIO_Port GPIOA
#define SENSOR5_Pin GPIO_PIN_6
#define SENSOR5_GPIO_Port GPIOA
#define SENSOR4_Pin GPIO_PIN_7
#define SENSOR4_GPIO_Port GPIOA
#define SENSOR3_Pin GPIO_PIN_4
#define SENSOR3_GPIO_Port GPIOC
#define SENSOR2_Pin GPIO_PIN_0
#define SENSOR2_GPIO_Port GPIOB
#define SENSOR1_Pin GPIO_PIN_1
#define SENSOR1_GPIO_Port GPIOB
#define LED6_Pin GPIO_PIN_2
#define LED6_GPIO_Port GPIOB
#define LED5_Pin GPIO_PIN_10
#define LED5_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_11
#define LED4_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_12
#define LED3_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_15
#define LED1_GPIO_Port GPIOB
#define ENCODER_CH2_Pin GPIO_PIN_6
#define ENCODER_CH2_GPIO_Port GPIOC
#define ENCODER_CH1_Pin GPIO_PIN_7
#define ENCODER_CH1_GPIO_Port GPIOC
#define MOTOR1_BIN_Pin GPIO_PIN_9
#define MOTOR1_BIN_GPIO_Port GPIOA
#define MOTOR1_AIN_Pin GPIO_PIN_10
#define MOTOR1_AIN_GPIO_Port GPIOA
#define MOTOR1_PWM_Pin GPIO_PIN_11
#define MOTOR1_PWM_GPIO_Port GPIOA
#define MOTOR2_BIN_Pin GPIO_PIN_2
#define MOTOR2_BIN_GPIO_Port GPIOD
#define MOTOR2_AIN_Pin GPIO_PIN_3
#define MOTOR2_AIN_GPIO_Port GPIOB
#define MOTOR2_PWM_Pin GPIO_PIN_4
#define MOTOR2_PWM_GPIO_Port GPIOB
#define ENCODER_CH1B6_Pin GPIO_PIN_6
#define ENCODER_CH1B6_GPIO_Port GPIOB
#define ENCODER_CH2B7_Pin GPIO_PIN_7
#define ENCODER_CH2B7_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
