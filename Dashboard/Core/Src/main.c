/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "lvgl.h"
#include "SHARP_MIP.h"
#include "ui.h"
#include "vehicle_data.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

FDCAN_HandleTypeDef hfdcan1;

LPTIM_HandleTypeDef hlptim2;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef handle_GPDMA1_Channel12;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_GPDMA1_Init(void);
static void MX_SPI1_Init(void);
static void MX_ADC1_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_LPTIM2_Init(void);
static void MX_TIM2_Init(void);
static void MX_ICACHE_Init(void);
/* USER CODE BEGIN PFP */
void Debug_LED_ShowError(uint8_t errorState);
void Light_Process(void);
void FDCAN_Send_ControlData(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void Light_Process(void)
{
	LightData.Left_Signal = (HAL_GPIO_ReadPin(STM_DIN0_GPIO_Port, STM_DIN0_Pin)
			== GPIO_PIN_SET);
	LightData.Right_Signal = (HAL_GPIO_ReadPin(STM_DIN1_GPIO_Port, STM_DIN1_Pin)
			== GPIO_PIN_SET);
	LightData.Hazard_Signal = (HAL_GPIO_ReadPin(STM_DIN2_GPIO_Port,
	STM_DIN2_Pin) == GPIO_PIN_SET);
}

void Dashboard_Update(void)
{
	lv_label_set_text(ui_FaultcodeLabel, "00");
	lv_label_set_text(ui_FaultcodecountLabel, "+0");

	/* 1. 속도 및 권장 속도 매핑 */
	lv_label_set_text_fmt(ui_SpeedLabel, "%d", DrivingData.Speed);
	lv_label_set_text_fmt(ui_RecommendLabel, "%d", DrivingData.Recommend_Speed);

	/* 2. 전력 시스템 및 배터리 정보 */
	lv_label_set_text_fmt(ui_MpptLabel, "%d W", DrivingData.MPPT_Power);
	lv_label_set_text_fmt(ui_MotorLabel, "%d W", DrivingData.Motor_Power);
	lv_label_set_text_fmt(ui_BattaryLabel, "%d%%", DrivingData.Battery_SOC);

	lv_bar_set_value(ui_Bar1, InfoData.Info_4, LV_ANIM_OFF);

	if (DrivingData.Signal_State == true)
	{
		lv_obj_clear_flag(ui_TeleImage, LV_OBJ_FLAG_HIDDEN); // 통신 연결 시 표시
	}
	else
	{
		lv_obj_add_flag(ui_TeleImage, LV_OBJ_FLAG_HIDDEN);   // 통신 끊김 시 숨김
	}

	// 배터리 상태 이상 시 경고 아이콘 노출 제어
	if (DrivingData.Battery_State == false)
	{
		lv_obj_clear_flag(ui_BattarywarnImage, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
		lv_obj_add_flag(ui_BattarywarnImage, LV_OBJ_FLAG_HIDDEN);
	}

	/* 3. 시스템 상세 정보 (라벨 위젯 연동) */
	lv_label_set_text_fmt(ui_info1unit, "%d %%", InfoData.Info_1);
	lv_label_set_text_fmt(ui_info2unit, "%d W", InfoData.Info_2);
	lv_label_set_text_fmt(ui_info3unit, "%d km", InfoData.Info_3);

	/* 4. 시간 표시 (HH:MM 포맷) */
	static char time_buf[16];
	snprintf(time_buf, sizeof(time_buf), "%02d:%02d", InfoData.Time_Hour,
			InfoData.Time_Min);
	lv_label_set_text(ui_TimeLabel, time_buf);

	/* 5. ACC UI 제어 */
	if (AccData.Acc_state)
	{
		lv_obj_clear_flag(ui_AccImage, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui_AccUnit, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(ui_AccLabel, LV_OBJ_FLAG_HIDDEN);
		lv_label_set_text_fmt(ui_AccLabel, "%d", AccData.Acc_speed);
	}
	else
	{
		lv_obj_add_flag(ui_AccImage, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui_AccUnit, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(ui_AccLabel, LV_OBJ_FLAG_HIDDEN);
	}

	static uint32_t lastBlinkTick = 0;
	static bool isVisible = false;

	// 일단 무조건 아이콘을 모두 숨김 상태로 시작
	lv_obj_add_flag(ui_Left1Image, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_Right1Image, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(ui_HazzardImage, LV_OBJ_FLAG_HIDDEN);

	// 셋 중 하나라도 신호가 들어와 있을 때만 연산 시작!
	if (LightData.Left_Signal || LightData.Right_Signal
			|| LightData.Hazard_Signal)
	{
		// 0.5초가 지났는지 확인
		if (HAL_GetTick() - lastBlinkTick >= 500)
		{
			isVisible = !isVisible;
			lastBlinkTick = HAL_GetTick();
		}

		// 보여줄 타이밍일 때만 아이콘 표시
		if (isVisible)
		{
			if (LightData.Hazard_Signal)
			{
				lv_obj_clear_flag(ui_HazzardImage, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(ui_Left1Image, LV_OBJ_FLAG_HIDDEN);
				lv_obj_clear_flag(ui_Right1Image, LV_OBJ_FLAG_HIDDEN);
			}
			else
			{
				if (LightData.Left_Signal)
				{
					lv_obj_clear_flag(ui_Left1Image, LV_OBJ_FLAG_HIDDEN);
				}
				if (LightData.Right_Signal)
				{
					lv_obj_clear_flag(ui_Right1Image, LV_OBJ_FLAG_HIDDEN);
				}
			}
		}
	}
	else
	{
		// 버튼을 모두 떼었을 때: 다음에 버튼을 누르면 즉시 불이 켜지도록 초기화
		isVisible = true;
		lastBlinkTick = HAL_GetTick();
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the System Power */
  SystemPower_Config();

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_GPDMA1_Init();
  MX_SPI1_Init();
  MX_ADC1_Init();
  MX_FDCAN1_Init();
  MX_LPTIM2_Init();
  MX_TIM2_Init();
  MX_ICACHE_Init();
  /* USER CODE BEGIN 2 */
	HAL_FDCAN_ConfigGlobalFilter(&hfdcan1, FDCAN_REJECT, FDCAN_REJECT,
	FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

	FDCAN_FilterTypeDef sFilterConfig;

	sFilterConfig.IdType = FDCAN_STANDARD_ID;

	sFilterConfig.FilterIndex = 0;		//filter 1
	sFilterConfig.FilterType = FDCAN_FILTER_DUAL;
	sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	sFilterConfig.FilterID1 = 0x010;
	sFilterConfig.FilterID2 = 0x100;
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);

	sFilterConfig.FilterIndex = 1;		//filter 2
	sFilterConfig.FilterID1 = 0x200;
	sFilterConfig.FilterID2 = 0x201;
	HAL_FDCAN_ConfigFilter(&hfdcan1, &sFilterConfig);

	HAL_FDCAN_Start(&hfdcan1);
	HAL_FDCAN_ActivateNotification(&hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

	HAL_LPTIM_PWM_Start(&hlptim2, LPTIM_CHANNEL_1);		//EXTCOMIN Start

	// 2. LVGL 핵심 초기화
	lv_init();

	// 3. 디스플레이 버퍼 및 드라이버 초기화 (SHARP MIP 설정)
	static lv_disp_draw_buf_t draw_buf;
	static uint8_t buf_1[240 * (2 + 320 / 8) + 2]; // Sharp MIP 전용 Oversized 버퍼
	lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, 320 * 240);

	static lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = 320;
	disp_drv.ver_res = 240;
	disp_drv.flush_cb = sharp_mip_flush;
	disp_drv.rounder_cb = sharp_mip_rounder;
	disp_drv.set_px_cb = sharp_mip_set_px;
	disp_drv.draw_buf = &draw_buf;
	lv_disp_drv_register(&disp_drv);

	// 4. SquareLine UI 초기화
	ui_init();

	Debug_LED_ShowError(0);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		Light_Process();			//Light Update
		Dashboard_Update();		//UI Update
		lv_timer_handler();		//LVGL Update
		static uint32_t lastTxTick = 0;
		if (HAL_GetTick() - lastTxTick >= 100)
		{
			FDCAN_Send_ControlData();
			lastTxTick = HAL_GetTick();
		}

		HAL_Delay(5);
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_0;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV4;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 1;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Power Configuration
  * @retval None
  */
static void SystemPower_Config(void)
{

  /*
   * Switch to SMPS regulator instead of LDO
   */
  if (HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE BEGIN PWR */
/* USER CODE END PWR */
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_14B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_5CYCLE;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief FDCAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_FDCAN1_Init(void)
{

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.ClockDivider = FDCAN_CLOCK_DIV1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 8;
  hfdcan1.Init.NominalSyncJumpWidth = 1;
  hfdcan1.Init.NominalTimeSeg1 = 15;
  hfdcan1.Init.NominalTimeSeg2 = 4;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 0;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* USER CODE END FDCAN1_Init 2 */

}

/**
  * @brief GPDMA1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPDMA1_Init(void)
{

  /* USER CODE BEGIN GPDMA1_Init 0 */

  /* USER CODE END GPDMA1_Init 0 */

  /* Peripheral clock enable */
  __HAL_RCC_GPDMA1_CLK_ENABLE();

  /* GPDMA1 interrupt Init */
    HAL_NVIC_SetPriority(GPDMA1_Channel12_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel12_IRQn);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* USER CODE END GPDMA1_Init 2 */

}

/**
  * @brief ICACHE Initialization Function
  * @param None
  * @retval None
  */
static void MX_ICACHE_Init(void)
{

  /* USER CODE BEGIN ICACHE_Init 0 */

  /* USER CODE END ICACHE_Init 0 */

  /* USER CODE BEGIN ICACHE_Init 1 */

  /* USER CODE END ICACHE_Init 1 */

  /** Enable instruction cache in 1-way (direct mapped cache)
  */
  if (HAL_ICACHE_ConfigAssociativityMode(ICACHE_1WAY) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_ICACHE_Enable() != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ICACHE_Init 2 */

  /* USER CODE END ICACHE_Init 2 */

}

/**
  * @brief LPTIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPTIM2_Init(void)
{

  /* USER CODE BEGIN LPTIM2_Init 0 */

  /* USER CODE END LPTIM2_Init 0 */

  LPTIM_OC_ConfigTypeDef sConfig1 = {0};

  /* USER CODE BEGIN LPTIM2_Init 1 */

  /* USER CODE END LPTIM2_Init 1 */
  hlptim2.Instance = LPTIM2;
  hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim2.Init.Period = 1599;
  hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;
  hlptim2.Init.Input1Source = LPTIM_INPUT1SOURCE_GPIO;
  hlptim2.Init.Input2Source = LPTIM_INPUT2SOURCE_GPIO;
  hlptim2.Init.RepetitionCounter = 0;
  if (HAL_LPTIM_Init(&hlptim2) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig1.Pulse = 800;
  sConfig1.OCPolarity = LPTIM_OCPOLARITY_HIGH;
  if (HAL_LPTIM_OC_ConfigChannel(&hlptim2, &sConfig1, LPTIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPTIM2_Init 2 */

  /* USER CODE END LPTIM2_Init 2 */
  HAL_LPTIM_MspPostInit(&hlptim2);

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi1.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi1.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(&hspi1, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 159;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 999999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_ERR1_Pin|LED_ERR0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : User_Button_Pin STM_DIN2_Pin STM_DIN3_Pin */
  GPIO_InitStruct.Pin = User_Button_Pin|STM_DIN2_Pin|STM_DIN3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : STM_DIN0_Pin STM_DIN1_Pin STM_Button_6_Pin STM_Button_5_Pin
                           STM_Button_4_Pin */
  GPIO_InitStruct.Pin = STM_DIN0_Pin|STM_DIN1_Pin|STM_Button_6_Pin|STM_Button_5_Pin
                          |STM_Button_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_SCS_Pin */
  GPIO_InitStruct.Pin = SPI1_SCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_SCS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_ERR1_Pin LED_ERR0_Pin */
  GPIO_InitStruct.Pin = LED_ERR1_Pin|LED_ERR0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : STM_Button_1_Pin STM_Button_0_Pin */
  GPIO_InitStruct.Pin = STM_Button_1_Pin|STM_Button_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : STM_Button_7_Pin STM_Button_3_Pin */
  GPIO_InitStruct.Pin = STM_Button_7_Pin|STM_Button_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : STM_Button_2_Pin */
  GPIO_InitStruct.Pin = STM_Button_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(STM_Button_2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI13_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI13_IRQn);

  HAL_NVIC_SetPriority(EXTI14_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI14_IRQn);

  HAL_NVIC_SetPriority(EXTI15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef RxHeader;
	uint8_t RxData[8];

	if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData)
			== HAL_OK)
	{
		switch (RxHeader.Identifier)
		{
		case 0x010: 		//Fault/Err
		{
			uint8_t faultNum = RxData[0];
			uint8_t faultCode = RxData[1];

			if (faultNum > 0 && faultNum < 100)
			{
				Fault_Status[faultNum] = faultCode;
				Fault_Data_Changed = true;
			}
			else if (faultNum == 0)
			{

				Fault_Status[0] = faultCode;
			}
			break;
		}

		case 0x100: 		//Driving/Power
		{
			DrivingData.Speed = RxData[0];
			DrivingData.Recommend_Speed = RxData[1];
			DrivingData.MPPT_Power = RxData[2];
			DrivingData.Motor_Power = RxData[3];
			break;
		}

		case 0x200: 		//Status/Information1
		{
			DrivingData.Battery_SOC = RxData[0];

			DrivingData.Signal_State = (RxData[1] & 0x01) ? true : false; // Bit 0
			DrivingData.Battery_State = (RxData[1] & 0x02) ? true : false; // Bit 1

			InfoData.Time_Hour = RxData[2];
			InfoData.Time_Min = RxData[3];
			break;
		}

		case 0x201: 		//Status/Information2
		{
			InfoData.Info_1 = RxData[0];
			InfoData.Info_2 = RxData[1];

			InfoData.Info_3 = (uint16_t) ((RxData[3] << 8) | RxData[2]);

			InfoData.Info_4 = RxData[4];
			break;
		}
		}
	}
}
/* 디버깅 LED 에러 표시 함수 */
void Debug_LED_ShowError(uint8_t errorState)
{
	switch (errorState)
	{
	case 0: // 00 : 정상 상태 (모두 꺼짐)
	{
		HAL_GPIO_WritePin(LED_ERR1_GPIO_Port, LED_ERR1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_ERR0_GPIO_Port, LED_ERR0_Pin, GPIO_PIN_RESET);
		break;
	}
	case 1: // 01 : 에러 상태 1 (ERR1 켜짐, ERR0 꺼짐)
	{
		HAL_GPIO_WritePin(LED_ERR1_GPIO_Port, LED_ERR1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_ERR0_GPIO_Port, LED_ERR0_Pin, GPIO_PIN_RESET);
		break;
	}
	case 2: // 10 : 에러 상태 2 (ERR1 꺼짐, ERR0 켜짐)
	{
		HAL_GPIO_WritePin(LED_ERR1_GPIO_Port, LED_ERR1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_ERR0_GPIO_Port, LED_ERR0_Pin, GPIO_PIN_SET);
		break;
	}
	case 3: // 11 : 에러 상태 3 (모두 켜짐 - 크리티컬 에러)
	{
		HAL_GPIO_WritePin(LED_ERR1_GPIO_Port, LED_ERR1_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_ERR0_GPIO_Port, LED_ERR0_Pin, GPIO_PIN_SET);
		break;
	}
	default: // 예외 처리 (혹시 모를 값 방지, 모두 꺼짐)
	{
		HAL_GPIO_WritePin(LED_ERR1_GPIO_Port, LED_ERR1_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_ERR0_GPIO_Port, LED_ERR0_Pin, GPIO_PIN_RESET);
		break;
	}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// ACC ON/OFF 토글 (버튼 0)
	if (GPIO_Pin == STM_Button_0_Pin)
	{
		AccData.Acc_state = !AccData.Acc_state;
	}

	// ACC 속도 조절은 ACC가 ON 상태일 때만 동작
	else if (AccData.Acc_state)
	{
		// 속도 UP (버튼 1)
		if (GPIO_Pin == STM_Button_1_Pin)
		{
			AccData.Acc_speed++;
		}
		// 속도 DOWN (버튼 2)
		else if (GPIO_Pin == STM_Button_2_Pin)
		{
			if (AccData.Acc_speed > 0)
			{
				AccData.Acc_speed--;
			}
		}
	}
}

void FDCAN_Send_ControlData(void)
{
	FDCAN_TxHeaderTypeDef TxHeader;
	uint8_t TxData[8] =
	{ 0, }; // 배열을 0으로 싹 초기화 (Data[2]~[7]은 자동으로 0x00이 됨)

	// 송신 헤더 설정 (표에 맞춰 ID 0x050 설정)
	TxHeader.Identifier = 0x050;
	TxHeader.IdType = FDCAN_STANDARD_ID;
	TxHeader.TxFrameType = FDCAN_DATA_FRAME;
	TxHeader.DataLength = FDCAN_DLC_BYTES_8;
	TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
	TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
	TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	TxHeader.MessageMarker = 0;

	/* 1. Control Bits (Data[0]) 조합 */
	// 비트 시프트(<<) 와 OR 연산(|)을 이용해 1바이트 안에 여러 bool 값을 우겨넣습니다.
	// LSB(우측 끝)부터 비트 0으로 시작합니다.
	if (AccData.Acc_state)
	{
		TxData[0] |= (1 << 0); // Bit 0
	}
	if (InfoData.Test_Value1)
	{
		TxData[0] |= (1 << 1); // Bit 1
	}
	if (InfoData.Test_Value2)
	{
		TxData[0] |= (1 << 2); // Bit 2
	}
	if (InfoData.Test_Value3)
	{
		TxData[0] |= (1 << 3); // Bit 3
	}

	/* 2. ACC Speed (Data[1]) */
	TxData[1] = AccData.Acc_speed;

	/* 3. CAN 메시지 송신 */
	// 메일박스(Tx FIFO)에 남은 공간이 있는지 안전하게 확인 후 전송
	if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0)
	{
		HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData);
	}
}

extern lv_disp_drv_t * active_disp_drv;

/**
  * @brief  SPI 전송 완료 콜백 (DMA 전송이 완료되면 자동 호출됨)
  * @param  hspi: SPI handle
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    /* 우리가 화면을 그리는 데 사용하는 SPI1이 맞는지 확인 */
    if (hspi->Instance == SPI1)
    {
        /* 1. 전송이 완료되었으므로 CS 핀 비활성화 (LOW) */
        HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);

        /* 2. LVGL에 프레임 버퍼 전송이 완료되었음을 보고 */
        if (active_disp_drv != NULL)
        {
            lv_disp_flush_ready(active_disp_drv);
            active_disp_drv = NULL; /* 다음 프레임을 위해 포인터 초기화 */
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
