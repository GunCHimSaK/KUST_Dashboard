/**
  ******************************************************************************
  * @file    dashboard_test.c
  * @brief   계기판 전체 구성요소 테스트 시뮬레이터 구현
  ******************************************************************************
  */
#include "dashboard_test.h"
#include "dashboard_ui.h"
#include "main.h"           /* HAL_GetTick */

/* ---- 왕복 스윕 값 --------------------------------------------------------
 * step은 한 스텝(200ms)당 증감량. 범위가 넓은 항목은 step을 키워
 * 한 방향 왕복에 10~20초 정도 걸리도록 맞춤. */
typedef struct
{
  int16_t val;
  int16_t min;
  int16_t max;
  int16_t step;
  int8_t  dir;
} Sweep_t;

static Sweep_t sw_dte       = {   0, 0, 467, 5, 1 };  /* ~19초/편도 */
static Sweep_t sw_mppt      = {   0, 0, 300, 3, 1 };  /* ~20초/편도 */
static Sweep_t sw_motor     = {   0, 0, 200, 2, 1 };  /* ~20초/편도 */
static Sweep_t sw_battery   = {   0, 0, 100, 1, 1 };  /* ~20초/편도 */
static Sweep_t sw_fcode     = {   0, 0,  99, 1, 1 };  /* ~20초/편도 */
static Sweep_t sw_speed     = {  50, 50, 100, 1, 1 }; /* ~10초/편도 */
static Sweep_t sw_recommend = {   0, 0,  99, 1, 1 };  /* ~20초/편도 */
static Sweep_t sw_bar       = {   0, 0, 100, 1, 1 };  /* ~20초/편도 */

/* ---- 5초 주기 상태 -------------------------------------------------------- */
static DashPos_t   simPos     = DASH_POS_DRIVE;
static DashRegen_t simRegen   = DASH_REGEN_OFF;
static uint8_t     simHour    = 9;
static uint8_t     simMin     = 30;
static uint8_t     simFCount  = 0;


static uint32_t sweepTick = 0;
static uint32_t slowTick  = 0;

/* CAN 0x501이 실제 차속을 공급하는 동안 true.
 * 이때 시뮬레이터는 속도만 건너뛰고 나머지 값은 계속 갱신합니다. */
static bool s_suppress_speed = false;

void DashboardTest_SuppressSpeed(bool suppress)
{
  s_suppress_speed = suppress;
}

/* 스윕 한 스텝 진행 후 현재 값 반환 */
static int16_t sweep_step(Sweep_t *s)
{
  s->val += (int16_t)(s->step * s->dir);
  if (s->val >= s->max) { s->val = s->max; s->dir = -1; }
  if (s->val <= s->min) { s->val = s->min; s->dir =  1; }
  return s->val;
}

/* 현재 상태를 UI에 반영 (초기 1회 + 주기 갱신에서 공용) */
static void push_all(void)
{
  if (!s_suppress_speed) DashboardUI_SetSpeed((uint8_t)sw_speed.val);
  DashboardUI_SetRecommend((uint8_t)sw_recommend.val);
  DashboardUI_SetDte((uint16_t)sw_dte.val);
  DashboardUI_SetMppt(sw_mppt.val);
  DashboardUI_SetMotor(sw_motor.val);
  DashboardUI_SetBattery((uint8_t)sw_battery.val);
  DashboardUI_SetBar((uint8_t)sw_bar.val);
  DashboardUI_SetFault((uint8_t)sw_fcode.val, simFCount);
  DashboardUI_SetPos(simPos);
  DashboardUI_SetRegen(simRegen);
  DashboardUI_SetTime(simHour, simMin);
}

void DashboardTest_Init(void)
{
  DashboardUI_Init();

  sweepTick = HAL_GetTick();
  slowTick  = HAL_GetTick();

  push_all();
}

void DashboardTest_Update(void)
{
  uint32_t now = HAL_GetTick();

  /* ---- 왕복 스윕 (200ms) ---- */
  if (now - sweepTick >= DASH_TEST_SWEEP_PERIOD_MS)
  {
    sweepTick = now;

    /* 억제 중이면 스윕도 진행시키지 않는다 -> CAN이 끊겼을 때 마지막 값에서 이어짐 */
    if (!s_suppress_speed) DashboardUI_SetSpeed((uint8_t)sweep_step(&sw_speed));
#if !DASH_TEST_SPEED_ONLY
    DashboardUI_SetRecommend((uint8_t)sweep_step(&sw_recommend));
    DashboardUI_SetDte((uint16_t)sweep_step(&sw_dte));
    DashboardUI_SetMppt(sweep_step(&sw_mppt));
    DashboardUI_SetMotor(sweep_step(&sw_motor));
    DashboardUI_SetBattery((uint8_t)sweep_step(&sw_battery));
    DashboardUI_SetBar((uint8_t)sweep_step(&sw_bar));
    DashboardUI_SetFault((uint8_t)sweep_step(&sw_fcode), simFCount);
#endif
  }

#if !DASH_TEST_SPEED_ONLY
  /* ---- 5초 주기: POS/REGEN 순환, 시간 +1분, FaultCount +1 ---- */
  if (now - slowTick >= DASH_TEST_SLOW_PERIOD_MS)
  {
    slowTick = now;

    simPos   = (DashPos_t)((simPos + 1) % DASH_POS_COUNT);
    simRegen = (DashRegen_t)((simRegen + 1) % DASH_REGEN_COUNT);
    DashboardUI_SetPos(simPos);
    DashboardUI_SetRegen(simRegen);

    if (++simMin >= 60)
    {
      simMin = 0;
      if (++simHour >= 24)
        simHour = 0;
    }
    DashboardUI_SetTime(simHour, simMin);

    if (simFCount < 100)
      simFCount++;
    DashboardUI_SetFault((uint8_t)sw_fcode.val, simFCount);
  }


  /* ---- 텔레메트리 아이콘: 500ms 점멸 ---- */
  static uint32_t teleTick = 0;
  static bool teleOn = true;
  if (now - teleTick >= 500U)
  {
    teleTick = now;
    teleOn = !teleOn;
    DashboardUI_ShowTele(teleOn);
  }
#endif
}
