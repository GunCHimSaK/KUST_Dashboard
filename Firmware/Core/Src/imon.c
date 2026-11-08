/**
  ******************************************************************************
  * @file    imon.c
  * @brief   LM76202-Q1 전류 모니터(IMON) 읽기 - 구현
  ******************************************************************************
  */

#include "imon.h"
#include "main.h"

/* ⚠ ADC1이 CubeMX 프로젝트에서 빠지면 이 파일 전체가 비활성화됩니다.
 *
 * 2026-08-28 CubeMX 재생성 때 ADC1이 IP 목록에서 사라지면서
 * stm32u5xx_hal_conf.h의 HAL_ADC_MODULE_ENABLED가 주석 처리되고
 * MX_ADC1_Init()/hadc1 핸들이 없어져 빌드가 깨졌습니다.
 * (.ioc의 PB1은 아직 Signal=ADC1_IN16으로 남아 있는 불일치 상태)
 *
 * 복구 방법: CubeMX > Analog > ADC1 > IN16 체크 후 재생성.
 *   Resolution      : 14 bits
 *   Clock Prescaler : Asynchronous div by 1
 *   Rank 1 Channel  : IN16 (PB1), Single-ended
 *   Sampling Time   : 814.5 Cycles   <- 반드시. IMON_OUT 앞 직렬 100k 때문에
 *                                       줄이면 측정값이 실제보다 낮게 나옵니다.
 * 복구되면 아래 가드가 저절로 풀려 원래 코드가 다시 살아납니다.
 * (호출부인 main.c는 손대지 않았습니다) */
#ifdef HAL_ADC_MODULE_ENABLED

/* ADC 핸들은 CubeMX 규칙대로 main.c에 있습니다 */
extern ADC_HandleTypeDef hadc1;

#define IMON_SAMPLE_PERIOD_MS  20U   /* 최소 샘플 간격 */

static uint32_t s_filtered_x8 = 0;   /* raw * IMON_FILTER_N (분해능 유지용) */
static uint16_t s_raw         = 0;
static uint32_t s_last_tick   = 0;
static bool     s_primed      = false;

/**
  * @brief  ADC 단발 변환 1회 (블로킹, 약 52us)
  * @retval raw 값, 실패 시 이전 값
  */
static uint16_t IMON_ReadOnce(void)
{
	uint16_t v = s_raw;

	if (HAL_ADC_Start(&hadc1) != HAL_OK)
	{
		return v;
	}
	if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK)
	{
		v = (uint16_t) HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);

	return v;
}

void IMON_Init(void)
{
	/* 오프셋/선형성 보정 - U5 ADC는 사용 전 캘리브레이션 권장 */
	if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED) != HAL_OK)
	{
		Error_Handler();
	}

	s_raw          = IMON_ReadOnce();
	s_filtered_x8  = (uint32_t) s_raw * IMON_FILTER_N;
	s_last_tick    = HAL_GetTick();
	s_primed       = true;
}

void IMON_Update(void)
{
	if (!s_primed)
	{
		return;
	}

	uint32_t now = HAL_GetTick();
	if ((now - s_last_tick) < IMON_SAMPLE_PERIOD_MS)
	{
		return;
	}
	s_last_tick = now;

	uint16_t sample = IMON_ReadOnce();

	/* 지수이동평균 : filt = filt - filt/N + sample */
	s_filtered_x8 = s_filtered_x8 - (s_filtered_x8 / IMON_FILTER_N) + sample;
	s_raw = (uint16_t) (s_filtered_x8 / IMON_FILTER_N);
}

uint16_t IMON_GetRaw(void)
{
	return s_raw;
}

uint16_t IMON_GetMilliVolts(void)
{
	return (uint16_t) (((uint32_t) s_raw * IMON_VREF_MV) / IMON_ADC_MAX);
}

uint16_t IMON_GetLoadCurrent_mA(void)
{
	uint32_t mv = IMON_GetMilliVolts();

	if (mv <= IMON_OFFSET_MV)
	{
		return 0;
	}

	/* I[mA] = (V - Voffset)[mV] * 10000 / (mV/A x10) */
	return (uint16_t) (((mv - IMON_OFFSET_MV) * 10000UL) / IMON_MV_PER_A_X10);
}

bool IMON_IsValid(void)
{
	return (IMON_GetLoadCurrent_mA() >= IMON_MIN_VALID_MA);
}

#else  /* HAL_ADC_MODULE_ENABLED 미정의 -> ADC1이 프로젝트에 없음 */

/* ADC가 없는 동안 링크가 되도록 하는 무동작 스텁.
 * IMON_IsValid()가 항상 false를 돌려주므로, 이 값을 쓰는 쪽은
 * '측정 불가'로 처리하면 됩니다. */
void     IMON_Init(void)               { }
void     IMON_Update(void)             { }
uint16_t IMON_GetRaw(void)             { return 0U; }
uint16_t IMON_GetMilliVolts(void)      { return 0U; }
uint16_t IMON_GetLoadCurrent_mA(void)  { return 0U; }
bool     IMON_IsValid(void)            { return false; }

#endif /* HAL_ADC_MODULE_ENABLED */
