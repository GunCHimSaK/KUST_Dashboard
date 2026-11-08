/**
  ******************************************************************************
  * @file    can_bus.h
  * @brief   FDCAN1 통신 계층 (뼈대)
  ******************************************************************************
  * 배선
  *   PA11 = FDCAN1_RX, PA12 = FDCAN1_TX (AF9) -> CAN 트랜시버
  *
  * 설정
  *   Classic CAN 2.0B, 500 kbit/s, 샘플포인트 87.5%
  *   커널 클럭 = HSE 16MHz 직결 (분주오차 0)
  *   16MHz / 1 / (1 + 27 + 4)tq = 500 kbit/s
  *
  * 사용법
  *   1) main()에서 MX_FDCAN1_Init() 뒤에 CanBus_Init() 한 번
  *   2) 메인 루프에서 CanBus_Process() 주기 호출
  *   3) 수신 프레임 해석은 CanBus_OnFrame()을 재정의(override)해서 작성
  *      - can_bus.c에 __weak 기본 구현이 있고, 거기 예시 뼈대가 있습니다.
  *   4) 송신은 CanBus_Send()
  *
  * 수신은 FIFO0 인터럽트 -> 링버퍼 적재까지만 ISR에서 하고,
  * 해석은 CanBus_Process()(메인 루프 컨텍스트)에서 합니다.
  * LVGL 호출을 ISR에서 하면 안 되므로 이 구조를 유지하세요.
  ******************************************************************************
  */

#ifndef CAN_BUS_H_
#define CAN_BUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* 수신 링버퍼 크기 (2의 거듭제곱) */
#define CAN_RX_QUEUE_SIZE   32U

/* 수신 프레임 */
typedef struct
{
	uint32_t id;         /* 11bit(표준) 또는 29bit(확장) 식별자 */
	uint32_t timestamp;  /* 수신 시각 HAL_GetTick() */
	uint8_t  data[8];
	uint8_t  dlc;        /* 유효 바이트 수 0~8 */
	bool     extended;   /* true = 29bit 확장 ID */
} CanFrame_t;

/* 버스 상태 통계 */
typedef struct
{
	uint32_t rx_count;      /* 수신 프레임 누적 */
	uint32_t tx_count;      /* 송신 성공 누적 */
	uint32_t rx_overflow;   /* 링버퍼가 꽉 차서 버린 프레임 수 */
	uint32_t error_count;   /* 프로토콜 에러 누적 */
	bool     bus_off;       /* Bus-Off 상태 여부 */
} CanStats_t;

/** FDCAN 필터/인터럽트 설정 후 Start. MX_FDCAN1_Init() 뒤에 호출. */
void CanBus_Init(void);

/** 메인 루프에서 주기 호출. 큐에 쌓인 프레임을 CanBus_OnFrame()으로 넘깁니다.
  * Bus-Off 자동 복구도 여기서 처리합니다. */
void CanBus_Process(void);

/** 프레임 1개 송신 (Tx FIFO에 넣기만 하고 바로 반환)
  * @param id   표준 11bit 또는 확장 29bit 식별자
  * @param ext  true면 확장 ID
  * @param data 보낼 데이터 (len==0 이면 NULL 허용)
  * @param len  0~8
  * @retval true = FIFO 적재 성공 */
bool CanBus_Send(uint32_t id, bool ext, const uint8_t *data, uint8_t len);

/** 큐에서 프레임 1개를 직접 꺼냅니다 (CanBus_Process()를 안 쓸 때).
  * @retval false = 큐가 비어 있음 */
bool CanBus_Receive(CanFrame_t *out);

/** 통계 스냅샷 */
void CanBus_GetStats(CanStats_t *out);

/** 수신 프레임 처리 훅.
  * can_bus.c의 __weak 기본 구현을 그대로 두고, 다른 파일에서 같은 이름으로
  * 다시 정의하면 그쪽이 호출됩니다. */
void CanBus_OnFrame(const CanFrame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* CAN_BUS_H_ */
