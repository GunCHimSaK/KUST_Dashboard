/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32u5xx_hal.h"

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

void HAL_LPTIM_MspPostInit(LPTIM_HandleTypeDef *hlptim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define User_Button_Pin GPIO_PIN_13
#define User_Button_GPIO_Port GPIOC
#define STM_DIN2_Pin GPIO_PIN_14
#define STM_DIN2_GPIO_Port GPIOC
#define STM_DIN3_Pin GPIO_PIN_15
#define STM_DIN3_GPIO_Port GPIOC
#define STM_DIN0_Pin GPIO_PIN_0
#define STM_DIN0_GPIO_Port GPIOA
#define STM_DIN1_Pin GPIO_PIN_2
#define STM_DIN1_GPIO_Port GPIOA
#define STM_AIN0_Pin GPIO_PIN_3
#define STM_AIN0_GPIO_Port GPIOA
#define EXTCOMIN_Pin GPIO_PIN_4
#define EXTCOMIN_GPIO_Port GPIOA
#define SPI1_SCS_Pin GPIO_PIN_6
#define SPI1_SCS_GPIO_Port GPIOA
#define LED_ERR1_Pin GPIO_PIN_0
#define LED_ERR1_GPIO_Port GPIOB
#define LED_ERR0_Pin GPIO_PIN_1
#define LED_ERR0_GPIO_Port GPIOB
#define STM_Button_1_Pin GPIO_PIN_13
#define STM_Button_1_GPIO_Port GPIOB
#define STM_Button_1_EXTI_IRQn EXTI13_IRQn
#define STM_Button_0_Pin GPIO_PIN_14
#define STM_Button_0_GPIO_Port GPIOB
#define STM_Button_0_EXTI_IRQn EXTI14_IRQn
#define STM_Button_7_Pin GPIO_PIN_15
#define STM_Button_7_GPIO_Port GPIOB
#define STM_Button_6_Pin GPIO_PIN_8
#define STM_Button_6_GPIO_Port GPIOA
#define STM_Button_5_Pin GPIO_PIN_9
#define STM_Button_5_GPIO_Port GPIOA
#define STM_Button_4_Pin GPIO_PIN_10
#define STM_Button_4_GPIO_Port GPIOA
#define STM_Button_2_Pin GPIO_PIN_15
#define STM_Button_2_GPIO_Port GPIOA
#define STM_Button_2_EXTI_IRQn EXTI15_IRQn
#define STM_Button_3_Pin GPIO_PIN_4
#define STM_Button_3_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
