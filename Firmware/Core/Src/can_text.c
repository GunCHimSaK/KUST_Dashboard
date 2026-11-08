/**
  ******************************************************************************
  * @file    can_text.c
  * @brief   CAN 텍스트 메시지 수신/조립 (Bus B, ID 0x500) - 구현
  ******************************************************************************
  */

#include "can_text.h"
#include "main.h"      /* HAL_GetTick */
#include <string.h>

/* ── 조립 상태 ────────────────────────────────────────────────────────── */
static uint8_t  s_buf[CAN_TEXT_RAW_CAPACITY]; /* 조립 중인 본문 (패딩 포함) */
static uint8_t  s_len      = 0;             /* 조립된 바이트 수 */
static uint8_t  s_next_seq = 0;             /* 다음에 와야 할 시퀀스 번호 */
static bool     s_active   = false;         /* 조립 진행 중 */
static uint32_t s_tick     = 0;             /* 마지막 프레임 수신 시각 */

/* ── 완성된 메시지 ────────────────────────────────────────────────────── */
static char     s_msg[CAN_TEXT_MAX_BYTES + 1] = {0};
static bool     s_msg_new = false;

static CanTextStats_t s_stats = {0};

/**
  * @brief  버퍼 앞쪽에서 온전한 UTF-8 시퀀스가 끝나는 지점을 찾는다
  * @note   프레임 유실이나 62바이트 절단으로 멀티바이트 문자가 중간에서 잘리면
  *         LVGL이 깨진 글리프를 그리므로, 마지막 완성 문자까지만 남깁니다.
  *         (overlong/surrogate까지는 검사하지 않습니다. 여기서 막으려는 건
  *          '깨진 인코딩'이 아니라 '중간에서 잘린 문자'입니다)
  * @retval 유효한 바이트 수 (0 ~ len)
  */
static uint8_t utf8_valid_len(const uint8_t *buf, uint8_t len)
{
	uint8_t i = 0U;

	while (i < len)
	{
		uint8_t c = buf[i];
		uint8_t need;
		uint8_t k;
		bool    ok = true;

		if (c < 0x80U)                        { need = 1U; }
		else if ((c >= 0xC2U) && (c <= 0xDFU)) { need = 2U; }
		else if ((c >= 0xE0U) && (c <= 0xEFU)) { need = 3U; }
		else if ((c >= 0xF0U) && (c <= 0xF4U)) { need = 4U; }
		else                                   { break; }        /* 불법 리드바이트 */

		if ((uint16_t) i + need > (uint16_t) len)
		{
			break;   /* 시퀀스가 버퍼 끝에서 잘림 */
		}

		for (k = 1U; k < need; k++)
		{
			if ((buf[i + k] < 0x80U) || (buf[i + k] > 0xBFU))
			{
				ok = false;
				break;
			}
		}
		if (!ok)
		{
			break;
		}

		i = (uint8_t) (i + need);
	}

	return i;
}

/** 조립 중이던 내용을 버린다 */
static void reasm_abort(uint32_t *counter)
{
	if (s_active)
	{
		(*counter)++;
	}
	s_active   = false;
	s_len      = 0U;
	s_next_seq = 0U;
}

/** 마지막 프레임까지 받았을 때: 패딩/인코딩을 정리해 s_msg에 확정 */
static void reasm_finish(void)
{
	uint8_t valid;

	/* ⚠ 마지막 프레임의 유효 길이가 전달되지 않으므로 뒤쪽 0x00 패딩을 잘라낸다.
	 * UTF-8 본문에는 0x00이 없으므로 버퍼 전체에서 잘라내도 결과는 같다. */
	while ((s_len > 0U) && (s_buf[s_len - 1U] == 0x00U))
	{
		s_len--;
	}


	/* 패딩을 걷어낸 본문이 62바이트를 넘으면 송신측 규격 위반입니다.
	 * (63바이트 전부가 0이 아닌 경우) 메시지를 통째로 버리기보다
	 * 상한까지만 잘라서 보여 주고, 뒤에서 UTF-8 정리를 한 번 더 합니다. */
	if (s_len > CAN_TEXT_MAX_BYTES)
	{
		s_stats.drop_overflow++;
		s_len = CAN_TEXT_MAX_BYTES;
	}

	valid = utf8_valid_len(s_buf, s_len);
	if (valid != s_len)
	{
		s_stats.trim_utf8++;
		s_len = valid;
	}

	memcpy(s_msg, s_buf, s_len);
	s_msg[s_len] = '\0';
	s_msg_new = true;
	s_stats.msg_count++;

	s_active   = false;
	s_len      = 0U;
	s_next_seq = 0U;
}

/* ── 공개 API ─────────────────────────────────────────────────────────── */

void CanText_Init(void)
{
	s_active   = false;
	s_len      = 0U;
	s_next_seq = 0U;
	s_tick     = HAL_GetTick();
	s_msg[0]   = '\0';
	s_msg_new  = false;
	memset(&s_stats, 0, sizeof(s_stats));
}

void CanText_OnFrame(const CanFrame_t *frame)
{
	uint8_t hdr;
	uint8_t seq;
	uint8_t n;
	bool    last;

	if ((frame == NULL) || (frame->id != CAN_TEXT_RX_ID) || frame->extended)
	{
		return;
	}
	if (frame->dlc < 1U)
	{
		return;   /* 헤더 바이트조차 없음 */
	}

	s_stats.frame_count++;

	/* 앞선 조립이 멈춰 있었다면 먼저 정리 */
	if (s_active && ((frame->timestamp - s_tick) > CAN_TEXT_TIMEOUT_MS))
	{
		reasm_abort(&s_stats.drop_timeout);
	}

	hdr  = frame->data[0];
	last = (hdr & 0x80U) != 0U;
	seq  = (uint8_t) (hdr & 0x7FU);

	if (seq >= CAN_TEXT_MAX_FRAMES)
	{
		reasm_abort(&s_stats.drop_overflow);   /* 규격상 seq는 0~8 */
		return;
	}

	if (seq == 0U)
	{
		/* 새 메시지 시작. 조립 중이던 게 있으면 버린다.
		 * (앞 메시지의 마지막 프레임을 놓쳤을 때 여기서 자동 복구됨) */
		reasm_abort(&s_stats.drop_seq);
		s_active = true;
	}
	else if (!s_active || (seq != s_next_seq))
	{
		/* 순서가 어긋났거나 seq 0을 못 받았다 -> 버리고 다음 seq 0을 기다린다 */
		reasm_abort(&s_stats.drop_seq);
		return;
	}
	else
	{
		/* 정상 연속 프레임 */
	}

	/* 유효 본문 바이트 수. 규격상 DLC는 항상 8이지만 짧게 와도 견디도록 한다. */
	n = (frame->dlc > 1U) ? (uint8_t) (frame->dlc - 1U) : 0U;
	if (n > 7U)
	{
		n = 7U;
	}

	if (((uint16_t) s_len + n) > (uint16_t) CAN_TEXT_RAW_CAPACITY)
	{
		reasm_abort(&s_stats.drop_overflow);
		return;
	}

	memcpy(&s_buf[s_len], &frame->data[1], n);
	s_len = (uint8_t) (s_len + n);
	s_tick = frame->timestamp;

	if (last)
	{
		reasm_finish();
	}
	else
	{
		s_next_seq = (uint8_t) (seq + 1U);
		if (s_next_seq >= CAN_TEXT_MAX_FRAMES)
		{
			/* seq 8이 마지막인데 종료 비트가 없다 -> 규격 위반 */
			reasm_abort(&s_stats.drop_overflow);
		}
	}
}

bool CanText_Poll(char *out, uint16_t out_size)
{
	uint16_t len;

	/* 조립이 중간에 끊긴 경우, 다음 프레임을 기다리지 않고 여기서 정리 */
	if (s_active && ((HAL_GetTick() - s_tick) > CAN_TEXT_TIMEOUT_MS))
	{
		reasm_abort(&s_stats.drop_timeout);
	}

	if ((out == NULL) || (out_size == 0U) || !s_msg_new)
	{
		return false;
	}
	s_msg_new = false;

	len = (uint16_t) strlen(s_msg);
	if (len > (uint16_t) (out_size - 1U))
	{
		len = (uint16_t) (out_size - 1U);
	}
	memcpy(out, s_msg, len);
	out[len] = '\0';

	return true;
}

void CanText_GetStats(CanTextStats_t *out)
{
	if (out != NULL)
	{
		*out = s_stats;
	}
}

/* 수신 프레임 디스패치(CanBus_OnFrame)는 can_app.c에 있습니다.
 * 0x500 텍스트 외에 0x501 차량 텔레메트리가 생기면서 디스패처를 한곳에 모았습니다.
 * 계기판 -> 보드 회신은 0x510(명령)으로 can_app.c가 보냅니다. */
