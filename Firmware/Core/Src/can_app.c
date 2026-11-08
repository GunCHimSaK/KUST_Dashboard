/**
  ******************************************************************************
  * @file    can_app.c
  * @brief   차량 텔레메트리(0x501) / 명령(0x510) + 프레임 디스패처 - 구현
  ******************************************************************************
  */

#include "can_app.h"
#include "can_bus.h"
#include "can_text.h"
#include "dashboard_ui.h"
#include "vehicle_data.h"
#include "main.h"
#include <string.h>

static CanTelemetry_t s_tlm = {0};
static uint16_t s_command   = 0;
static uint32_t s_tx_tick   = 0;
static uint32_t s_tx_ok     = 0;
static uint32_t s_tx_fail   = 0;

/* ── 진단: 최근 8개 0x501 프레임의 원본 바이트 링버퍼 ──────────────────
 * 디버거로 한 번에 읽어 연속 프레임을 비교하기 위한 것입니다.
 * (CLI 접속이 보드를 리셋해 버려서, 한 프레임씩 찍으면 항상 비슷한
 *  시점만 보게 됩니다. 이 링버퍼는 그 영향을 받지 않습니다) */
uint8_t g_can_raw_hist[8][8];
uint8_t g_can_raw_head;

/* ── 리틀엔디안 디코드 (하위 바이트 먼저) ────────────────────────────────
 * Cortex-M33도 리틀엔디안이라 memcpy로도 되지만, 바이트 순서를 코드에
 * 명시해 두면 나중에 빅엔디안으로 바뀌어도 여기만 고치면 됩니다.
 * (규격서: "빅엔디안을 기대하시면 미리 말씀해 주세요") */
static uint16_t rd_u16le(const uint8_t *p)
{
	return (uint16_t) ((uint16_t) p[0] | ((uint16_t) p[1] << 8));
}

static int16_t rd_i16le(const uint8_t *p)
{
	return (int16_t) rd_u16le(p);
}

static void wr_u16le(uint8_t *p, uint16_t v)
{
	p[0] = (uint8_t) (v & 0xFFU);
	p[1] = (uint8_t) ((v >> 8) & 0xFFU);
}

/* ── 초기화 ──────────────────────────────────────────────────────────── */

void CanApp_Init(void)
{
	memset(&s_tlm, 0, sizeof(s_tlm));
	s_tlm.rx_dt_min = 0xFFFFU;
	s_command = 0;
	s_tx_tick = HAL_GetTick();
	s_tx_ok   = 0;
	s_tx_fail = 0;
}

/* ── 0x501 수신 ──────────────────────────────────────────────────────── */

/* 시험용 합성 주입 중에는 실물 0x501을 무시한다.
 * 둘 다 s_tlm을 쓰면 화면이 두 값 세트 사이를 오간다 (can_app.h 참고). */
static bool s_inject_lock = false;

void CanApp_SetInjectLock(bool on)
{
	s_inject_lock = on;
}

void CanApp_OnTelemetry(const CanFrame_t *frame)
{
	if (s_inject_lock)
	{
		return;   /* 합성 주입 재생 중 -- 실물 프레임은 버린다 */
	}
	CanApp_InjectTelemetry(frame);
}

void CanApp_InjectTelemetry(const CanFrame_t *frame)
{
	if ((frame == NULL) || (frame->id != CAN_APP_TELEMETRY_ID))
	{
		return;
	}

	/* 규격상 DLC는 항상 8. 짧으면 필드가 잘리므로 버립니다. */
	if (frame->dlc < 8U)
	{
		s_tlm.drop_dlc++;
		return;
	}

	/* ── 수신 품질 계측 (상대 송신이 규칙적인지 판정) ─────────────────── */
	if (s_tlm.rx_count > 0U)
	{
		uint32_t dt   = frame->timestamp - s_tlm.last_rx_tick;
		int32_t  step = (int32_t) rd_u16le(&frame->data[0]) - (int32_t) s_tlm.speed_cms;

		if (dt > 0xFFFFU) { dt = 0xFFFFU; }
		if ((uint16_t) dt < s_tlm.rx_dt_min) { s_tlm.rx_dt_min = (uint16_t) dt; }
		if ((uint16_t) dt > s_tlm.rx_dt_max) { s_tlm.rx_dt_max = (uint16_t) dt; }
		/* 프레임 간격 평균은 지수이동평균(N=8)으로 냅니다.
		 * 부팅 이후 누적 평균이면 최근 변화(상대가 주기를 바꾸거나 끊겼다 붙는 등)를
		 * 못 잡고, 누적합이 계속 커져 장시간 동작에서 오버플로도 생깁니다. */
		if (s_tlm.rx_dt_avg == 0U)
		{
			s_tlm.rx_dt_avg = (uint16_t) dt;          /* 첫 표본으로 시드 */
		}
		else
		{
			s_tlm.rx_dt_avg = (uint16_t) (((uint32_t) s_tlm.rx_dt_avg * 7UL + dt) / 8UL);
		}

		if (step < 0) { step = -step; }
		if (step > (int32_t) s_tlm.rx_step_max) { s_tlm.rx_step_max = (uint16_t) step; }
	}

	/* 원본 바이트 보존 (보드 팀과 오프셋/엔디안 대조용) */
	memcpy(s_tlm.raw, frame->data, 8);
	memcpy(g_can_raw_hist[g_can_raw_head], frame->data, 8);
	g_can_raw_head = (uint8_t) ((g_can_raw_head + 1U) & 7U);
	s_tlm.raw_dlc = frame->dlc;

	s_tlm.speed_cms    = rd_u16le(&frame->data[0]);
	s_tlm.current_da   = rd_i16le(&frame->data[2]);
	s_tlm.soc_permille = rd_u16le(&frame->data[4]);
	s_tlm.power_w      = rd_i16le(&frame->data[6]);

	s_tlm.last_rx_tick = frame->timestamp;
	s_tlm.valid        = true;
	s_tlm.rx_count++;
}

/* ── 단위 변환 ───────────────────────────────────────────────────────── */

uint16_t CanApp_SpeedKmh(void)
{
	/* 0.01 m/s -> km/h :  x * 0.01 * 3.6 = x * 0.036
	 * 정수로 x * 36 / 1000. speed_cms 최대 65535 -> 65535*36 = 2.36M, u32에 안전. */
	return (uint16_t) (((uint32_t) s_tlm.speed_cms * 36UL) / 1000UL);
}

uint8_t CanApp_SocPercent(void)
{
	uint16_t pct = (uint16_t) (s_tlm.soc_permille / 10U);   /* 875 -> 87 */
	return (uint8_t) ((pct > 100U) ? 100U : pct);
}

/* ── UI 반영 ─────────────────────────────────────────────────────────── */





void CanApp_ApplyToUI(void)
{
	uint16_t kmh = 0U;
	uint8_t  soc = 0U;
	int32_t  pwr = 0;

	/* ── 현재 표시 항목: 차속 / SOC / 전력 ───────────────────────────
	 * 0x501이 안 들어오면(valid=false) 전부 0으로 떨어집니다.
	 *
	 * ⚠ 전력은 '모터 전력'(byte 6~7)인데 화면에서는 MPPT 자리에 띄웁니다.
	 *   패널의 모터 표시 영역이 파손되어 안 보이기 때문입니다.
	 *   패널 교체 후에는 SetMppt -> SetMotor 로 되돌리세요.
	 *
	 * 부호는 그대로 전달합니다. 음수 = 회생(배터리로 되돌아가는 전력)이고
	 * 화면에는 "-1234W"로 그려집니다. 표시 폭은 DashboardUI_Init()에서
	 * 84px(7글자)로 넓혀 두었으므로 int16 전 범위가 한 줄에 들어갑니다. */
	if (s_tlm.valid)
	{
		kmh = CanApp_SpeedKmh();
		soc = CanApp_SocPercent();
		pwr = (int32_t) s_tlm.power_w;   /* 음수 = 회생 */
	}

	/* Set 함수들은 값이 실제로 바뀔 때만 LVGL을 건드리므로 매 루프 호출해도 됩니다 */
	DashboardUI_SetSpeed((uint8_t) ((kmh > 255U) ? 255U : kmh));
	DashboardUI_SetBattery(soc);   /* ui_BatteryVal "%d%%" */
	DashboardUI_SetBar(soc);       /* ui_Bar1 0~100 */
	DashboardUI_SetMppt(pwr);      /* ui_MpptVal "%dW" <- 모터 전력(부호 포함) */
}

/* ── 0x510 송신 + 타임아웃 감시 ──────────────────────────────────────── */

void CanApp_SetAccCommand(bool acc_on, uint8_t speed_kmh)
{
	/* byte 0 = ACC 설정속도, byte 1 = ACC on/off.
	 *
	 * wr_u16le()가 하위 바이트부터 쓰므로 아래 조합이 그대로
	 * payload[0] = speed_kmh, payload[1] = acc_on 이 됩니다.
	 *
	 * ⚠ 보드가 0x510 -> 0x420으로 중계할 때 '앞 2바이트만' 전달하므로,
	 *   모터 팀이 봐야 하는 값은 반드시 byte 0~1 안에 들어가야 합니다.
	 *   여기에 필드를 더 얹으려면 먼저 보드 팀과 중계 폭을 늘리는 협의가
	 *   필요합니다.
	 *
	 * 설정속도는 ACC가 꺼져 있어도 그대로 실어 보냅니다. 모터 팀이
	 * '대기 중인 설정값'을 미리 볼 수 있고, byte 1이 0이면 무시하면 됩니다. */
	s_command = (uint16_t) speed_kmh
	          | ((uint16_t) (acc_on ? 1U : 0U) << 8);
}

void CanApp_Process(void)
{
	uint32_t now = HAL_GetTick();
	uint8_t  payload[8] = {0};

	/* 0x501이 끊기면 stale 처리 -> CanApp_ApplyToUI()가 표시를 0으로 내림 */
	if (s_tlm.valid && ((now - s_tlm.last_rx_tick) > CAN_APP_RX_TIMEOUT_MS))
	{
		s_tlm.valid = false;
	}

	/* 최소 간격 100ms 규칙을 지켜 주기 송신 */
	if ((now - s_tx_tick) < CAN_APP_TX_PERIOD_MS)
	{
		return;
	}
	s_tx_tick = now;

	wr_u16le(&payload[0], s_command);   /* byte 2~7은 0 유지 */

	if (CanBus_Send(CAN_APP_COMMAND_ID, false, payload, 8U))
	{
		s_tx_ok++;
	}
	else
	{
		s_tx_fail++;   /* Tx FIFO 가득참 또는 미시작 */
	}
}

void CanApp_GetTelemetry(CanTelemetry_t *out)
{
	if (out != NULL)
	{
		*out = s_tlm;
	}
}


void CanApp_GetTxStats(uint32_t *ok, uint32_t *fail)
{
	if (ok   != NULL) { *ok   = s_tx_ok; }
	if (fail != NULL) { *fail = s_tx_fail; }
}
bool CanApp_IsTelemetryValid(void)
{
	return s_tlm.valid;
}

/* ── 수신 프레임 디스패치 ────────────────────────────────────────────── */

/**
  * @brief  can_bus.c의 __weak CanBus_OnFrame()을 대체하는 실제 구현
  * @note   CanBus_Process()에서 호출되므로 메인 루프 컨텍스트입니다.
  *         LVGL을 직접 불러도 안전합니다.
  */
void CanBus_OnFrame(const CanFrame_t *frame)
{
	if (frame->extended)
	{
		return;   /* 이 버스는 표준 11bit만 사용 */
	}

	switch (frame->id)
	{
	case CAN_APP_TELEMETRY_ID:    /* 0x501 : 차속/전류/SOC/모터전력 */
		CanApp_OnTelemetry(frame);
		break;

	case CAN_TEXT_RX_ID:          /* 0x500 : 지상국 텍스트 메시지 */
		CanText_OnFrame(frame);
		break;

	default:
		break;
	}
}
