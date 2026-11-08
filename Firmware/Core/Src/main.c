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
#include "dashboard_test.h"
#include "dashboard_ui.h"
#include "imon.h"
#include "can_bus.h"
#include "can_text.h"
#include "can_app.h"
#include "debug_log.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* ⚠ 전원 공급 방식(SMPS/LDO)과 시스템 클럭 소스는 CubeMX 설정입니다.
 *   이 파일에 스위치를 두지 마세요 -- 재생성 때 덮여 사라집니다.
 *
 *   전원 : displaycontrol.ioc 의 PWR.PowerMode = PWR_SMPS_SUPPLY
 *          -> SystemPower_Config() 의 HAL_PWREx_ConfigSupply()
 *          이 보드는 SMPS 인덕터가 실장되어 있어 SMPS로 동작합니다.
 *   클럭 : .ioc 의 PH0/PH1 = HSE-External-Oscillator, PLL 소스 = HSE
 *          -> SystemClock_Config() (HSE 16MHz -> PLL -> SYSCLK 160MHz)
 *          FDCAN 커널클럭도 HSE 직결이라 500kbit/s 타이밍이 여기 묶여 있습니다
 *          (stm32u5xx_hal_msp.c 의 HAL_FDCAN_MspInit 참고).
 *
 *   바꾸려면 CubeMX에서 .ioc를 고치고 재생성하세요.
 *   (예전에 USE_SMPS_SUPPLY / USE_HSE_CRYSTAL 매크로가 여기 있었지만
 *    #if로 참조하는 곳이 한 군데도 없는 죽은 코드였습니다. 0으로 바꿔도
 *    아무 효과가 없으면서 효과가 있는 것처럼 보여 위험해 삭제했습니다) */

/* 방향지시등/비상등 표시 스위치
 *   0 = 항상 소등
 *   1 = Switch_Process()가 읽은 LightData대로 표시   <- 현재 값
 *
 * 입력 극성은 main.h의 DIN_ACTIVE_HIGH로 정합니다. 현재 0(Active-LOW)이고,
 * 회로도 U9(TLP291-4GB) + 외부 10K 풀업(R30/R31/R35/R36) 기준으로 맞는
 * 값입니다. 미결선 상태에서는 풀업이 HIGH로 잡아 주므로 '소등'으로 읽힙니다.
 * (근거는 main.h의 DIN_ACTIVE_HIGH 주석)
 *
 * 현재 Switch_Process()는 DIN_1(우측)만 읽습니다. DIN_0(좌측) 배선이
 * 끝나면 Switch_Process() 안의 주석을 되살려 좌측/비상등을 켜세요. */
#define DASH_LIGHTS_ENABLE  1

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

FDCAN_HandleTypeDef hfdcan1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef handle_GPDMA1_Channel12;

TIM_HandleTypeDef htim3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void SystemPower_Config(void);
static void MX_GPIO_Init(void);
static void MX_GPDMA1_Init(void);
static void MX_ICACHE_Init(void);
static void MX_FDCAN1_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void Switch_Process(void);
void Dashboard_ApplyInputs(void);
static void Dashboard_ShowZeros(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*
 * ===== displaycontrol 보드 (STM32U545CETxQ / LQFP48) =====
 * test_nucleo(STM32U545RETxQ / LQFP64)에서 검증된 MIP LCD 코드를 옮겨온 것입니다.
 * 표시 로직(LVGL/UI/dashboard)은 그대로이고 핀 배치만 바뀌었습니다.
 *
 *   SPI1_SCK  : PA1  -> PA5
 *   SPI1_SCS  : PA6  (동일)
 *   SPI1_MOSI : PA7  (동일)
 *   EXTCOMIN  : PB0 / TIM3_CH3 (동일)
 *   버튼      : PA8/PB10/PA2 -> PA0~PA4 (5개)
 *   입력      : PB6/PB4/PB5  -> DIN_0~DIN_3 (PA8, PB15, PB14, PB13)
 *   상태 LED  : PA5  -> PC13
 *
 * USART1(PA9/PA10), I2C1(PB6/PB7)은 아직 사용하지 않아 비워 둡니다.
 *
 * ===== 입력 사양 =====
 *   BT_0 (PA0) : ACC ON  -> 설정속도 +1
 *                ACC OFF -> 실측 0x501 회생 프레임 10초 재생
 *                           (66km/h / 79% / -2250W. 음수 전력 표시 검증용)
 *   BT_1 (PA1) : ACC on/off 토글
 *   BT_2 (PA2) : ACC ON  -> 설정속도 -1
 *                ACC OFF -> 화면 메시지 확인(지우고 로고 복귀)
 *   BT_3 (PA3) : 0x500 텍스트 셀프테스트 주입
 *   BT_4 (PA4) : 예약 (EXTI만 등록, 동작 없음)
 *
 *   DIN_0 (PA8)  : 좌측 방향지시등
 *   DIN_1 (PB15) : 우측 방향지시등
 *     -> 둘 다 ON 이면 비상등
 *   DIN_2 (PB14), DIN_3 (PB13) : 미사용
 *
 * 버튼은 EXTI(하강엣지 + 디바운스), DIN은 메인 루프 폴링으로 읽습니다.
 * 두 입력의 결과는 vehicle_data.c의 AccData / LightData에 담기고,
 * Dashboard_ApplyInputs()가 매 루프 화면에 반영합니다.
 * (dashboard_test.c의 시뮬레이터는 ACC/방향지시등을 건드리지 않습니다)
 */

/**
  * @brief  속도를 제외한 모든 표시 값을 0으로 한 번 세팅
  * @note   시뮬레이터(dashboard_test.c)를 더 이상 돌리지 않으므로, 부팅 때
  *         한 번만 0을 찍고 그대로 둡니다. 값이 안 바뀌면 dashboard_ui의
  *         캐시가 LVGL 호출을 막아 화면을 다시 그리지 않습니다.
  *
  *         표시할 항목이 CAN으로 확정되면 can_app.c의 CanApp_ApplyToUI()에
  *         하나씩 추가하면 됩니다.
  */
static void Dashboard_ShowZeros(void)
{
	DashboardUI_SetSpeed(0);
	DashboardUI_SetRecommend(0);
	DashboardUI_SetCruise(0);
	DashboardUI_SetDte(0);
	DashboardUI_SetMppt(0);
	DashboardUI_SetMotor(0);
	DashboardUI_SetBattery(0);
	DashboardUI_SetBar(0);
	DashboardUI_SetTime(0, 0);
	DashboardUI_SetFault(0, 0);
	DashboardUI_SetPos(DASH_POS_NEUTRAL);
	DashboardUI_SetRegen(DASH_REGEN_OFF);
}

/**
  * @brief  DIN_1을 폴링해서 우측 방향지시등 상태를 갱신
  * @note   현재 DIN_1(우측)만 사용합니다. 500ms 점멸은 표시 계층에서 처리합니다.
  *         입력 극성은 main.h의 DIN_ACTIVE_HIGH로 바꿉니다.
  */
void Switch_Process(void)
{
	/* 지금은 DIN_1(우측 방향지시등)만 사용합니다.
	 *
	 * DIN_0은 배선이 아직 확정되지 않아 의도적으로 무시합니다.
	 * 극성은 main.h의 DIN_ACTIVE_HIGH = 0 (Active-LOW)이고, 외부 10K 풀업이
	 * 미결선 상태를 HIGH = '소등'으로 잡아 줍니다.
	 * DIN_0 배선이 끝나면 아래 주석을 되살리세요.
	 *
	 *   bool din0 = DIN_IS_ON(DIN_0_GPIO_Port, DIN_0_Pin);   // 좌측
	 *   LightData.Hazard_Signal = ( din0 &&  din1);
	 *   LightData.Left_Signal   = ( din0 && !din1);
	 *   LightData.Right_Signal  = (!din0 &&  din1);
	 */
	bool din1 = DIN_IS_ON(DIN_1_GPIO_Port, DIN_1_Pin);   /* 우측 */

	LightData.Left_Signal   = false;
	LightData.Right_Signal  = din1;
	LightData.Hazard_Signal = false;

	/* DIN_0, DIN_2, DIN_3: 미사용 */
}
/**
  * @brief  버튼/DIN 입력 결과를 화면에 반영 (메인 루프에서 매 회 호출)
  * @note   방향지시등 점멸은 DashboardUI_UpdateLights() 내부에서 처리하므로
  *         매 루프 정확히 1회만 호출해야 합니다.
  */
void Dashboard_ApplyInputs(void)
{
	/* ---- ACC (크루즈) : BT_1로 표시 on/off, BT_0/BT_2로 설정속도 ---- */
	DashboardUI_ShowCruise(AccData.Acc_state);
	if (AccData.Acc_state)
	{
		DashboardUI_SetCruise(AccData.Acc_speed);
	}

	/* ---- 방향지시등/비상등 ----
	 * 500ms 점멸은 DashboardUI_UpdateLights() 내부에서 처리하므로
	 * 매 루프 1회만 호출해야 합니다 (여러 번 호출하면 점멸이 깨집니다). */
#if DASH_LIGHTS_ENABLE
	DashboardUI_UpdateLights(LightData.Left_Signal,
	                         LightData.Right_Signal,
	                         LightData.Hazard_Signal);
#else
	/* 전부 소등 유지. show_obj가 상태 변화 시에만 invalidate하므로
	 * 첫 호출 이후로는 LVGL을 건드리지 않습니다. */
	DashboardUI_UpdateLights(false, false, false);
#endif
}

/* 그 밖의 표시 값은 dashboard_test.c(시뮬레이터)가 공급합니다.
 * 표시 값은 can_app.c의 CanApp_ApplyToUI()가 CAN 0x501에서 공급합니다.
 * (dashboard_test.c 시뮬레이터는 더 이상 호출하지 않습니다) */
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
  MX_ICACHE_Init();
  MX_FDCAN1_Init();
  MX_SPI1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

	// SPI1 커널클럭을 HSI16으로 전환 후 8분주 -> 정확히 2.0MHz (LS044Q7DH01 최대 스펙)
	// 기본 설정(160MHz/128 = 1.25MHz)보다 1.6배 빨라져 전체화면 갱신 64ms -> 40ms.
	// (MX_SPI1_Init 이후, 첫 SPI 사용 전에 실행. USER CODE 구역이라 재생성에도 안전)
	{
		__HAL_RCC_HSI_ENABLE();
		while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSIRDY) == 0U) { }

		RCC_PeriphCLKInitTypeDef spiClk = {0};
		spiClk.PeriphClockSelection = RCC_PERIPHCLK_SPI1;
		spiClk.Spi1ClockSelection = RCC_SPI1CLKSOURCE_HSI;
		if (HAL_RCCEx_PeriphCLKConfig(&spiClk) != HAL_OK)
		{
			Error_Handler();
		}
		// 분주비 128 -> 8 (SPI가 아직 비활성 상태이므로 레지스터 직접 수정 가능)
		MODIFY_REG(SPI1->CFG1, SPI_CFG1_MBR, SPI_BAUDRATEPRESCALER_8);

		// ⚠ 핸들의 값도 같이 맞춰 둡니다. 레지스터만 고치면 핸들에는 128이
		//    남아, 나중에 누가 HAL_SPI_Init(&hspi1)을 다시 부르는 순간
		//    커널클럭은 HSI16인 채로 128분주가 복원되어 125kHz로 떨어집니다.
		//    (전체화면 10,082바이트 = 645ms -> SHARP_MIP_DMA_TIMEOUT_MS 150ms
		//     워치독이 매 프레임 터지고 화면이 깨집니다)
		hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
	}

	// 디스플레이 EXTCOMIN용 PWM 시작 (2Hz, 50% - VCOM 반전용, EXTMODE=H 전제)
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

	// 디스플레이 전원 안정화 대기 후 패널 메모리 초기화 (파워온 랜덤 픽셀 제거)
	HAL_Delay(100);
	sharp_mip_clear();

	// 1. LVGL 핵심 초기화
	lv_init();

	// 2. 디스플레이 버퍼 및 드라이버 초기화 (SHARP MIP 설정)
	static lv_disp_draw_buf_t draw_buf;
	static uint8_t buf_1[240 * (2 + 320 / 8) + 2]; // Sharp MIP 전용 Oversized 버퍼
	lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, 320 * 240);

	static lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);
#if SHARP_MIP_ROTATION == 90 || SHARP_MIP_ROTATION == 270
	disp_drv.hor_res = SHARP_MIP_PANEL_VER;   // 240 (세로 모드)
	disp_drv.ver_res = SHARP_MIP_PANEL_HOR;   // 320
#else
	disp_drv.hor_res = SHARP_MIP_PANEL_HOR;   // 320 (가로 모드)
	disp_drv.ver_res = SHARP_MIP_PANEL_VER;   // 240
#endif
	disp_drv.flush_cb = sharp_mip_flush;
	disp_drv.rounder_cb = sharp_mip_rounder;
	disp_drv.set_px_cb = sharp_mip_set_px;
	disp_drv.draw_buf = &draw_buf;
	lv_disp_drv_register(&disp_drv);

	// 3. SquareLine UI 초기화
	ui_init();

	// 4. 계기판 UI 초기화 + 표시 값을 0으로 (시뮬레이터는 더 이상 쓰지 않음).
	//    속도는 CAN 0x501에서만 오고, 안 들어오면 0을 표시합니다.
	DashboardUI_Init();
	Dashboard_ShowZeros();

	// 5. 전류 모니터(LM76202-Q1 IMON) 및 CAN 시작
	IMON_Init();
	CanBus_Init();
	CanText_Init();				// 0x500 텍스트 메시지 조립기
	CanApp_Init();				// 0x501 텔레메트리 / 0x510 명령
	DBG_Init();					// SWO 로그 + g_dbg 진단 스냅샷

	AccData.Acc_speed = 80;   // ACC 설정속도 기본값 (버튼 로직 유지용)

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		// 하트비트(PC13): 500ms 주기로 30ms만 짧게 점멸.
		// 루프가 멈추면 LED는 꺼진 상태로 정지하므로 다른 패턴과 구분됩니다.
		static uint32_t hbTick = 0;
		uint32_t hbNow = HAL_GetTick();
		if ((hbNow - hbTick) < 30) { LED_ON(); } else { LED_OFF(); }
		if (hbNow - hbTick >= 500)
		{
			hbTick = hbNow;
		}

		Switch_Process();			// DIN 폴링 (방향지시등/비상등)
		IMON_Update();				// 전류 모니터 샘플링 (내부에서 20ms 간격 유지)
		CanBus_Process();			// 수신 CAN 프레임 -> CanBus_OnFrame() 디스패치

		// 지상국 텍스트 메시지(0x500) 회수.
		// CanBus_Process() 안에서 조립되므로 반드시 그 뒤에 호출할 것.
		//
		// 완성된 메시지는 ui_Text(로고 자리)에 우측 정렬로 표시됩니다.
		// 12글자를 넘으면 전광판처럼 한 글자씩 밀며 순환합니다.
		// 수신 이력은 g_dbg.txt_last / g_dbg.txt_msgs 로도 확인할 수 있습니다.
		{
			char canMsg[CAN_TEXT_MAX_BYTES + 1];
			if (CanText_Poll(canMsg, sizeof(canMsg)))
			{
				DashboardUI_ShowMessage(canMsg);	// 표시 중이면 즉시 교체
				DBG_NoteMessage(canMsg);	// g_dbg.txt_last 기록 + 로그
			}
		}
		// 차량 텔레메트리(0x501) 반영 + 명령(0x510) 주기 송신.
		// 표시하는 건 차속 하나뿐이고, 0x501이 600ms 넘게 끊기면 0으로 떨어진다.
		// 나머지 표시 값은 부팅 때 Dashboard_ShowZeros()로 0을 찍고 고정.
		// 0x510(ACC 명령): byte0 = 설정속도, byte1 = ACC on/off.
		// 보드가 0x420으로 중계 -> 모터 팀이 수신. 앞 2바이트만 전달됩니다.
		CanApp_SetAccCommand(AccData.Acc_state, AccData.Acc_speed);
		CanApp_Process();
		CanApp_ApplyToUI();			// 0x501 차속 반영 (수신 없으면 0)
		Dashboard_ApplyInputs();	// ACC/방향지시등을 실제 입력으로 갱신
		DashboardUI_UpdateMessage();	// 전광판 스크롤 + BT_2 확인 처리
		sharp_mip_service();		// DMA 완료 인터럽트 유실 감시 (워치독)
		DBG_Process();				// 루프 주기 측정 + 주기 진단 덤프 + 셀프테스트
		{
			uint32_t lvT0 = HAL_GetTick();
			lv_timer_handler();		// LVGL 갱신
			DBG_NoteLvglMs(HAL_GetTick() - lvT0);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMBOOST = RCC_PLLMBOOST_DIV1;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLLVCIRANGE_0;
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
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 4;
  hfdcan1.Init.NominalTimeSeg1 = 27;
  hfdcan1.Init.NominalTimeSeg2 = 4;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 1;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */

  /* 비트레이트 : 커널클럭(HSE 16MHz) / 1 / (1 + 27 + 4)tq = 500 kbit/s
   * 샘플포인트 = (1 + 27) / 32 = 87.5%  (CiA 권장)
   * 필터/인터럽트 설정과 Start는 CanBus_Init()에서 합니다. */

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
    HAL_NVIC_SetPriority(GPDMA1_Channel12_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel12_IRQn);

  /* USER CODE BEGIN GPDMA1_Init 1 */

  /* USER CODE END GPDMA1_Init 1 */
  /* USER CODE BEGIN GPDMA1_Init 2 */

  /* 채널 설정(SPI1_TX 요청, MEM->PERIPH)과 __HAL_LINKDMA는
   * stm32u5xx_hal_msp.c의 HAL_SPI_MspInit()에서 수행됩니다. */

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
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_LSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_ENABLE;
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
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 15999;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 4999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 2500;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

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
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BT_0_Pin BT_1_Pin BT_2_Pin BT_3_Pin
                           BT_4_Pin */
  GPIO_InitStruct.Pin = BT_0_Pin|BT_1_Pin|BT_2_Pin|BT_3_Pin
                          |BT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_SCS_Pin */
  GPIO_InitStruct.Pin = SPI1_SCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(SPI1_SCS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : IMON_OUT_Pin */
  GPIO_InitStruct.Pin = IMON_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IMON_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIN_3_Pin DIN_2_Pin DIN_1_Pin */
  GPIO_InitStruct.Pin = DIN_3_Pin|DIN_2_Pin|DIN_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DIN_0_Pin */
  GPIO_InitStruct.Pin = DIN_0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DIN_0_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  버튼 인터럽트 콜백 (디바운스 + 릴리즈 바운스 필터 포함)
  * @note   버튼은 Active-LOW (누르면 GND) → FALLING 엣지에서 동작.
  *         STM32U5 HAL은 HAL_GPIO_EXTI_Callback이 아니라
  *         HAL_GPIO_EXTI_Rising_Callback / Falling_Callback을 호출합니다!
  *         DIN_0/DIN_1(방향지시등)은 Switch_Process()가 폴링으로 처리합니다.
  */
void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
	static uint32_t lastTick[5] = {0};
	uint8_t idx;
	GPIO_TypeDef *port;

	if (GPIO_Pin == BT_0_Pin)      { idx = 0; port = BT_0_GPIO_Port; }
	else if (GPIO_Pin == BT_1_Pin) { idx = 1; port = BT_1_GPIO_Port; }
	else if (GPIO_Pin == BT_2_Pin) { idx = 2; port = BT_2_GPIO_Port; }
	else if (GPIO_Pin == BT_3_Pin) { idx = 3; port = BT_3_GPIO_Port; }
	else if (GPIO_Pin == BT_4_Pin) { idx = 4; port = BT_4_GPIO_Port; }
	else return;                   // BT_0~4 외의 핀은 무시

	uint32_t now = HAL_GetTick();
	if (now - lastTick[idx] < 120) return;  // 디바운스 120ms

	// 버튼을 뗄 때 생기는 바운스도 falling 엣지를 만들 수 있으므로,
	// 실제로 눌린 상태(LOW)인지 레벨을 한 번 더 확인해서 유령 입력을 거른다
	if (HAL_GPIO_ReadPin(port, GPIO_Pin) != GPIO_PIN_RESET) return;

	lastTick[idx] = now;

	if (GPIO_Pin == BT_1_Pin)
	{
		// BT_1: ACC on/off 토글 (OFF면 크루즈 표시가 화면에서 숨겨짐)
		AccData.Acc_state = !AccData.Acc_state;
	}
	else if (GPIO_Pin == BT_3_Pin)
	{
		// BT_3: 0x500 셀프테스트 메시지 주입 (CAN 하드웨어 없이 표시 경로 검증).
		// 실제 주입은 메인 루프의 DBG_Process()에서 실행합니다.
		// ISR에서 조립기를 직접 건드리면 CanText_Poll()과 경합합니다.
		DBG_RequestSelfTest();
	}
	else if (GPIO_Pin == BT_0_Pin)
	{
		// BT_0은 ACC 상태에 따라 두 가지 역할을 합니다.
		if (AccData.Acc_state)
		{
			// ACC ON: 설정속도 +1 (상한 150은 임의값이니 필요하면 조정하세요)
			if (AccData.Acc_speed < 150)
			{
				AccData.Acc_speed++;
			}
		}
		else
		{
			// ACC OFF: 보드 팀이 캡처한 실측 0x501 '회생 최대' 프레임을 10초간 재생.
			// 기대 표시 -> 66 km/h / 79% / -2250W (음수 전력 표시 검증)
			//
			// ACC가 꺼져 있으면 설정속도 조절이 어차피 동작하지 않으므로,
			// 그 상태의 BT_0을 시험용으로 씁니다. 버튼을 새로 배정하지 않아도
			// 되고, ACC를 켜면 원래 기능으로 자동 복귀합니다.
			//
			// BT_3과 같은 이유로 여기서는 플래그만 세웁니다
			// (실제 주입은 메인 루프의 DBG_Process()에서 실행).
			DBG_RequestCapturedFrame();
		}
	}
	else if (GPIO_Pin == BT_2_Pin)
	{
		// BT_2도 ACC 상태에 따라 두 가지 역할을 합니다.
		if (AccData.Acc_state)
		{
			// ACC ON: 설정속도 -1
			if (AccData.Acc_speed > 0)
			{
				AccData.Acc_speed--;
			}
		}
		else
		{
			// ACC OFF: 화면에 떠 있는 지상국 메시지를 '확인'하고 지웁니다.
			//          메시지가 사라지면 KUST 로고가 다시 나옵니다.
			//          플래그만 세우고 실제 처리는 메인 루프에서 합니다.
			DashboardUI_AckMessage();
		}
	}
	// BT_4 : 예약 (EXTI만 등록, 동작 없음)
}

/**
  * @brief  LVGL assert(메모리 부족 등) 발생 시 호출됨 (lv_conf.h LV_ASSERT_HANDLER)
  * @note   LED(PC13) 패턴: 길게 켜짐(400ms) + 짧게 꺼짐(100ms) 반복
  */
void app_lvgl_assert(void)
{
	__disable_irq();
	while (1)
	{
		LED_ON();
		for (volatile uint32_t i = 0; i < 16000000; i++) { } // ~400ms
		LED_OFF();
		for (volatile uint32_t i = 0; i < 4000000; i++) { }  // ~100ms
	}
}

/**
  * @brief  SPI 전송 완료 콜백 (DMA 전송이 완료되면 자동 호출됨)
  * @note   실제 처리(SCS 내리기 + lv_disp_flush_ready)는 드라이버 안에 있습니다.
  *         SHARP_MIP.c의 sharp_mip_tx_complete_isr() 참고.
  * @param  hspi: SPI handle
  * @retval None
  */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        sharp_mip_tx_complete_isr();
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
  LED_ON();   /* LED 켜짐 유지 = Error_Handler 진입 표시 */
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
