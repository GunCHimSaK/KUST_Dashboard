/**
  ******************************************************************************
  * @file    dashboard_ui.c
  * @brief   계기판 UI 바인딩 계층 구현
  *
  *          위젯 타입 참고 (ui_Screen1.c 기준 - UI 재수출 시 반드시 재확인!):
  *            label    : SpeedVal, TimeVal, CruiseVal, MpptVal, MotorVal,
  *                       BatteryVal, FaultCodeVal, FaultCountVal,
  *                       RecommendTxt(<- 추천속도 '값'), PosVal, RegenVal, DteVal
  *            textarea : SpeedTxt, CruiseTxt, RecommendVal(<- 'km/h' 단위),
  *                       PosTitleTxt, RegenTitleTxt, DteTitleTxt
  *            img      : BatteryWarnImg, HazardImg, Left/RightTurnOn/OffImg,
  *                       CruiseImg, MpptImg, MotorImg, BatteryImg, TeleImg,
  *                       RecommendImg, KustLogoImg
  *            bar      : Bar1 (0~100)
  ******************************************************************************
  */
#include "dashboard_ui.h"
#include "main.h"       /* HAL_GetTick */
#include "ui.h"
#include <stdio.h>
#include <string.h>

/* ---- 값 캐시 (값이 바뀔 때만 LVGL 갱신) --------------------------------- */
#define CACHE_INVALID_U16  0xFFFFU

/* 전력은 음수(회생)를 표시하므로 부호 있는 캐시를 쓴다.
 * 무효값은 int32 최솟값 -- 실제 전력이 절대 가질 수 없는 값이라 안전하다.
 * (0xFFFF를 쓰면 65535W가 '무효'와 구분되지 않는 문제도 같이 사라진다) */
#define CACHE_INVALID_S32  INT32_MIN

static uint16_t c_speed     = CACHE_INVALID_U16;
static uint16_t c_recommend = CACHE_INVALID_U16;
static uint16_t c_cruise    = CACHE_INVALID_U16;
static uint16_t c_pos       = CACHE_INVALID_U16;
static uint16_t c_regen     = CACHE_INVALID_U16;
static uint16_t c_dte       = CACHE_INVALID_U16;
static int32_t  c_mppt      = CACHE_INVALID_S32;
static int32_t  c_motor     = CACHE_INVALID_S32;
static uint16_t c_battery   = CACHE_INVALID_U16;
static uint16_t c_bar       = CACHE_INVALID_U16;
static uint16_t c_time      = CACHE_INVALID_U16;   /* hour*60+min */
static uint16_t c_fcode     = CACHE_INVALID_U16;
static uint16_t c_fcount    = CACHE_INVALID_U16;

static const char *pos_str[DASH_POS_COUNT]     = { "DRIVE", "NEUTRAL", "REVERSE" };
static const char *regen_str[DASH_REGEN_COUNT] = { "OFF", "LOW", "MID", "HIGH" };

/* ---- 지상국 텍스트 메시지 (ui_Text 전광판) --------------------------------
 *
 * SquareLine의 ui_Text 라벨(로고 자리, 폭 150px, 우측 정렬)에 표시합니다.
 * 예전의 코드 생성 오버레이 배너는 제거했습니다.
 *
 * 표시 규칙
 *   - 영문(ASCII) 전용. 폰트 ui_font_fontspeedunit이 12px 고정폭이라
 *     150px / 12px = 12글자가 한 화면에 들어갑니다.
 *   - 12글자 이하  : 그대로 우측 정렬 표시
 *   - 12글자 초과  : 전광판처럼 한 글자씩 왼쪽으로 밀며 무한 순환
 *   - 표시 중에 새 메시지가 오면 중복 검사 없이 즉시 교체
 *   - ACC OFF에서 BT_2를 누르면 '확인' 처리되어 사라지고 로고 복귀
 *   - 자동 소거 없음. 확인하거나 새 메시지가 올 때까지 계속 표시
 *
 * ⚠ 한 글자 밀 때마다 라벨 텍스트가 바뀌므로 전체화면을 다시 그립니다
 *   (약 148ms). MSG_SCROLL_MS를 너무 줄이면 화면 갱신이 밀립니다. */

/* 한 화면에 들어가는 글자 수 = ui_Text 폭 150px / 폰트 고정폭 12px */
#define MSG_VISIBLE_CHARS   12U

/* 한 글자 이동 간격. 전체화면 리드로우가 148ms이므로 그보다 넉넉히 잡습니다. */
#ifndef MSG_SCROLL_MS
#define MSG_SCROLL_MS       300U
#endif

/* 순환할 때 문장 끝과 처음 사이에 넣는 공백 칸 수 */
#define MSG_GAP_CHARS       3U

/* 본문 최대 바이트 (CAN 0x500 본문 상한 62 + 널) */
#define MSG_CAPACITY        63U

static char     msg_text[MSG_CAPACITY] = {0};  /* ASCII로 정제된 원문 */
static uint16_t msg_len   = 0;                 /* 원문 길이 */
static uint16_t msg_pos   = 0;                 /* 스크롤 시작 오프셋 */
static uint32_t msg_tick  = 0;                 /* 마지막 이동 시각 */
static bool     msg_shown = false;             /* ui_Text 표시 중 */
static volatile bool msg_ack_req = false;      /* BT_2 확인 요청 (ISR에서 세움) */

static void msg_reset(void);
static void msg_render(void);

/* 숨김 상태가 실제로 바뀔 때만 플래그를 건드림 (불필요한 invalidate 방지) */
static void show_obj(lv_obj_t *obj, bool show)
{
  bool hidden = lv_obj_has_flag(obj, LV_OBJ_FLAG_HIDDEN);
  if (show && hidden)
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);
  else if (!show && !hidden)
    lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);
}

void DashboardUI_Init(void)
{
  /* 값 캐시 리셋 (재초기화 대비) */
  c_speed = c_recommend = c_cruise = c_pos = c_regen = CACHE_INVALID_U16;
  c_dte = c_battery = c_bar = CACHE_INVALID_U16;
  c_time = c_fcode = c_fcount = CACHE_INVALID_U16;
  c_mppt = c_motor = CACHE_INVALID_S32;

  /* ⚠ 전력 라벨(MpptVal/MotorVal)의 폭·위치는 SquareLine 설계값 그대로 둡니다.
   *   (67px, x = ±80. 예전에 음수 표시를 위해 84px/±72로 넓히던 코드는 제거)
   *
   *   ui_font_fontspeedunit이 12px 고정폭이라 67px = 5글자입니다.
   *   "-2250W"(6글자)는 들어가지 않아 줄바꿈됩니다. 4자리 회생값을 한 줄로
   *   보시려면 SquareLine에서 두 라벨 폭을 84px로 키워 재수출하세요. */

  /* 항상 표시 요소 */
  DashboardUI_ShowBatteryWarn(true);
  DashboardUI_ShowCruise(true);
  show_obj(ui_TeleImg, true);

  /* 메시지 라벨은 숨기고 로고를 띄운 상태로 시작 */
  msg_reset();
}

void DashboardUI_SetSpeed(uint8_t kmh)
{
  if (c_speed == kmh) return;
  c_speed = kmh;
  lv_label_set_text_fmt(ui_SpeedVal, "%d", kmh);
}

void DashboardUI_SetRecommend(uint8_t kmh)
{
  if (c_recommend == kmh) return;
  c_recommend = kmh;
  /* 주의: RecommendTxt가 값 label (RecommendVal은 'km/h' 단위 textarea) */
  lv_label_set_text_fmt(ui_RecommendTxt, "%d", kmh);
}

void DashboardUI_SetCruise(uint8_t kmh)
{
  if (c_cruise == kmh) return;
  c_cruise = kmh;
  lv_label_set_text_fmt(ui_CruiseVal, "%d", kmh);
}

void DashboardUI_SetPos(DashPos_t pos)
{
  if (pos >= DASH_POS_COUNT || c_pos == (uint16_t)pos) return;
  c_pos = (uint16_t)pos;
  lv_label_set_text(ui_PosVal, pos_str[pos]);
}

void DashboardUI_SetRegen(DashRegen_t regen)
{
  if (regen >= DASH_REGEN_COUNT || c_regen == (uint16_t)regen) return;
  c_regen = (uint16_t)regen;
  lv_label_set_text(ui_RegenVal, regen_str[regen]);
}

void DashboardUI_SetDte(uint16_t km)
{
  if (c_dte == km) return;
  c_dte = km;
  lv_label_set_text_fmt(ui_DteVal, "%dkm", km);
}

void DashboardUI_SetMppt(int32_t watt)
{
  if (c_mppt == watt) return;
  c_mppt = watt;
  lv_label_set_text_fmt(ui_MpptVal, "%dW", (int) watt);
}

void DashboardUI_SetMotor(int32_t watt)
{
  if (c_motor == watt) return;
  c_motor = watt;
  lv_label_set_text_fmt(ui_MotorVal, "%dW", (int) watt);
}

void DashboardUI_SetBattery(uint8_t percent)
{
  if (c_battery == percent) return;
  c_battery = percent;
  lv_label_set_text_fmt(ui_BatteryVal, "%d%%", percent);
}

void DashboardUI_SetBar(uint8_t percent)
{
  if (c_bar == percent) return;
  c_bar = percent;
  lv_bar_set_value(ui_Bar1, percent, LV_ANIM_OFF);
}

void DashboardUI_SetTime(uint8_t hour, uint8_t min)
{
  uint16_t t = (uint16_t)hour * 60U + min;
  if (c_time == t) return;
  c_time = t;
  lv_label_set_text_fmt(ui_TimeVal, "%02d:%02d", hour, min);
}

void DashboardUI_SetFault(uint8_t code, uint8_t count)
{
  if (c_fcode != code)
  {
    c_fcode = code;
    lv_label_set_text_fmt(ui_FaultCodeVal, "%d", code);
  }
  if (c_fcount != count)
  {
    c_fcount = count;
    lv_label_set_text_fmt(ui_FaultCountVal, "+%d", count);
  }
}

void DashboardUI_ShowBatteryWarn(bool show)
{
  show_obj(ui_BatteryWarnImg, show);
}

void DashboardUI_ShowCruise(bool show)
{
  show_obj(ui_CruiseImg, show);
  show_obj(ui_CruiseTxt, show);
  show_obj(ui_CruiseVal, show);
}

void DashboardUI_ShowTele(bool show)
{
  show_obj(ui_TeleImg, show);
}

void DashboardUI_UpdateLights(bool left, bool right, bool hazard)
{
  static uint32_t lastBlinkTick = 0;
  static bool isVisible = false;

  bool anyOn = left || right || hazard;

  if (anyOn)
  {
    if (HAL_GetTick() - lastBlinkTick >= 500U)
    {
      isVisible = !isVisible;
      lastBlinkTick = HAL_GetTick();
    }
  }
  else
  {
    /* 다음에 켤 때 바로 점등 상태로 시작 */
    isVisible = true;
    lastBlinkTick = HAL_GetTick();
  }

  bool showLeft   = anyOn && isVisible && (hazard || left);
  bool showRight  = anyOn && isVisible && (hazard || right);
  bool showHazard = anyOn && isVisible && hazard;

  show_obj(ui_LeftTurnOnImg,  showLeft);
  show_obj(ui_RightTurnOnImg, showRight);
  show_obj(ui_HazardImg,      showHazard);
}

/* ---- 지상국 텍스트 메시지 배너 ------------------------------------------ */
/**
  * @brief  표시 가능한 ASCII만 남기고 나머지는 '?' 하나로 접는다
  * @note   폰트가 ASCII 전용이라 비ASCII를 그대로 넘기면
  *         LV_USE_FONT_PLACEHOLDER=1 때문에 네모(□)가 늘어섭니다.
  *         한글 한 글자는 3바이트라 바이트마다 '?'를 찍으면 "???"가 되므로,
  *         연속된 비ASCII 구간은 '?' 하나로 합칩니다.
  * @param  dst      출력 버퍼
  * @param  dst_size 출력 버퍼 크기 (널 포함, 1 이상)
  * @param  src      입력 문자열 (널 종단)
  */
static void ascii_sanitize(char *dst, size_t dst_size, const char *src)
{
  size_t o    = 0U;
  bool   held = false;   /* 직전 바이트가 비ASCII였는가 */

  while ((*src != '\0') && (o < (dst_size - 1U)))
  {
    uint8_t c = (uint8_t) *src++;

    if ((c >= 0x20U) && (c <= 0x7EU))
    {
      dst[o++] = (char) c;
      held = false;
    }
    else if (!held)
    {
      dst[o++] = '?';    /* 비ASCII 구간 시작 -> 물음표 하나 */
      held = true;
    }
    else
    {
      /* 비ASCII 구간 계속 -> 버림 */
    }
  }

  dst[o] = '\0';
}

/**
  * @brief  메시지를 지우고 로고를 다시 띄운다 (표시 상태 초기화)
  */
static void msg_reset(void)
{
  msg_text[0] = '\0';
  msg_len     = 0U;
  msg_pos     = 0U;
  msg_shown   = false;

  show_obj(ui_Text, false);
  show_obj(ui_KustLogoImg, true);
}

/**
  * @brief  현재 스크롤 위치에 해당하는 12글자 창을 ui_Text에 그린다
  * @note   12글자 이하면 원문을 그대로 넣습니다. 라벨이 우측 정렬이라
  *         짧은 문장은 오른쪽 끝에 붙습니다.
  *
  *         12글자를 넘으면 '원문 + 공백 MSG_GAP_CHARS개'를 한 바퀴로 보고
  *         msg_pos부터 12글자를 잘라 냅니다. 공백 구간이 있어야 마지막
  *         글자와 첫 글자가 붙어 보이지 않습니다.
  */
static void msg_render(void)
{
  char     win[MSG_VISIBLE_CHARS + 1U];
  uint16_t cycle;
  uint16_t i;

  if (msg_len <= MSG_VISIBLE_CHARS)
  {
    lv_label_set_text(ui_Text, msg_text);
    return;
  }

  cycle = (uint16_t) (msg_len + MSG_GAP_CHARS);

  for (i = 0U; i < MSG_VISIBLE_CHARS; i++)
  {
    uint16_t k = (uint16_t) ((msg_pos + i) % cycle);
    win[i] = (k < msg_len) ? msg_text[k] : ' ';
  }
  win[MSG_VISIBLE_CHARS] = '\0';

  lv_label_set_text(ui_Text, win);
}

void DashboardUI_ShowMessage(const char *utf8)
{
  /* 본문이 비어 있는 메시지 = 지우라는 뜻 */
  if ((utf8 == NULL) || (utf8[0] == '\0'))
  {
    DashboardUI_ClearMessage();
    return;
  }

  /* 폰트가 ASCII 전용이므로 여기서 걸러낸다 (인자 문자열은 건드리지 않음).
   * 이미 표시 중이어도 중복 검사 없이 그대로 새 메시지로 교체한다. */
  ascii_sanitize(msg_text, sizeof(msg_text), utf8);
  msg_len = (uint16_t) strlen(msg_text);

  if (msg_len == 0U)
  {
    DashboardUI_ClearMessage();       /* 표시할 게 하나도 안 남았다 */
    return;
  }

  msg_pos   = 0U;                     /* 새 메시지는 항상 처음부터 */
  msg_tick  = HAL_GetTick();
  msg_shown = true;

  /* 로고를 감추고 텍스트를 띄운다 (둘은 같은 자리라 동시에 못 보인다) */
  show_obj(ui_KustLogoImg, false);
  show_obj(ui_Text, true);
  msg_render();
}

void DashboardUI_ClearMessage(void)
{
  msg_reset();
}

void DashboardUI_AckMessage(void)
{
  /* ISR에서 불려도 안전하도록 플래그만 세운다.
   * 실제 처리는 메인 루프의 DashboardUI_UpdateMessage()에서 한다
   * (LVGL을 ISR에서 건드리면 안 됨). */
  msg_ack_req = true;
}

void DashboardUI_UpdateMessage(void)
{
  /* BT_2 확인 -> 메시지를 지우고 로고 복귀 */
  if (msg_ack_req)
  {
    msg_ack_req = false;
    if (msg_shown)
    {
      msg_reset();
    }
    return;
  }

  if (!msg_shown || (msg_len <= MSG_VISIBLE_CHARS))
  {
    return;   /* 표시 중이 아니거나 한 화면에 다 들어가면 움직일 필요 없음 */
  }

  if ((HAL_GetTick() - msg_tick) < MSG_SCROLL_MS)
  {
    return;
  }
  msg_tick = HAL_GetTick();

  msg_pos = (uint16_t) ((msg_pos + 1U) % (msg_len + MSG_GAP_CHARS));
  msg_render();
}
