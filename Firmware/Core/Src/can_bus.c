/**
  ******************************************************************************
  * @file    can_bus.c
  * @brief   FDCAN1 통신 계층 (뼈대) - 구현
  ******************************************************************************
  */

#include "can_bus.h"
#include "main.h"
#include "vehicle_data.h"
#include <string.h>

/* FDCAN 핸들은 CubeMX 규칙대로 main.c에 있습니다 */
extern FDCAN_HandleTypeDef hfdcan1;

/* ── 수신 링버퍼 ──────────────────────────────────────────────────────── */
static CanFrame_t s_queue[CAN_RX_QUEUE_SIZE];
static volatile uint16_t s_head = 0;   /* ISR이 쓰는 위치 */
static volatile uint16_t s_tail = 0;   /* 메인이 읽는 위치 */

static volatile CanStats_t s_stats = {0};
static bool s_started = false;

/* Bus-Off 복구 재시도 간격. 매 루프 Stop/Start를 때리지 않도록 간격을 둡니다. */
#define CAN_BUSOFF_RETRY_MS   200U
static uint32_t s_busoff_tick = 0;

/* STM32U5 HAL은 헤더 구조체의 DataLength에 DLC '코드'를 그대로 담습니다.
 * (코드 0~8 = 0~8바이트, 9 이상은 CAN FD 전용 12~64바이트)
 * Classic CAN만 쓰므로 8바이트로 클램프합니다. */
static uint8_t CanBus_DlcToLen(uint32_t dlc_code)
{
	return (dlc_code > 8U) ? 8U : (uint8_t) dlc_code;
}

static uint32_t CanBus_LenToDlc(uint8_t len)
{
	return (uint32_t) len;
}

/* ── 초기화 ──────────────────────────────────────────────────────────── */

void CanBus_Init(void)
{
	FDCAN_FilterTypeDef filter = {0};

	/* 표준 ID 전체 통과 -> Rx FIFO0
	 * (특정 ID만 받으려면 FilterType을 FDCAN_FILTER_MASK로 두고
	 *  FilterID1 = ID, FilterID2 = 마스크 로 바꾸면 됩니다) */
	filter.IdType       = FDCAN_STANDARD_ID;
	filter.FilterIndex  = 0;
	filter.FilterType   = FDCAN_FILTER_MASK;
	filter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	filter.FilterID1    = 0x000;
	filter.FilterID2    = 0x000;   /* 마스크 0 = 모든 ID 통과 */
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK)
	{
		Error_Handler();
	}

	/* 확장 ID 전체 통과 -> Rx FIFO0 */
	filter.IdType      = FDCAN_EXTENDED_ID;
	filter.FilterIndex = 0;
	filter.FilterID1   = 0x00000000UL;
	filter.FilterID2   = 0x00000000UL;
	if (HAL_FDCAN_ConfigFilter(&hfdcan1, &filter) != HAL_OK)
	{
		Error_Handler();
	}

	/* 필터에 걸리지 않은 프레임은 버림 */
	if (HAL_FDCAN_ConfigGlobalFilter(&hfdcan1,
	                                 FDCAN_REJECT, FDCAN_REJECT,
	                                 FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK)
	{
		Error_Handler();
	}

	if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK)
	{
		Error_Handler();
	}

	/* Rx FIFO0 새 메시지 + 버스 에러 인터럽트 */
	if (HAL_FDCAN_ActivateNotification(&hfdcan1,
	                                   FDCAN_IT_RX_FIFO0_NEW_MESSAGE |
	                                   FDCAN_IT_BUS_OFF, 0) != HAL_OK)
	{
		Error_Handler();
	}

	s_started = true;
}

/* ── 송신 ────────────────────────────────────────────────────────────── */

bool CanBus_Send(uint32_t id, bool ext, const uint8_t *data, uint8_t len)
{
	FDCAN_TxHeaderTypeDef tx = {0};
	uint8_t payload[8] = {0};

	if (!s_started || len > 8U)
	{
		return false;
	}
	if (len > 0U && data != NULL)
	{
		memcpy(payload, data, len);
	}

	tx.Identifier          = ext ? (id & 0x1FFFFFFFUL) : (id & 0x7FFUL);
	tx.IdType              = ext ? FDCAN_EXTENDED_ID : FDCAN_STANDARD_ID;
	tx.TxFrameType         = FDCAN_DATA_FRAME;
	tx.DataLength          = CanBus_LenToDlc(len);
	tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	tx.BitRateSwitch       = FDCAN_BRS_OFF;      /* Classic CAN */
	tx.FDFormat            = FDCAN_CLASSIC_CAN;
	tx.TxEventFifoControl  = FDCAN_NO_TX_EVENTS;
	tx.MessageMarker       = 0;

	if (HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) == 0U)
	{
		return false;   /* Tx FIFO 가득 참 */
	}
	if (HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx, payload) != HAL_OK)
	{
		return false;
	}

	s_stats.tx_count++;
	return true;
}

/* ── 수신 ────────────────────────────────────────────────────────────── */

bool CanBus_Receive(CanFrame_t *out)
{
	if (out == NULL || s_tail == s_head)
	{
		return false;
	}

	__DMB();   /* ISR이 채운 슬롯 내용이 보이도록 */
	*out = s_queue[s_tail];
	s_tail = (uint16_t) ((s_tail + 1U) & (CAN_RX_QUEUE_SIZE - 1U));
	return true;
}

void CanBus_Process(void)
{
	CanFrame_t frame;

	while (CanBus_Receive(&frame))
	{
		CanBus_OnFrame(&frame);
	}

	/* Bus-Off 자동 복구
	 *
	 * Bus-Off가 되면 FDCAN 하드웨어가 스스로 CCCR.INIT을 세우고 버스 활동을
	 * 멈춥니다. 복구 절차(129 x 11 recessive bit 대기)는 소프트웨어가 INIT을
	 * 내려 줘야 시작됩니다. 그런데 HAL_FDCAN_Start()는 State가 READY일 때만
	 * 동작하고, CanBus_Init() 이후 State는 계속 BUSY라 그냥 부르면 HAL_ERROR만
	 * 납니다. 그래서 HAL_FDCAN_Stop()으로 State를 READY로 되돌린 뒤 Start합니다.
	 * (Stop/Start는 IE/ILS와 메시지 RAM의 필터를 건드리지 않으므로
	 *  인터럽트 설정과 필터는 그대로 유지됩니다) */
	if (s_stats.bus_off && s_started)
	{
		FDCAN_ProtocolStatusTypeDef st;
		HAL_FDCAN_GetProtocolStatus(&hfdcan1, &st);

		if (st.BusOff == 0U)
		{
			s_stats.bus_off = false;   /* 복구 완료 */
		}
		else if ((HAL_GetTick() - s_busoff_tick) >= CAN_BUSOFF_RETRY_MS)
		{
			s_busoff_tick = HAL_GetTick();
			if (HAL_FDCAN_Stop(&hfdcan1) == HAL_OK)
			{
				(void) HAL_FDCAN_Start(&hfdcan1);
			}
		}
	}
}

void CanBus_GetStats(CanStats_t *out)
{
	if (out != NULL)
	{
		*out = *(const CanStats_t *) &s_stats;
	}
}

/* ── HAL 콜백 (ISR 컨텍스트) ─────────────────────────────────────────── */

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	FDCAN_RxHeaderTypeDef rx;
	/* ⚠ 반드시 64바이트여야 합니다.
	 * HAL_FDCAN_GetRxMessage()는 수신된 raw DLC 코드(0~15)로 내부
	 * DLCtoBytes[] = {0,1,..,8,12,16,20,24,32,48,64} 를 인덱싱해 그만큼 복사합니다.
	 * Classic CAN 2.0에서도 DLC 필드 9~15는 버스에서 합법이므로(규격상 8바이트로
	 * 해석) 8바이트 버퍼를 넘기면 최대 64바이트가 기록되어 ISR 스택이 깨집니다.
	 * CanBus_DlcToLen()의 8 클램프는 복사가 끝난 뒤라 방어가 되지 않습니다. */
	uint8_t payload[64];

	if (hfdcan->Instance != FDCAN1)
	{
		return;
	}
	if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) == 0U)
	{
		return;
	}

	/* FIFO가 빌 때까지 모두 꺼낸다 (인터럽트 누락 대비) */
	while (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0) > 0U)
	{
		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx, payload) != HAL_OK)
		{
			break;
		}

		uint16_t next = (uint16_t) ((s_head + 1U) & (CAN_RX_QUEUE_SIZE - 1U));
		if (next == s_tail)
		{
			s_stats.rx_overflow++;   /* 큐 가득 참 - 가장 최근 프레임을 버림 */
			continue;
		}

		CanFrame_t *slot = &s_queue[s_head];
		slot->id        = rx.Identifier;
		slot->extended  = (rx.IdType == FDCAN_EXTENDED_ID);
		slot->dlc       = CanBus_DlcToLen(rx.DataLength);
		slot->timestamp = HAL_GetTick();
		/* HAL은 DLC 길이만큼만 payload를 채웁니다. 무조건 8바이트를 복사하면
		 * 짧은 프레임에서 이전 ISR 호출의 스택 잔여값이 그대로 실려 나갑니다.
		 * 유효 구간만 복사하고 나머지는 0으로 채웁니다. */
		memcpy(slot->data, payload, slot->dlc);
		if (slot->dlc < 8U)
		{
			memset(&slot->data[slot->dlc], 0, (size_t) (8U - slot->dlc));
		}

		__DMB();   /* 슬롯 내용이 먼저 보이게 한 뒤 head 갱신 */
		s_head = next;
		s_stats.rx_count++;
	}
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
	if (hfdcan->Instance != FDCAN1)
	{
		return;
	}

	if ((ErrorStatusITs & FDCAN_IT_BUS_OFF) != 0U)
	{
		s_stats.bus_off = true;
		s_stats.error_count++;
	}
}

/* ── 수신 해석 훅 (여기부터 채우면 됩니다) ───────────────────────────── */

/**
  * @brief  수신 프레임 1개를 해석해서 vehicle_data 구조체에 반영
  * @note   __weak 이므로 다른 .c 파일에서 같은 시그니처로 다시 정의하면
  *         그쪽 구현이 링크됩니다. 여기 직접 채워도 됩니다.
  *
  *         아래는 형태를 보여주는 예시입니다. 실제 DBC/ID가 정해지면
  *         case 라벨과 바이트 오프셋만 바꾸면 됩니다.
  */
__weak void CanBus_OnFrame(const CanFrame_t *frame)
{
	switch (frame->id)
	{
	/* 예시) 0x100 : 주행 정보
	 * case 0x100:
	 *     if (frame->dlc >= 4)
	 *     {
	 *         DrivingData.Speed        = frame->data[0];
	 *         DrivingData.Motor_Power  = frame->data[1];
	 *         DrivingData.MPPT_Power   = frame->data[2];
	 *         DrivingData.Battery_SOC  = frame->data[3];
	 *     }
	 *     break;
	 */

	/* 예시) 0x200 : 등화/경고
	 * case 0x200:
	 *     if (frame->dlc >= 1)
	 *     {
	 *         LightData.Left_Signal   = (frame->data[0] & 0x01) != 0;
	 *         LightData.Right_Signal  = (frame->data[0] & 0x02) != 0;
	 *         LightData.Hazard_Signal = (frame->data[0] & 0x04) != 0;
	 *     }
	 *     break;
	 */

	default:
		break;
	}

	(void) frame;
}
