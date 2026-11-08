/**
  ******************************************************************************
  * @file    debug_log.h
  * @brief   디버그 출력 / 진단 스냅샷
  ******************************************************************************
  * 두 가지 경로를 함께 제공합니다. 배선 상황에 맞는 쪽을 쓰면 됩니다.
  *
  * (A) g_dbg 전역 스냅샷  -- 추가 핀 불필요, 지금 배선 그대로 동작
  *     PA13/PA14 2선 SWD만 있으면 됩니다. STM32CubeIDE에서
  *       Window > Show View > Live Expressions 에 `g_dbg` 를 추가하면
  *       보드를 멈추지 않고 값이 갱신되는 걸 볼 수 있습니다.
  *     ⚠ Live Expressions는 실행 중 메모리를 읽으므로 구조체가 통째로
  *       원자적이지는 않습니다. 한 필드씩 보는 용도입니다.
  *
  * (B) ITM/SWO 텍스트 로그 -- PB3(TRACESWO) 1핀 추가 필요
  *     CubeMX에서 SYS > Debug 를 "Serial Wire"가 아니라
  *     "Trace Asynchronous Sw" 로 바꾸면 PB3가 TRACESWO로 잡힙니다.
  *     (PB3는 현재 미사용이라 충돌 없음)
  *     그다음 디버그 설정에서
  *       Debug Configurations > Debugger > Serial Wire Viewer(SWV) 체크
  *       Core Clock = 160.0 MHz  (main.c의 SYSCLK와 반드시 일치)
  *     디버깅을 시작하고 Window > Show View > SWV > SWV ITM Data Console
  *     에서 Port 0을 켜고 Start Trace 를 누르면 로그가 흐릅니다.
  *
  *     SWO를 안 켜거나 디버거를 안 붙이면 ITM 출력은 조용히 버려집니다.
  *     (CMSIS ITM_SendChar가 ITM 활성 여부를 먼저 확인)
  *
  * DBG_ENABLE 을 0으로 두면 로그 문자열과 코드가 전부 컴파일에서 빠집니다.
  * g_dbg 스냅샷은 그때도 남습니다(크기가 작고 Live Expressions 전용이라).
  ******************************************************************************
  */

#ifndef DEBUG_LOG_H_
#define DEBUG_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* 1 = 로그 활성. 릴리스에서 0으로 두면 코드/문자열이 사라집니다. */
#ifndef DBG_ENABLE
#define DBG_ENABLE            1
#endif

/* 주기적 진단 덤프 간격 */
#ifndef DBG_DUMP_PERIOD_MS
#define DBG_DUMP_PERIOD_MS    5000U
#endif

/* ---- Live Expressions용 스냅샷 ------------------------------------------
 * 디버거에서 `g_dbg` 하나만 걸어 두면 아래 값이 전부 보입니다. */
typedef struct
{
	uint32_t uptime_ms;        /* 부팅 후 경과 시간 */
	uint32_t loop_count;       /* 메인 루프 반복 횟수 */

	/* 메인 루프 주기 (직전 덤프 구간 기준, ms).
	 * 전체화면 flush가 도는 사이클은 40ms 안팎이 정상입니다. */
	uint16_t loop_ms_min;
	uint16_t loop_ms_max;
	uint16_t loop_ms_avg;

	/* CAN 전송 계층 (can_bus.c) */
	uint32_t can_rx;           /* 수신 프레임 누적 */
	uint32_t can_tx;           /* 송신 누적 */
	uint32_t can_rx_overflow;  /* 링버퍼 넘침 */
	uint32_t can_err;          /* 프로토콜 에러 */
	uint8_t  can_bus_off;      /* 1 = Bus-Off 상태 */

	/* CAN 텍스트 계층 (can_text.c) */
	uint32_t txt_frames;       /* 0x500 프레임 누적 */
	uint32_t txt_msgs;         /* 완성된 메시지 누적 */
	uint32_t txt_drop_seq;     /* 시퀀스 어긋남 */
	uint32_t txt_drop_timeout; /* 조립 타임아웃 */
	uint32_t txt_drop_overflow;/* 길이 초과 */
	uint32_t txt_trim_utf8;    /* UTF-8 절단 보정 */

	/* 마지막으로 수신한 메시지의 '원문' 바이트.
	 * 화면 표시는 ASCII만 남기고 나머지를 '?'로 접지만, 여기에는 걸러내기 전
	 * 바이트가 그대로 들어옵니다. 지상국이 규격을 어기고 비ASCII를 보냈는지
	 * 확인할 때 이 값을 보세요 (길이는 txt_last_len). */
	char     txt_last[64];
	uint8_t  txt_last_len;

	/* LVGL 힙 (LV_MEM_SIZE = 96KB).
	 * used_pct가 계속 오르면 어딘가에서 객체를 안 지우고 있는 것입니다. */
	uint8_t  lv_used_pct;
	uint8_t  lv_frag_pct;
	uint32_t lv_free_biggest;

	/* 전류 모니터 (LM76202-Q1 IMON, PB1/ADC1_IN16).
	 * ⚠ ADC1이 CubeMX 프로젝트에 없으면 imon.c가 스텁이라 전부 0/false입니다.
	 * ⚠ imon_valid는 부하가 50mA 이상일 때만 true입니다. 그 밑은 IMON 칩의
	 *   오프셋 전류(2uA)가 지배적이라 데이터시트상 선형성이 없습니다. */
	uint16_t imon_raw;         /* ADC raw (필터 후, 0~16383) */
	uint16_t imon_mv;          /* IMON 전압 [mV] */
	uint16_t imon_ma;          /* 환산 부하 전류 [mA], 1A = 1565.6mV */
	uint8_t  imon_valid;       /* 1 = 50mA 이상이라 값이 유효 */

	/* 차량 텔레메트리 (CAN 0x501, 200ms 주기).
	 * tlm_valid=0 이면 0x501이 끊겨 표시 값이 0으로 내려간 상태입니다. */
	uint16_t tlm_speed_cms;    /* 0.01 m/s 원본 */
	int16_t  tlm_current_da;   /* 0.1 A 원본, 음수 = 회생 */
	uint16_t tlm_soc_permille; /* 1/1000 원본 */
	int16_t  tlm_power_w;      /* 1 W 원본, 음수 = 회생 */
	uint16_t tlm_speed_kmh;    /* 환산 km/h */
	uint8_t  tlm_soc_pct;      /* 환산 % */
	uint8_t  tlm_valid;        /* 1 = 0x501 살아있음 */
	uint32_t tlm_rx_count;     /* 0x501 수신 누적 */
	uint32_t tlm_drop_dlc;     /* DLC 8 미만이라 버린 수 */

	/* lv_timer_handler() 단독 소요 시간 [ms] (직전 덤프 구간).
	 * 루프 전체(loop_ms_*)에서 이 값을 빼면 나머지 로직 비용이 나옵니다. */
	uint16_t lv_ms_min;
	uint16_t lv_ms_max;
	uint16_t lv_ms_avg;
	uint32_t dma_timeouts;     /* SHARP_MIP DMA 완료 인터럽트 유실 수 */

	/* 0x501 수신 품질 (상대 송신 규칙성 판정) */
	uint16_t rx_dt_min;      /* 프레임 간격 최소 [ms] */
	uint16_t rx_dt_max;      /* 최대 [ms] */
	uint16_t rx_dt_avg;      /* 평균 [ms], 규격은 200 */
	uint16_t rx_step_max;    /* 연속 프레임 간 차속 변화 최대 [0.01 m/s] */

	/* 마지막 0x501 원본 8바이트 + DLC (보드 팀 대조용) */
	uint8_t  tlm_raw[8];
	uint8_t  tlm_raw_dlc;
	/* 0x510 명령 송신 통계.
	 * ⚠ ok는 'Tx FIFO 적재 성공'이지 '버스에서 ACK 받음'이 아닙니다.
	 *   실제 전달 여부는 FDCAN TEC / PSR.LEC로 판단하세요. */
	uint32_t cmd_tx_ok;
	uint32_t cmd_tx_fail;
} DbgSnapshot_t;

extern volatile DbgSnapshot_t g_dbg;

/* ---- 디버거로 조작하는 시험 스위치 (버튼 없이 사용) --------------------
 * STM32CubeIDE의 Live Expressions에 g_dbg_ctl 을 추가하고 값을 써 넣으면
 * 됩니다. 코어를 멈추지 않고 쓰기가 되므로 실행 중에 켜고 끌 수 있습니다.
 *
 *   g_dbg_ctl.tlm_sim  = 1  -> 0x501 텔레메트리 프레임을 200ms마다 합성해
 *                             수신 경로(CanApp_OnTelemetry)에 주입.
 *                             차속이 0 <-> 90 km/h를 60초 주기로 왕복합니다
 *                             (규격서의 더미 노드와 동일한 거동).
 *                          0  -> 중지. 600ms 뒤 tlm_valid가 0이 되며
 *                             화면은 시뮬레이터로 자동 복귀합니다.
 *
 *   g_dbg_ctl.text_sim = 1  -> 0x500 텍스트 메시지 1회 주입 (자동으로 0 복귀)
 *
 *   g_dbg_ctl.frame_sim = 1 -> 보드 팀이 캡처한 '회생 최대' 실측 0x501 프레임
 *                             3A 07 41 FF 18 03 36 F7 을 10초 동안 200ms마다
 *                             주입 (자동으로 0 복귀). ACC OFF 상태의 BT_0과 동일합니다.
 *                             화면 기대값: 66 km/h / 79% / -2250W
 *                             음수 전력 표시가 실제로 되는지 확인하는 용도.
 *
 * 실제 CAN 배선이 끝나면 tlm_sim은 0으로 두세요. 실물 0x501과 합성 프레임이
 * 동시에 들어가면 화면이 두 값 사이에서 튑니다.
 * (frame_sim이 도는 동안에는 tlm_sim 주입이 자동으로 멈춥니다) */
typedef struct
{
	uint8_t tlm_sim;
	uint8_t text_sim;
	uint8_t frame_sim;
} DbgControl_t;

extern volatile DbgControl_t g_dbg_ctl;

/* ---- API ---------------------------------------------------------------- */

/** ITM 준비 + 배너 출력. main()에서 다른 모듈 Init 뒤에 1회. */
void DBG_Init(void);

/** 메인 루프에서 매 회 호출.
  * 루프 주기를 재고, DBG_DUMP_PERIOD_MS마다 g_dbg 갱신 + 로그를 뿌립니다.
  * BT_3 셀프테스트 요청도 여기서 처리합니다(ISR이 아닌 메인 루프에서 실행). */
void DBG_Process(void);

/** 서식 있는 로그 1줄 (자동으로 [타임스탬프] 접두 + 개행).
  * ⚠ 내부에 정적 버퍼를 쓰므로 ISR에서 호출하지 마세요. */
void DBG_Printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

/** 화면에 띄운 메시지를 스냅샷에 기록 (main.c에서 호출) */
void DBG_NoteMessage(const char *utf8);

/** lv_timer_handler() 소요 시간 1회분 보고 (메인 루프에서 호출) */
void DBG_NoteLvglMs(uint32_t ms);

/** BT_3 셀프테스트 요청 (0x500 텍스트). ISR에서 호출해도 안전(플래그만 세움). */
void DBG_RequestSelfTest(void);

/** 캡처한 실측 0x501 '회생 최대' 프레임을 10초간 재생 요청.
  * 트리거: ACC OFF 상태에서 BT_0, 또는 g_dbg_ctl.frame_sim = 1
  * ISR에서 호출해도 안전(플래그만 세움). 기대 표시: 66 km/h / 79% / -2250W */
void DBG_RequestCapturedFrame(void);

#if DBG_ENABLE
  #define DBG_LOG(...)   DBG_Printf(__VA_ARGS__)
#else
  #define DBG_LOG(...)   ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_LOG_H_ */
