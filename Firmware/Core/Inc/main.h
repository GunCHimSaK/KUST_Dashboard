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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void app_lvgl_assert(void);   /* LVGL assert 발생 시 LED 패턴 표시 */
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define BT_0_Pin GPIO_PIN_0
#define BT_0_GPIO_Port GPIOA
#define BT_0_EXTI_IRQn EXTI0_IRQn
#define BT_1_Pin GPIO_PIN_1
#define BT_1_GPIO_Port GPIOA
#define BT_1_EXTI_IRQn EXTI1_IRQn
#define BT_2_Pin GPIO_PIN_2
#define BT_2_GPIO_Port GPIOA
#define BT_2_EXTI_IRQn EXTI2_IRQn
#define BT_3_Pin GPIO_PIN_3
#define BT_3_GPIO_Port GPIOA
#define BT_3_EXTI_IRQn EXTI3_IRQn
#define BT_4_Pin GPIO_PIN_4
#define BT_4_GPIO_Port GPIOA
#define BT_4_EXTI_IRQn EXTI4_IRQn
#define SPI1_SCK_Pin GPIO_PIN_5
#define SPI1_SCK_GPIO_Port GPIOA
#define SPI1_SCS_Pin GPIO_PIN_6
#define SPI1_SCS_GPIO_Port GPIOA
#define SPI1_MOSI_Pin GPIO_PIN_7
#define SPI1_MOSI_GPIO_Port GPIOA
#define EXTCOMIN_Pin GPIO_PIN_0
#define EXTCOMIN_GPIO_Port GPIOB
#define IMON_OUT_Pin GPIO_PIN_1
#define IMON_OUT_GPIO_Port GPIOB
#define DIN_3_Pin GPIO_PIN_13
#define DIN_3_GPIO_Port GPIOB
#define DIN_2_Pin GPIO_PIN_14
#define DIN_2_GPIO_Port GPIOB
#define DIN_1_Pin GPIO_PIN_15
#define DIN_1_GPIO_Port GPIOB
#define DIN_0_Pin GPIO_PIN_8
#define DIN_0_GPIO_Port GPIOA

/* USER CODE BEGIN Private defines */
/* DIN 입력 극성 : 1 = High일 때 '켜짐'(Active-HIGH), 0 = Low일 때 '켜짐'
 *
 * 이 보드는 0(Active-LOW)이 맞습니다. 회로도 U9(TLP291-4GB) 기준:
 *   DIN_x -- 2.2K -- 포토커플러 LED -- GND_EXT      (입력측, 1N4148 환류다이오드)
 *   포토트랜지스터 컬렉터 -- STM_DIN_x -- 10K 풀업 -- +3V3   (출력측, 이미터=GND)
 *
 *   신호 없음 -> LED off -> 트랜지스터 off -> 10K 풀업이 HIGH로  => '꺼짐'
 *   신호 있음 -> LED on  -> 트랜지스터 on  -> GND로 당김 = LOW   => '켜짐'
 *
 * 외부 10K 풀업(R30/R31/R35/R36)이 있으므로 MCU 내부 풀업은 불필요합니다.
 * GPIO는 NOPULL 그대로 두세요. */
#define DIN_ACTIVE_HIGH  0

#if DIN_ACTIVE_HIGH
#define DIN_IS_ON(port, pin)  (HAL_GPIO_ReadPin((port), (pin)) == GPIO_PIN_SET)
#else
#define DIN_IS_ON(port, pin)  (HAL_GPIO_ReadPin((port), (pin)) == GPIO_PIN_RESET)
#endif

/* LED 극성 : 1 = High일 때 점등(Active-HIGH), 0 = Low일 때 점등(Active-LOW)
 * 하트비트가 반대로 보이면 이 값을 0으로 바꾸세요. */
#define LED_ACTIVE_HIGH  1

#if LED_ACTIVE_HIGH
#define LED_ON()   (LED_GPIO_Port->BSRR = LED_Pin)
#define LED_OFF()  (LED_GPIO_Port->BRR  = LED_Pin)
#else
#define LED_ON()   (LED_GPIO_Port->BRR  = LED_Pin)
#define LED_OFF()  (LED_GPIO_Port->BSRR = LED_Pin)
#endif
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
