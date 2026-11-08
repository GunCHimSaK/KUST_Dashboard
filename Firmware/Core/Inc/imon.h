/**
  ******************************************************************************
  * @file    imon.h
  * @brief   LM76202-Q1 전류 모니터(IMON) 읽기
  ******************************************************************************
  * 회로 (schematic 기준)
  *
  *   LM76202-Q1 IMON ─┬─ R4 100k ─→ IMON_OUT ─→ PB1 (ADC1_IN16)
  *                    └─ R5 20k  ─→ RTN
  *
  *   - R5(20k)가 데이터시트의 R(IMON) 이고, IMON 핀의 전류를 전압으로 바꿉니다.
  *   - R4(100k)는 입력 역극성 고장 시 ADC 보호용 직렬저항입니다
  *     (데이터시트 8.3.5.4 권장). ADC 입력은 고임피던스라 전압 분배는 없고,
  *     대신 샘플링 시간을 아주 길게 잡아야 합니다 (imon.c 참고).
  *
  * 변환식 (데이터시트 SLVSEM1A, 식 7)
  *
  *   V(IMON) = I(OUT) x GAIN(IMON) x R(IMON)
  *
  *   GAIN(IMON) = 78.28 uA/A (typ, 0.1A <= I(OUT) <= 2A, min 72 / max 85)
  *   R(IMON)    = 20 kOhm
  *   => 1 A 당 1565.6 mV
  *
  *   I(OUT) < 50mA 구간은 오프셋 전류 I(IMON_OS) = 2uA (typ) 가 지배적이라
  *   선형성이 없습니다. 이 구간은 IMON_IsValid()가 false를 돌려줍니다.
  ******************************************************************************
  */

#ifndef IMON_H_
#define IMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ── 보드/부품 상수 : 저항이나 칩이 바뀌면 여기만 고치면 됩니다 ────────── */
#define IMON_RSENSE_OHM        20000UL   /* R5 */
#define IMON_GAIN_UA_PER_A_X100 7828UL   /* 78.28 uA/A (x100 정수) */
#define IMON_OFFSET_UA          2UL      /* I(IMON_OS) typ */
#define IMON_VREF_MV           3300UL    /* VDDA/VREF+ */
#define IMON_ADC_MAX           16383UL   /* ADC1 14bit full scale */

/* 1A 당 IMON 전압 [mV x10] = 78.28uA/A * 20000ohm = 1565.6mV/A */
#define IMON_MV_PER_A_X10      ((IMON_GAIN_UA_PER_A_X100 * IMON_RSENSE_OHM) / 10000UL)
/* 무부하 오프셋 전압 [mV] = 2uA * 20k = 40mV */
#define IMON_OFFSET_MV         ((IMON_OFFSET_UA * IMON_RSENSE_OHM) / 1000UL)
/* 선형 구간 하한 (데이터시트상 50mA) */
#define IMON_MIN_VALID_MA      50UL

/* 지수이동평균 필터 계수 : new = (old*(N-1) + sample)/N  (N은 2의 거듭제곱) */
#define IMON_FILTER_N          8UL

/* ── API ──────────────────────────────────────────────────────────────── */

/** ADC 캘리브레이션 후 첫 샘플을 받아 필터를 초기화합니다.
  * MX_ADC1_Init() 이후에 한 번 호출하세요. */
void     IMON_Init(void);

/** 주기적으로 호출 (권장 10~100ms). 내부에서 최소 간격을 지키므로
  * 메인 루프에서 매번 불러도 됩니다. 1회 변환에 약 52us 소요. */
void     IMON_Update(void);

/** 마지막 ADC raw 값 (필터 적용 후, 0~16383) */
uint16_t IMON_GetRaw(void);

/** IMON_OUT 핀 전압 [mV] */
uint16_t IMON_GetMilliVolts(void);

/** 환산된 부하 전류 [mA]. 선형 구간 밖이면 0 */
uint16_t IMON_GetLoadCurrent_mA(void);

/** 측정값이 데이터시트 선형 구간(>=50mA) 안에 있는지 */
bool     IMON_IsValid(void);

#ifdef __cplusplus
}
#endif

#endif /* IMON_H_ */
