/**
  ******************************************************************************
  * @file    can_text.h
  * @brief   CAN 텍스트 메시지 수신/조립 (Bus B, ID 0x500)
  ******************************************************************************
  * Bus B 사양
  *   차량 CAN과 전기적으로 절연된 별도 버스. 통신보드와 이 계기판 둘만 연결.
  *   500 kbit/s, Classic CAN 2.0, 표준 11bit ID, 양단 120Ω 종단.
  *
  * 프레임 형식 (통신보드 -> 계기판, ID 0x500)
  *   DLC     : 항상 8  (본문이 짧아도 남는 자리는 0x00으로 채워 옴)
  *   byte 0  : bit7   = 마지막 프레임이면 1 (0x80)
  *             bit6~0 = 시퀀스 번호 (0부터 1씩 증가)
  *   byte 1~7: 본문 최대 7바이트, UTF-8, 널 종단 없음
  *
  *   본문 최대 62바이트 -> 최대 9프레임 (seq 0~8).
  *
  * ⚠ 표시는 영문(ASCII) 전용입니다. 계기판에 한글 폰트가 없어서
  *   비ASCII 바이트는 DashboardUI_ShowMessage()가 '?'로 접어 버립니다.
  *   지상국에는 ASCII로 보내달라고 요청하세요.
  *   조립기(이 모듈)는 원문 바이트를 그대로 보존합니다 -- 아래 UTF-8 절단
  *   보정은 g_dbg.txt_last에 깨진 바이트가 남지 않게 하는 방어용입니다.
  *
  * ⚠ 마지막 프레임의 '유효 바이트 수'는 전달되지 않습니다.
  *   DLC가 항상 8이라 남는 자리가 0x00 패딩으로 오므로, 수신측이 뒤쪽 0x00을
  *   직접 잘라내야 합니다. UTF-8 본문에는 0x00이 나오지 않는다는 (규격에 적히지
  *   않은) 암묵적 가정에 의존합니다. 이 처리를 빼면 화면 끝에 깨진 문자가 붙습니다.
  *
  * 동작
  *   CanBus_Process() -> CanBus_OnFrame() -> CanText_OnFrame() 경로로 조립되고,
  *   메인 루프의 CanText_Poll()이 완성된 메시지를 꺼내 갑니다.
  *   조립/완성 모두 메인 루프 컨텍스트라 LVGL 호출과 충돌하지 않습니다.
  *
  * 사용법
  *   1) CanBus_Init() 뒤에 CanText_Init() 한 번
  *   2) 메인 루프에서
  *        char msg[CAN_TEXT_MAX_BYTES + 1];
  *        if (CanText_Poll(msg, sizeof(msg))) DashboardUI_ShowMessage(msg);
  ******************************************************************************
  */

#ifndef CAN_TEXT_H_
#define CAN_TEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "can_bus.h"

/* 통신보드 -> 계기판 텍스트 메시지 ID */
#define CAN_TEXT_RX_ID            0x500U

/* 본문 최대 길이 (바이트).
 * ⚠ 지상국 MAX_MESSAGE_BYTES / 통신보드 UART_RX_TEXT_MSG_MAX_BYTES와
 *   반드시 같은 값이어야 합니다. 한쪽만 줄이면 긴 메시지가 조용히 사라집니다. */
#define CAN_TEXT_MAX_BYTES        62U

/* 최대 프레임 수 = seq 0~8 */
#define CAN_TEXT_MAX_FRAMES       9U

/* 프레임이 실어 나를 수 있는 최대 '원시' 바이트 수 (0x00 패딩 포함).
 * 9프레임 x 7바이트 = 63으로, 본문 상한 62보다 1 큽니다.
 * 본문이 정확히 62바이트일 때 마지막 프레임이 6바이트 + 패딩 1바이트를
 * 싣고 오기 때문에, 조립 버퍼는 62가 아니라 63이어야 합니다. */
#define CAN_TEXT_RAW_CAPACITY     (CAN_TEXT_MAX_FRAMES * 7U)

/* 조립 타임아웃. 첫 프레임 이후 이 시간 안에 마지막 프레임이 안 오면 버립니다.
 * 규격상 프레임당 약 250us 연속 송신이므로 9프레임도 3ms면 끝납니다. */
#define CAN_TEXT_TIMEOUT_MS       500U

/* 수신 통계 (진단용) */
typedef struct
{
	uint32_t frame_count;    /* 수신한 0x500 프레임 누적 */
	uint32_t msg_count;      /* 완성된 메시지 누적 */
	uint32_t drop_seq;       /* 시퀀스가 어긋나 버린 조립 */
	uint32_t drop_timeout;   /* 타임아웃으로 버린 조립 */
	uint32_t drop_overflow;  /* 62바이트/9프레임을 넘겨 버린 조립 */
	uint32_t trim_utf8;      /* UTF-8이 깨져 뒤를 잘라낸 메시지 */
} CanTextStats_t;

/** 조립 상태 초기화. CanBus_Init() 뒤에 1회 호출. */
void CanText_Init(void);

/** 0x500 프레임 1개를 조립기에 투입.
  * CanBus_OnFrame()에서 호출되며, 다른 ID는 무시합니다. */
void CanText_OnFrame(const CanFrame_t *frame);

/** 메인 루프에서 주기 호출.
  * 조립 타임아웃을 감시하고, 새로 완성된 메시지가 있으면 out에 담아 반환합니다.
  * @param out       널 종단 UTF-8 문자열이 담길 버퍼
  * @param out_size  out 크기. CAN_TEXT_MAX_BYTES + 1 이상 권장
  * @retval true = 새 메시지를 꺼냈음 (같은 메시지를 두 번 주지 않습니다)
  * @note  본문이 비어 있는 메시지(모두 0x00)도 true + 빈 문자열로 전달됩니다.
  *        표시를 지우라는 뜻으로 쓰면 됩니다. */
bool CanText_Poll(char *out, uint16_t out_size);

/** 통계 스냅샷 */
void CanText_GetStats(CanTextStats_t *out);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TEXT_H_ */
