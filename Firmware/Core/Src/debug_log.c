/**
  ******************************************************************************
  * @file    debug_log.c
  * @brief   디버그 출력 / 진단 스냅샷 - 구현
  ******************************************************************************
  */

#include "debug_log.h"
#include "main.h"
#include "can_bus.h"
#include "can_text.h"
#include "can_app.h"
#include "SHARP_MIP.h"
#include "imon.h"
#include "lvgl.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

volatile DbgSnapshot_t g_dbg = {0};

/* ── ITM(SWO) 출력 ────────────────────────────────────────────────────── */

/* 호스트가 SWO를 안 읽어 갈 때 무한 대기하지 않도록 스핀 횟수를 제한합니다.
 * 계기판은 실시간 표시가 우선이라 로그 몇 줄보다 화면이 안 멈추는 게 중요합니다. */
#define DBG_ITM_SPIN_LIMIT    2000U

static void itm_putc(uint8_t c)
{
	uint32_t guard;

	/* 디버거 미연결 또는 SWV 미활성 -> 조용히 버림 */
	if (((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0UL) || ((ITM->TER & 1UL) == 0UL))
	{
		return;
	}

	for (guard = 0U; ITM->PORT[0U].u32 == 0UL; guard++)
	{
		if (guard >= DBG_ITM_SPIN_LIMIT)
		{
			return;   /* 호스트가 안 읽는다 -> 이 문자는 포기 */
		}
	}
	ITM->PORT[0U].u8 = c;
}

/**
  * @brief  syscalls.c의 weak __io_putchar 훅을 채웁니다.
  * @note   이걸로 표준 printf() 출력도 SWO로 나갑니다.
  *         (printf는 코드 크기가 크고 느리니 DBG_Printf를 권장)
  */
int __io_putchar(int ch)
{
	itm_putc((uint8_t) ch);
	return ch;
}

void DBG_Printf(const char *fmt, ...)
{
	/* ⚠ 정적 버퍼 -> 재진입 불가. 메인 루프에서만 호출할 것. */
	static char buf[192];
	va_list ap;
	int      n;
	int      i;

	n = snprintf(buf, sizeof(buf), "[%8lu] ", (unsigned long) HAL_GetTick());
	if ((n < 0) || ((size_t) n >= sizeof(buf)))
	{
		return;
	}

	va_start(ap, fmt);
	n += vsnprintf(&buf[n], sizeof(buf) - (size_t) n, fmt, ap);
	va_end(ap);

	/* vsnprintf는 잘렸을 때 '잘리지 않았다면 필요했을 길이'를 돌려주므로 클램프 */
	if (n < 0)
	{
		return;
	}
	if ((size_t) n > sizeof(buf) - 1U)
	{
		n = (int) (sizeof(buf) - 1U);
	}

	for (i = 0; i < n; i++)
	{
		itm_putc((uint8_t) buf[i]);
	}
	itm_putc((uint8_t) '\r');
	itm_putc((uint8_t) '\n');
}

/* ── 루프 주기 측정 ───────────────────────────────────────────────────── */

static uint32_t s_last_loop_tick = 0;
static uint32_t s_loop_sum       = 0;
static uint32_t s_loop_n         = 0;
static uint16_t s_loop_min       = 0xFFFFU;
static uint16_t s_loop_max       = 0;

static void loop_timing_sample(void)
{
	uint32_t now = HAL_GetTick();
	uint32_t dt  = now - s_last_loop_tick;

	s_last_loop_tick = now;

	if (dt > 0xFFFFU)
	{
		dt = 0xFFFFU;   /* 첫 호출/오버플로 방어 */
	}
	if ((uint16_t) dt < s_loop_min) { s_loop_min = (uint16_t) dt; }
	if ((uint16_t) dt > s_loop_max) { s_loop_max = (uint16_t) dt; }

	s_loop_sum += dt;
	s_loop_n++;
	g_dbg.loop_count++;
}


/* ── lv_timer_handler() 소요 시간 ─────────────────────────────────────── */

static uint32_t s_lv_sum = 0;
static uint32_t s_lv_n   = 0;
static uint16_t s_lv_min = 0xFFFFU;
static uint16_t s_lv_max = 0;

void DBG_NoteLvglMs(uint32_t ms)
{
	if (ms > 0xFFFFU) { ms = 0xFFFFU; }
	if ((uint16_t) ms < s_lv_min) { s_lv_min = (uint16_t) ms; }
	if ((uint16_t) ms > s_lv_max) { s_lv_max = (uint16_t) ms; }
	s_lv_sum += ms;
	s_lv_n++;
}
/* ── BT_3 셀프테스트 ──────────────────────────────────────────────────── */

static volatile bool s_selftest_req = false;

void DBG_RequestSelfTest(void)
{
	s_selftest_req = true;   /* ISR에서 불려도 안전하도록 플래그만 */
}

#if DBG_ENABLE
/**
  * @brief  0x500 프레임을 합성해서 수신 경로에 그대로 흘려 넣습니다.
  * @note   CAN 하드웨어/지상국 없이도 조립 -> 패딩 제거 -> UTF-8 정리 ->
  *         배너 표시까지 전 구간을 실기에서 확인할 수 있습니다.
  *         프레임을 직접 만들어 넣으므로 규격 그대로(DLC 8 고정, 0x00 패딩)
  *         재현됩니다.
  * @param  text  ASCII 본문. 배너 폰트가 ASCII 전용이라 시험 문자열도
  *               실제로 표시될 문자만 씁니다.
  */
static void selftest_inject(const char *text)
{
	CanFrame_t f;
	size_t     len = strlen(text);
	size_t     off = 0;
	uint8_t    seq = 0;

	DBG_Printf("selftest: \"%s\" (%u bytes)", text, (unsigned) len);

	if (len > CAN_TEXT_MAX_BYTES)
	{
		DBG_Printf("selftest: body exceeds %uB limit, aborted", CAN_TEXT_MAX_BYTES);
		return;
	}

	do
	{
		size_t n = len - off;
		if (n > 7U)
		{
			n = 7U;
		}

		memset(&f, 0, sizeof(f));
		f.id        = CAN_TEXT_RX_ID;
		f.extended  = false;
		f.dlc       = 8U;                      /* 규격: DLC는 항상 8 */
		f.timestamp = HAL_GetTick();
		f.data[0]   = seq;
		if ((off + n) >= len)
		{
			f.data[0] |= 0x80U;                /* 마지막 프레임 */
		}
		memcpy(&f.data[1], &text[off], n);     /* 남는 자리는 memset의 0x00 패딩 */

		CanText_OnFrame(&f);

		off += n;
		seq++;
	} while (off < len);

	DBG_Printf("selftest: injected %u frame(s)", (unsigned) seq);
}
#endif /* DBG_ENABLE */


/* ── 0x501 텔레메트리 합성 주입 (버튼 없이 수신 경로 시험) ────────────── */

#define TLM_SIM_PERIOD_MS   200U    /* 규격: 0x501은 200ms 주기 */
#define TLM_SIM_MAX_CMS     2500    /* 25.00 m/s = 90 km/h */
#define TLM_SIM_STEP_CMS    17      /* 150스텝(30초)에 0->2500 -> 왕복 60초 */

volatile DbgControl_t g_dbg_ctl = {0, 0, 0};

static uint32_t s_tlm_sim_tick = 0;
static int16_t  s_tlm_sim_cms  = 0;
static int8_t   s_tlm_sim_dir  = 1;

/* ── 실측 프레임 재생 (BT_4 / g_dbg_ctl.frame_sim) ──────────────────────
 *
 * 보드 팀이 '회생 최대' 상태에서 캡처한 실제 0x501 프레임입니다.
 * 규격대로 i16 리틀엔디안으로 읽으면 전부 물리적으로 말이 되는 값이라,
 * 송신측이 규격을 지키고 있다는 근거이자 우리 표시 경로의 검증 벡터입니다.
 *
 *   byte 0-1  0x073A =  1850  -> 18.50 m/s -> 66 km/h
 *   byte 2-3  0xFF41 =  -191  -> -19.1 A   (회생)
 *   byte 4-5  0x0318 =   792  -> 79.2 %
 *   byte 6-7  0xF736 = -2250  -> -2250 W   (회생)
 *
 * 한 번만 주입하면 CAN_APP_RX_TIMEOUT_MS(600ms) 뒤에 stale 처리되어 화면이
 * 0으로 떨어져 버립니다. 눈으로 확인할 수 있도록 규격 주기(200ms)로
 * TLM_FRAME_HOLD_MS 동안 반복 재생한 뒤 스스로 멈춥니다. */
#define TLM_FRAME_HOLD_MS   10000U

static const uint8_t s_frame_regen[8] =
	{ 0x3AU, 0x07U, 0x41U, 0xFFU, 0x18U, 0x03U, 0x36U, 0xF7U };

static volatile bool s_frame_req    = false;
static bool          s_frame_active = false;
static uint32_t      s_frame_start  = 0;
static uint32_t      s_frame_tick   = 0;

void DBG_RequestCapturedFrame(void)
{
	s_frame_req = true;   /* ISR에서 불려도 안전하도록 플래그만 */
}

/** 캡처 프레임 1개를 수신 경로에 그대로 흘려 넣는다 */
static void captured_frame_inject(void)
{
	CanFrame_t f;

	memset(&f, 0, sizeof(f));
	f.id        = CAN_APP_TELEMETRY_ID;
	f.extended  = false;
	f.dlc       = 8U;
	f.timestamp = HAL_GetTick();
	memcpy(f.data, s_frame_regen, 8);

	CanApp_InjectTelemetry(&f);
}

/**
  * @brief  규격대로 0x501 프레임을 만들어 수신 경로에 직접 넣습니다.
  * @note   CAN 하드웨어를 거치지 않으므로 트랜시버/상대 보드 없이도
  *         디코드 -> 단위환산 -> 화면 표시까지 전 구간이 검증됩니다.
  *         전부 리틀엔디안(하위 바이트 먼저).
  */
static void telemetry_inject(void)
{
	CanFrame_t f;
	int32_t    v = s_tlm_sim_cms;

	v += (int32_t) TLM_SIM_STEP_CMS * s_tlm_sim_dir;
	if (v >= TLM_SIM_MAX_CMS) { v = TLM_SIM_MAX_CMS; s_tlm_sim_dir = -1; }
	if (v <= 0)               { v = 0;               s_tlm_sim_dir =  1; }
	s_tlm_sim_cms = (int16_t) v;

	memset(&f, 0, sizeof(f));
	f.id        = CAN_APP_TELEMETRY_ID;
	f.extended  = false;
	f.dlc       = 8U;
	f.timestamp = HAL_GetTick();

	/* 모터 전력은 스윕 방향을 따라간다.
	 *   가속 구간(dir=+1) -> +3400 W (소비)
	 *   감속 구간(dir=-1) -> -1200 W (회생)
	 * 음수가 화면에 "-1200W"로 제대로 그려지는지 확인하는 용도이기도 하다.
	 * 30초마다 부호가 바뀌므로 한 번만 켜 두면 양쪽 다 볼 수 있다. */
	int16_t sim_pw = (s_tlm_sim_dir > 0) ? (int16_t) 3400 : (int16_t) -1200;

	f.data[0] = (uint8_t) ((uint16_t) s_tlm_sim_cms & 0xFFU);        /* 차속 u16 */
	f.data[1] = (uint8_t) (((uint16_t) s_tlm_sim_cms >> 8) & 0xFFU);
	f.data[2] = 0xABU; f.data[3] = 0xFFU;   /* 버스 전류 i16 = -85  -> -8.5 A */
	f.data[4] = 0x6BU; f.data[5] = 0x03U;   /* SOC       u16 = 875  -> 87.5 % */
	f.data[6] = (uint8_t) ((uint16_t) sim_pw & 0xFFU);               /* 모터 전력 i16 */
	f.data[7] = (uint8_t) (((uint16_t) sim_pw >> 8) & 0xFFU);

	CanApp_InjectTelemetry(&f);
}
/* ── 진단 덤프 ────────────────────────────────────────────────────────── */

static void snapshot_update(void)
{
	CanStats_t     can;
	CanTextStats_t txt;
	lv_mem_monitor_t mon;

	CanBus_GetStats(&can);
	CanText_GetStats(&txt);
	lv_mem_monitor(&mon);

	g_dbg.uptime_ms        = HAL_GetTick();

	g_dbg.loop_ms_min      = (s_loop_min == 0xFFFFU) ? 0U : s_loop_min;
	g_dbg.loop_ms_max      = s_loop_max;
	g_dbg.loop_ms_avg      = (s_loop_n > 0U)
	                       ? (uint16_t) (s_loop_sum / s_loop_n) : 0U;

	g_dbg.can_rx           = can.rx_count;
	g_dbg.can_tx           = can.tx_count;
	g_dbg.can_rx_overflow  = can.rx_overflow;
	g_dbg.can_err          = can.error_count;
	g_dbg.can_bus_off      = can.bus_off ? 1U : 0U;

	g_dbg.txt_frames       = txt.frame_count;
	g_dbg.txt_msgs         = txt.msg_count;
	g_dbg.txt_drop_seq     = txt.drop_seq;
	g_dbg.txt_drop_timeout = txt.drop_timeout;
	g_dbg.txt_drop_overflow= txt.drop_overflow;
	g_dbg.txt_trim_utf8    = txt.trim_utf8;

	g_dbg.lv_used_pct      = mon.used_pct;
	g_dbg.lv_frag_pct      = mon.frag_pct;
	g_dbg.lv_free_biggest  = (uint32_t) mon.free_biggest_size;

	g_dbg.imon_raw         = IMON_GetRaw();
	g_dbg.imon_mv          = IMON_GetMilliVolts();
	g_dbg.imon_ma          = IMON_GetLoadCurrent_mA();
	g_dbg.imon_valid       = IMON_IsValid() ? 1U : 0U;

	{
		CanTelemetry_t t;
		CanApp_GetTelemetry(&t);
		g_dbg.tlm_speed_cms    = t.speed_cms;
		g_dbg.tlm_current_da   = t.current_da;
		g_dbg.tlm_soc_permille = t.soc_permille;
		g_dbg.tlm_power_w      = t.power_w;
		g_dbg.tlm_speed_kmh    = CanApp_SpeedKmh();
		g_dbg.tlm_soc_pct      = CanApp_SocPercent();
		g_dbg.tlm_valid        = t.valid ? 1U : 0U;
		g_dbg.tlm_rx_count     = t.rx_count;
		g_dbg.tlm_drop_dlc     = t.drop_dlc;
		g_dbg.rx_dt_min        = t.rx_dt_min;
		g_dbg.rx_dt_max        = t.rx_dt_max;
		g_dbg.rx_dt_avg        = t.rx_dt_avg;
		g_dbg.rx_step_max      = t.rx_step_max;
		memcpy((void *) g_dbg.tlm_raw, t.raw, 8);
		g_dbg.tlm_raw_dlc      = t.raw_dlc;
		{
			uint32_t ok = 0, fail = 0;
			CanApp_GetTxStats(&ok, &fail);
			g_dbg.cmd_tx_ok   = ok;
			g_dbg.cmd_tx_fail = fail;
		}
	}

	g_dbg.lv_ms_min = (s_lv_min == 0xFFFFU) ? 0U : s_lv_min;
	g_dbg.lv_ms_max = s_lv_max;
	g_dbg.lv_ms_avg = (s_lv_n > 0U) ? (uint16_t) (s_lv_sum / s_lv_n) : 0U;
	g_dbg.dma_timeouts = sharp_mip_get_dma_timeouts();
	s_lv_min = 0xFFFFU; s_lv_max = 0U; s_lv_sum = 0U; s_lv_n = 0U;

	/* 다음 구간을 위해 리셋 */
	s_loop_min = 0xFFFFU;
	s_loop_max = 0U;
	s_loop_sum = 0U;
	s_loop_n   = 0U;
}

void DBG_NoteMessage(const char *utf8)
{
	size_t len;

	if (utf8 == NULL)
	{
		g_dbg.txt_last[0]  = '\0';
		g_dbg.txt_last_len = 0U;
		return;
	}

	len = strlen(utf8);
	if (len > sizeof(g_dbg.txt_last) - 1U)
	{
		len = sizeof(g_dbg.txt_last) - 1U;
	}
	memcpy((void *) g_dbg.txt_last, utf8, len);
	g_dbg.txt_last[len] = '\0';
	g_dbg.txt_last_len  = (uint8_t) len;

	DBG_LOG("msg: \"%s\" (%u bytes)", utf8, (unsigned) len);
}

void DBG_Init(void)
{
	s_last_loop_tick = HAL_GetTick();
	memset((void *) &g_dbg, 0, sizeof(g_dbg));
	snapshot_update();

	DBG_LOG("---- displaycontrol boot ----");
	DBG_LOG("SYSCLK %lu Hz, LVGL %d.%d, CAN 500k / text ID 0x%03X",
	        (unsigned long) HAL_RCC_GetSysClockFreq(),
	        LVGL_VERSION_MAJOR, LVGL_VERSION_MINOR, (unsigned) CAN_TEXT_RX_ID);
	DBG_LOG("press BT_3 to inject a 0x500 selftest message (ASCII only)");
}

void DBG_Process(void)
{
	static uint32_t dumpTick = 0;
	uint32_t now;

	loop_timing_sample();

	/* ---- 캡처한 실측 0x501 프레임 재생 (BT_4 / g_dbg_ctl.frame_sim) ---- */
	if (g_dbg_ctl.frame_sim != 0U)
	{
		g_dbg_ctl.frame_sim = 0U;   /* 한 번 누른 것으로 처리 */
		s_frame_req = true;
	}
	if (s_frame_req)
	{
		s_frame_req    = false;
		s_frame_active = true;
		s_frame_start  = HAL_GetTick();
		s_frame_tick   = s_frame_start - TLM_SIM_PERIOD_MS;   /* 즉시 첫 주입 */
		DBG_LOG("captured 0x501 replay start: expect 66km/h / 79%% / -2250W");
	}
	if (s_frame_active)
	{
		uint32_t fnow = HAL_GetTick();

		if ((fnow - s_frame_start) >= TLM_FRAME_HOLD_MS)
		{
			s_frame_active = false;
			DBG_LOG("captured 0x501 replay end (%lu ms)",
			        (unsigned long) TLM_FRAME_HOLD_MS);
		}
		else if ((fnow - s_frame_tick) >= TLM_SIM_PERIOD_MS)
		{
			s_frame_tick = fnow;
			captured_frame_inject();
		}
	}

	/* 합성 주입이 도는 동안에는 실물 0x501을 막는다.
	 *
	 * 이게 없으면 실물(현장 실측 95ms 주기)과 합성(200ms 주기)이 같은 s_tlm을
	 * 번갈아 덮어써서 화면 값이 두 세트 사이를 오갑니다. 값이 바뀔 때마다
	 * 전체화면(약 148ms)을 다시 그리므로 눈에는 '깜빡거림'으로 보입니다.
	 * 잠금 중에도 can_bus.c의 수신/통계는 계속 돌아 g_dbg.can_rx는 올라갑니다. */
	CanApp_SetInjectLock(s_frame_active || (g_dbg_ctl.tlm_sim != 0U));

	/* 디버거로 켠 0x501 합성 주입 (g_dbg_ctl.tlm_sim = 1)
	 * 캡처 프레임 재생 중에는 양보한다 -- 둘이 동시에 들어가면 화면이 튄다. */
	if ((g_dbg_ctl.tlm_sim != 0U) && !s_frame_active)
	{
		if ((HAL_GetTick() - s_tlm_sim_tick) >= TLM_SIM_PERIOD_MS)
		{
			s_tlm_sim_tick = HAL_GetTick();
			telemetry_inject();
		}
	}
	else if (g_dbg_ctl.tlm_sim == 0U)
	{
		s_tlm_sim_cms = 0;      /* 껐다 켜면 0부터 다시 */
		s_tlm_sim_dir = 1;
	}

	/* 디버거로 켠 0x500 텍스트 1회 주입 (g_dbg_ctl.text_sim = 1) */
	if (g_dbg_ctl.text_sim != 0U)
	{
		g_dbg_ctl.text_sim = 0U;
		s_selftest_req = true;
	}

	if (s_selftest_req)
	{
		s_selftest_req = false;
#if DBG_ENABLE
		/* ASCII 57바이트 = 9프레임(seq 0~8). CAN_TEXT_MAX_FRAMES 상한을
		 * 정확히 채우므로 마지막 프레임 처리까지 그대로 밟습니다.
		 * (배너 폰트가 ASCII 전용이라 시험 문자열도 ASCII로 둡니다) */
		selftest_inject("Drive status OK, continue to next checkpoint ahead safely");
#endif
	}

	now = HAL_GetTick();
	if ((now - dumpTick) < DBG_DUMP_PERIOD_MS)
	{
		return;
	}
	dumpTick = now;

	snapshot_update();

#if DBG_ENABLE
	DBG_Printf("loop min/avg/max = %u/%u/%u ms | lv heap %u%% (frag %u%%)",
	           g_dbg.loop_ms_min, g_dbg.loop_ms_avg, g_dbg.loop_ms_max,
	           g_dbg.lv_used_pct, g_dbg.lv_frag_pct);
	DBG_Printf("lv_timer_handler min/avg/max = %u/%u/%u ms | dma_timeouts=%lu",
	           g_dbg.lv_ms_min, g_dbg.lv_ms_avg, g_dbg.lv_ms_max,
	           (unsigned long) g_dbg.dma_timeouts);
	DBG_Printf("can rx=%lu tx=%lu ovf=%lu err=%lu busoff=%u",
	           (unsigned long) g_dbg.can_rx, (unsigned long) g_dbg.can_tx,
	           (unsigned long) g_dbg.can_rx_overflow,
	           (unsigned long) g_dbg.can_err, g_dbg.can_bus_off);
	DBG_Printf("txt frame=%lu msg=%lu | drop seq=%lu to=%lu ovf=%lu utf8=%lu",
	           (unsigned long) g_dbg.txt_frames, (unsigned long) g_dbg.txt_msgs,
	           (unsigned long) g_dbg.txt_drop_seq,
	           (unsigned long) g_dbg.txt_drop_timeout,
	           (unsigned long) g_dbg.txt_drop_overflow,
	           (unsigned long) g_dbg.txt_trim_utf8);
	DBG_Printf("imon %u mA (%u mV, raw %u) valid=%u",
	           g_dbg.imon_ma, g_dbg.imon_mv, g_dbg.imon_raw, g_dbg.imon_valid);
	DBG_Printf("tlm valid=%u rx=%lu | %u km/h  %d.%u A  %u%%  %d W",
	           g_dbg.tlm_valid, (unsigned long) g_dbg.tlm_rx_count,
	           g_dbg.tlm_speed_kmh,
	           g_dbg.tlm_current_da / 10, (unsigned) (abs(g_dbg.tlm_current_da) % 10),
	           g_dbg.tlm_soc_pct, g_dbg.tlm_power_w);
	DBG_Printf("cmd tx ok=%lu fail=%lu",
	           (unsigned long) g_dbg.cmd_tx_ok, (unsigned long) g_dbg.cmd_tx_fail);
#endif
}
