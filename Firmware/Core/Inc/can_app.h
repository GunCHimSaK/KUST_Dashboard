/**
  ******************************************************************************
  * @file    can_app.h
  * @brief   차량 텔레메트리 수신(0x501) / 명령 송신(0x510) + 프레임 디스패처
  ******************************************************************************
  * 버스 : Classic CAN 2.0, 500 kbps, 표준 11bit ID, 종단 120Ω x 2
  *
  * ── 보드 -> 대시보드 : ID 0x501, 200 ms 주기, DLC 8 ──────────────────────
  *   byte 0~1  차속       u16  0.01 m/s   (1250 = 12.50 m/s)
  *   byte 2~3  버스 전류  i16  0.1 A      (-85 = -8.5 A, 음수 = 회생)
  *   byte 4~5  SOC        u16  1/1000     (875 = 87.5 %)
  *   byte 6~7  모터 전력  i16  1 W        (3400 = 3.4 kW, 음수 = 회생)
  *   전부 리틀엔디안(하위 바이트 먼저).
  *
  *   ※ 초판 규격의 byte 6~7은 '모터 온도'였으나 수정판에서 '모터 전력'으로
  *     바뀌었습니다. 이 구현은 수정판 기준입니다.
  *
  * ── 대시보드 -> 보드 : ID 0x510 (ACC 명령), 200 ms 주기, DLC 8 ──────────
  *   byte 0    ACC 설정속도  u8  1 km/h   (0~150)
  *   byte 1    ACC 상태      u8  0 = OFF, 1 = ON
  *   byte 2~7  미사용 (0 채움)
  *
  *   최소 간격 100 ms — 더 빨리 보내면 보드가 초과분을 버립니다.
  *   보드가 0x420으로 변환해 차량 버스(VCU/모터)로 중계하며,
  *   ⚠ 중계 시 '앞 2바이트만' 전달됩니다. 새 필드를 byte 2 이후에 넣으면
  *     모터 팀에 도달하지 않습니다. 필드를 늘리려면 보드 팀과 중계 폭을
  *     먼저 협의하세요.
  *
  *   설정속도는 ACC가 꺼져 있어도 계속 실립니다(대기 중인 설정값).
  *   수신측은 byte 1이 0이면 byte 0을 무시하면 됩니다.
  *
  * ⚠ 0x501의 필드 구성은 배관 검증용 임시값입니다.
  *   표시 항목·주기가 확정되면 이 파일의 디코드만 고치면 됩니다.
  ******************************************************************************
  */

#ifndef CAN_APP_H_
#define CAN_APP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "can_bus.h"

#define CAN_APP_TELEMETRY_ID     0x501U   /* 보드 -> 대시보드 */
#define CAN_APP_COMMAND_ID       0x510U   /* 대시보드 -> 보드 */

/* 0x510 최소 송신 간격은 100 ms. 여유를 둬 200 ms로 보냅니다.
 * 이보다 빨리 보내면 보드가 초과분을 조용히 버립니다. */
#define CAN_APP_TX_PERIOD_MS     200U

/* 0x501이 200 ms 주기이므로 3회 연속 누락이면 끊긴 것으로 봅니다.
 * stale이 되면 표시가 dashboard_test 시뮬레이터로 되돌아갑니다. */
#define CAN_APP_RX_TIMEOUT_MS    600U

/* 0x501에서 디코드한 값 (원본 단위 그대로 보관) */
typedef struct
{
	uint16_t speed_cms;      /* 0.01 m/s */
	int16_t  current_da;     /* 0.1 A, 음수 = 회생 */
	uint16_t soc_permille;   /* 1/1000 (875 = 87.5%) */
	int16_t  power_w;        /* 1 W, 음수 = 회생 */

	uint32_t last_rx_tick;   /* 마지막 수신 시각 */
	uint32_t rx_count;       /* 수신 누적 */
	uint32_t drop_dlc;       /* DLC 8 미만이라 버린 수 */
	bool     valid;          /* 타임아웃 전 = true */

	/* 수신 품질 진단 (0x501 프레임 간격 / 차속 변화폭) */
	uint16_t rx_dt_min;      /* 프레임 간격 최소 [ms] */
	uint16_t rx_dt_max;      /* 최대 [ms] -- 규격 200ms 대비 얼마나 튀는지 */
	uint16_t rx_dt_avg;      /* 평균 [ms] */
	uint16_t rx_step_max;    /* 연속 프레임 간 차속 변화 최대 [0.01 m/s] */

	/* 마지막으로 받은 0x501 프레임의 원본 8바이트 (오프셋/엔디안 검증용) */
	uint8_t  raw[8];
	uint8_t  raw_dlc;
} CanTelemetry_t;

/** 상태 초기화. CanBus_Init() 뒤에 1회. */
void CanApp_Init(void);

/** 0x501 프레임 1개 디코드 (CanBus_OnFrame에서 호출 = 실물 버스 경로).
  * 주입 잠금이 걸려 있으면 조용히 무시합니다. */
void CanApp_OnTelemetry(const CanFrame_t *frame);

/** 0x501 프레임 1개를 잠금과 무관하게 강제로 디코드 (시험용 합성 주입 전용).
  * debug_log.c의 재생/시뮬레이터가 이 경로를 씁니다. */
void CanApp_InjectTelemetry(const CanFrame_t *frame);

/** 시험용 합성 프레임을 주입하는 동안 실물 0x501 수신을 막습니다.
  *
  * ⚠ 이게 없으면 실물 프레임과 합성 프레임이 같은 s_tlm을 번갈아 덮어써서
  *   화면 값이 두 세트 사이를 오갑니다(값이 나왔다 안 나왔다 깜빡임).
  *   실물이 95ms, 합성이 200ms 주기면 초당 여러 번 전체화면을 다시 그립니다.
  *
  * 잠금 중에도 can_bus.c의 수신/통계는 계속 돌기 때문에, 버스가 살아있는지는
  * g_dbg.can_rx로 계속 확인할 수 있습니다. */
void CanApp_SetInjectLock(bool on);

/** 메인 루프에서 매 회 호출.
  * 수신 타임아웃을 감시하고, 100 ms 규칙을 지켜 0x510을 주기 송신합니다. */
void CanApp_Process(void);

/** 최신 텔레메트리 스냅샷 */
void CanApp_GetTelemetry(CanTelemetry_t *out);

/** 0x501이 살아 있는가 (false면 표시 값이 0으로 떨어짐) */
bool CanApp_IsTelemetryValid(void);

/** 최신 텔레메트리를 계기판 위젯에 반영. valid일 때만 호출하세요. */
void CanApp_ApplyToUI(void);

/* ---- 단위 변환 헬퍼 (표시/디버그용) ---- */
uint16_t CanApp_SpeedKmh(void);      /* km/h */
uint8_t  CanApp_SocPercent(void);    /* % */

/** 0x510으로 보낼 ACC 명령 설정. 실제 송신은 CanApp_Process()가 200ms마다 합니다.
  *
  * @param acc_on     ACC(크루즈) on/off  -> byte 1 (0 = OFF, 1 = ON)
  * @param speed_kmh  ACC 설정속도 [km/h] -> byte 0 (0~150)
  *
  * 값은 저장만 하고 바로 보내지 않습니다. 매 루프 호출해도 됩니다. */
void CanApp_SetAccCommand(bool acc_on, uint8_t speed_kmh);

/** 0x510 송신 통계 (FIFO 적재 성공/실패 누적)
  * @note ok는 'Tx FIFO에 넣는 데 성공'이지 '버스에서 ACK를 받았다'가 아닙니다.
  *       실제 전달 여부는 FDCAN의 TEC / PSR.LEC로 판단하세요. */
void CanApp_GetTxStats(uint32_t *ok, uint32_t *fail);

#ifdef __cplusplus
}
#endif

#endif /* CAN_APP_H_ */
