/**
  ******************************************************************************
  * @file    dashboard_test.h
  * @brief   계기판 전체 구성요소 테스트 시뮬레이터
  *
  *          모든 표시 요소에 가상 값을 주기적으로 공급하여 UI 전체를 검증한다.
  *          UI 접근은 전부 dashboard_ui.h 를 통해서만 수행하므로,
  *          실전 전환 시 이 모듈 호출만 CAN 데이터 처리부로 교체하면 된다.
  *
  *          시나리오 (요구사항):
  *           - 5초 주기 : POS(DRIVE→NEUTRAL→REVERSE 순환),
  *                        REGEN(OFF→LOW→MID→HIGH 순환),
  *                        시간 +1분(09:30 시작), FaultCount +1(최대 100)
  *           - 왕복 스윕: DTE 0~467km, MPPT 0~300W, MOTOR 0~200W,
  *                        BATTERY 0~100%, FaultCode 0~99,
  *                        SPEED 50~100, RECOMMEND 70~100,
  *                        BAR 0~100
  *           - 항상 표시: 배터리 경고 아이콘
  *           - ACC(크루즈)와 방향지시등은 시뮬레이션하지 않음
  *                        -> 실제 버튼/DIN 입력이 main.c에서 직접 구동
  ******************************************************************************
  */
#ifndef __DASHBOARD_TEST_H
#define __DASHBOARD_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

/* 스윕 값 갱신 주기 [ms] - 낮출수록 빠르게 움직임 */
#define DASH_TEST_SWEEP_PERIOD_MS   200U
/* 상태(POS/REGEN/시간/FaultCount) 갱신 주기 [ms] */
#define DASH_TEST_SLOW_PERIOD_MS    5000U

/* [임시 진단] 1 = 속도만 갱신하고 나머지 표시 값은 전부 고정.
 * 다시 그릴 영역이 속도 숫자만 남아 루프가 크게 빨라집니다.
 * 이래도 화면에서 주춤하면 원인은 펌웨어가 아니라 패널입니다.
 * 확인 끝나면 0으로 되돌리세요. */
#ifndef DASH_TEST_SPEED_ONLY
#define DASH_TEST_SPEED_ONLY  0
#endif

/* ui_init() 이후 1회 호출 (내부에서 DashboardUI_Init 수행) */
void DashboardTest_Init(void);

/* 메인 루프에서 매 회 호출 */
void DashboardTest_Update(void);

/* 속도 시뮬레이션 억제.
 * CAN 0x501로 실제 차속이 들어오는 동안 시뮬레이터가 속도를 덮어쓰면
 * 두 값이 번갈아 표시되어 화면이 떨립니다. true를 주면 시뮬레이터가
 * 속도만 건드리지 않고 나머지(전력/SOC/기어/시간 등)는 계속 공급합니다. */
void DashboardTest_SuppressSpeed(bool suppress);

#ifdef __cplusplus
}
#endif

#endif /* __DASHBOARD_TEST_H */
