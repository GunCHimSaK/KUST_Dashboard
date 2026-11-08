/**
  ******************************************************************************
  * @file    dashboard_ui.h
  * @brief   계기판 UI 바인딩 계층 (SquareLine 위젯 <-> 애플리케이션 값)
  *
  *          이 모듈이 UI 갱신의 유일한 통로입니다. 실전(CAN 연동)에서는
  *          CAN 수신 처리부가 이 Set 함수들을 그대로 호출하면 됩니다.
  *
  *          내부 규칙:
  *           - 위젯 타입에 맞는 API만 사용 (label vs textarea 구분.
  *             textarea에 label API를 쓰면 메모리가 깨져 HardFault 발생!)
  *           - 모든 Set 함수는 값을 캐싱하여, 값이 실제로 바뀐 경우에만
  *             LVGL을 건드림 (불필요한 전체화면 리드로우 방지)
  ******************************************************************************
  */
#ifndef __DASHBOARD_UI_H
#define __DASHBOARD_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* 기어 상태 (ui_PosVal에 표시) */
typedef enum
{
  DASH_POS_DRIVE = 0,
  DASH_POS_NEUTRAL,
  DASH_POS_REVERSE,
  DASH_POS_COUNT            /* 항목 수 (순환용) */
} DashPos_t;

/* 회생제동 단계 (ui_RegenVal에 표시) */
typedef enum
{
  DASH_REGEN_OFF = 0,
  DASH_REGEN_LOW,
  DASH_REGEN_MID,
  DASH_REGEN_HIGH,
  DASH_REGEN_COUNT          /* 항목 수 (순환용) */
} DashRegen_t;

/* ---- 초기화 -------------------------------------------------------------- */
/* ui_init() 이후 1회 호출. 항상 표시할 요소(배터리 경고, 크루즈, 텔레메트리)를
 * 보이게 하고 값 캐시를 초기화한다. */
void DashboardUI_Init(void);

/* ---- 주행 값 ------------------------------------------------------------- */
void DashboardUI_SetSpeed(uint8_t kmh);           /* ui_SpeedVal   "%d"     */
void DashboardUI_SetRecommend(uint8_t kmh);       /* ui_RecommendTxt "%d"
                                                     (주의: Val이 아니라 Txt가
                                                      값 label, Val은 단위)   */
void DashboardUI_SetCruise(uint8_t kmh);          /* ui_CruiseVal  "%d"     */
void DashboardUI_SetPos(DashPos_t pos);           /* ui_PosVal   DRIVE/...  */
void DashboardUI_SetRegen(DashRegen_t regen);     /* ui_RegenVal OFF/...    */
void DashboardUI_SetDte(uint16_t km);             /* ui_DteVal    "%dkm"    */

/* ---- 전력/배터리 --------------------------------------------------------- */
/* 전력 표시. 음수 = 회생(배터리로 되돌아가는 전력)이며 "-1234W"로 그립니다.
 *
 * 표시 폭: 폰트(ui_font_fontspeedunit)가 문자당 12px 고정폭이고, 두 라벨을
 * DashboardUI_Init()에서 84px로 넓혀 두었으므로 부호 포함 7글자까지 들어갑니다.
 * -> int16 전 범위("-32768W" 7글자)가 줄바꿈 없이 표시됩니다. */
void DashboardUI_SetMppt(int32_t watt);           /* ui_MpptVal   "%dW"     */
void DashboardUI_SetMotor(int32_t watt);          /* ui_MotorVal  "%dW"     */
void DashboardUI_SetBattery(uint8_t percent);     /* ui_BatteryVal "%d%%"   */
void DashboardUI_SetBar(uint8_t percent);         /* ui_Bar1 0~100          */

/* ---- 상태/정보 ----------------------------------------------------------- */
void DashboardUI_SetTime(uint8_t hour, uint8_t min);       /* "%02d:%02d"   */
void DashboardUI_SetFault(uint8_t code, uint8_t count);    /* "%d" / "+%d"  */
void DashboardUI_ShowBatteryWarn(bool show);      /* ui_BatteryWarnImg      */
void DashboardUI_ShowCruise(bool show);           /* 크루즈 Img+Txt+Val 묶음 */
void DashboardUI_ShowTele(bool show);             /* ui_TeleImg (텔레메트리) */

/* ---- 방향지시등/비상등 ----------------------------------------------------
 * 메인 루프에서 매 회 호출. 내부에서 500ms 주기로 깜빡임을 처리한다.
 * hazard가 true면 left/right 인자와 무관하게 양쪽 + 비상등 아이콘 점멸. */
void DashboardUI_UpdateLights(bool left, bool right, bool hazard);

/* ---- 지상국 텍스트 메시지 (CAN 0x500) -------------------------------------
 * 화면 위(lv_layer_top)에 뜨는 오버레이 배너입니다. SquareLine 프로젝트에는
 * 없는 위젯이라 코드에서 만들며, UI를 재수출해도 지워지지 않습니다.
 *
 * SquareLine의 ui_Text 라벨(로고 자리, 폭 150px, 우측 정렬)에 표시합니다.
 * ui_Text와 ui_KustLogoImg는 같은 자리라 동시에 보이지 않습니다.
 * 메시지가 뜨면 로고가 숨고, 메시지를 지우면 로고가 돌아옵니다.
 *
 * 표시 규칙
 *   - 영문(ASCII) 전용. 폰트가 12px 고정폭이라 한 화면에 12글자.
 *     프로젝트 폰트는 전부 ASCII(0x20~0x7F)뿐이고 한글 폰트 계획은 없습니다.
 *     지상국에는 ASCII로 보내달라고 요청하세요.
 *   - 12글자 이하 : 우측 정렬로 그대로 표시
 *   - 12글자 초과 : 전광판처럼 한 글자씩 밀며 무한 순환 (기본 300ms/글자)
 *   - 표시 중 새 메시지가 오면 중복 검사 없이 즉시 교체하고 처음부터 다시
 *   - 자동 소거 없음. 확인하거나 새 메시지가 올 때까지 계속 표시
 *
 * ⚠ 한 글자 밀 때마다 전체화면을 다시 그립니다(약 148ms). 스크롤 간격은
 *   dashboard_ui.c의 MSG_SCROLL_MS로 조정하세요. */

/* 메시지 표시 (널 종단). NULL이거나 빈 문자열이면 지우고 로고로 돌아갑니다.
 *
 * 표시 가능한 ASCII(0x20~0x7E)만 남기고, 연속된 비ASCII 구간은 '?' 하나로
 * 접어서 그립니다. 걸러낸 뒤 남는 게 없으면 지웁니다.
 * 인자 문자열 자체는 수정하지 않습니다. */
void DashboardUI_ShowMessage(const char *utf8);

/* 메시지 즉시 제거 + 로고 복귀 */
void DashboardUI_ClearMessage(void);

/* 사용자 '확인' 요청 (ACC OFF 상태에서 BT_2).
 * ISR에서 호출해도 안전합니다 -- 플래그만 세우고 실제 처리는
 * DashboardUI_UpdateMessage()가 메인 루프에서 합니다. */
void DashboardUI_AckMessage(void);

/* 메인 루프에서 매 회 호출. 전광판 스크롤과 확인 요청을 처리합니다. */
void DashboardUI_UpdateMessage(void);

#ifdef __cplusplus
}
#endif

#endif /* __DASHBOARD_UI_H */
