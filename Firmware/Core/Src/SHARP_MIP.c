/**
 * @file SHARP_MIP.c
 *
 */

#include "SHARP_MIP.h"

#if USE_SHARP_MIP

#include <stdbool.h>
#include "main.h"   /* HAL, hspi1, SPI1_SCS pin */
#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE

/*********************
 * DEFINES
 *********************/

/* SPI가 LSB-first이므로 커맨드 비트도 LSB 기준입니다.
 * (M0=bit0=0x01 write, M1=bit1=0x02 VCOM, M2=bit2=0x04 clear)
 * 게이트 주소/픽셀 비트도 반전 없이 그대로 보내면 됩니다
 * (lv_drv_conf.h의 SHARP_MIP_REV_BYTE(b)=(b) 그대로 유지). */
#define SHARP_MIP_HEADER              0
#define SHARP_MIP_UPDATE_RAM_FLAG     (1 << 0)
#define SHARP_MIP_COM_INVERSION_FLAG  (1 << 1)
#define SHARP_MIP_CLEAR_SCREEN_FLAG   (1 << 2)

/* 1 = GPDMA 비동기 전송 (완료는 HAL_SPI_TxCpltCallback -> sharp_mip_tx_complete_isr)
 * 0 = 블로킹 전송 (문제 분리/디버깅용 - 확실히 동작하는 경로)
 *
 * DMA 모드에서 완료 인터럽트가 유실되면 LVGL이 flush 상태로 영구히 멈추므로,
 * sharp_mip_service()가 메인 루프에서 타임아웃을 감시해 강제 복구합니다.
 * 문제가 재발하면 sharp_mip_get_dma_timeouts()가 0보다 커집니다. */
#define SHARP_MIP_USE_DMA  1

/**********************
 * STATIC VARIABLES
 **********************/

#if SHARP_MIP_SOFT_COM_INVERSION
static bool com_output_state = false;
#endif

/**********************
 * MACROS
 **********************/

/* 버퍼 인덱스는 항상 '패널' 좌표(320x240) 기준 */
#define BUFIDX(x, y)  (((x) >> 3) + ((y) * (2 + (SHARP_MIP_PANEL_HOR >> 3))) + 2)
#define PIXIDX(x)     SHARP_MIP_REV_BYTE(1 << ((x) & 7))

/**********************
 * GLOBAL VARIABLES (DMA용)
 **********************/

/* main.c에 선언된 SPI 핸들을 가져옵니다. */
extern SPI_HandleTypeDef hspi1;

/* DMA 인터럽트 발생 시 사용할 LVGL 디스플레이 드라이버 포인터 */
lv_disp_drv_t * active_disp_drv = NULL;

#if SHARP_MIP_USE_DMA
/* 전송 감시용 상태.
 * 전체화면 10,082바이트를 2MHz로 보내면 약 40ms 걸리므로
 * 타임아웃은 넉넉히 3배 이상으로 잡습니다. */
#define SHARP_MIP_DMA_TIMEOUT_MS  150U

static volatile bool     dma_busy       = false;
static volatile uint32_t dma_start_tick = 0;
static volatile uint32_t dma_timeouts   = 0;   /* 진단용: 유실된 완료 인터럽트 수 */
#endif

/**********************
 * GLOBAL FUNCTIONS
 **********************/

/* 대략적인 마이크로초 지연 (160MHz, 루프당 ~4사이클) */
static void sharp_mip_delay_us(uint32_t us) {
  volatile uint32_t n = us * 40U;
  while (n--) { }
}

void sharp_mip_init(void) {
  /* These displays have nothing to initialize */
}

/**
 * @brief 패널 내부 메모리 전체 클리어 (전원 인가 후 랜덤 픽셀 제거용)
 * @note  블로킹 SPI로 2바이트만 보내므로 LVGL 초기화 전에 호출해도 안전
 */
void sharp_mip_clear(void) {
  uint8_t clear_cmd[2] = { SHARP_MIP_CLEAR_SCREEN_FLAG, 0x00 };

  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_SET);
  sharp_mip_delay_us(6);                       /* tsSCS  >= 6us */
  HAL_SPI_Transmit(&hspi1, clear_cmd, 2, HAL_MAX_DELAY);
  sharp_mip_delay_us(2);                       /* thSCS  >= 2us */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);
  sharp_mip_delay_us(6);                       /* twSCSL >= 6us */
}

void sharp_mip_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
#if SHARP_MIP_ROTATION == 90 || SHARP_MIP_ROTATION == 270
  /* 회전 모드: rounder가 항상 전체 화면을 요청하므로 area(LVGL 세로 좌표)는
   * 무시하고 패널 전체 라인(0~239)을 전송한다. */
  (void)area;
  uint16_t act_y1 = 0;
  uint16_t act_y2 = SHARP_MIP_PANEL_VER - 1;
#else
  /*Return if the area is out the screen*/
  if(area->y2 < 0) return;
  if(area->y1 > SHARP_MIP_PANEL_VER - 1) return;

  /*Truncate the area to the screen*/
  uint16_t act_y1 = area->y1 < 0 ? 0 : area->y1;
  uint16_t act_y2 = area->y2 > SHARP_MIP_PANEL_VER - 1 ? SHARP_MIP_PANEL_VER - 1 : area->y2;
#endif

  uint8_t * buf      = (uint8_t *) color_p;                       /*Get the buffer address*/
  uint16_t  buf_h    = (act_y2 - act_y1 + 1);                     /*Number of buffer lines*/
  uint16_t  buf_size = buf_h * (2 + SHARP_MIP_PANEL_HOR / 8) + 2; /*Buffer size in bytes  */

  /* Set lines to flush dummy byte & gate address in draw_buf*/
  for(uint16_t act_y = 0 ; act_y < buf_h ; act_y++) {
    buf[BUFIDX(0, act_y) - 1] = SHARP_MIP_REV_BYTE((act_y1 + act_y + 1));
    buf[BUFIDX(0, act_y) - 2] = 0;
  }

  /* Set last dummy two bytes in draw_buf */
  buf[BUFIDX(0, buf_h) - 1] = 0;
  buf[BUFIDX(0, buf_h) - 2] = 0;

  /* Set frame header in draw_buf */
  buf[0] = SHARP_MIP_HEADER         |
           SHARP_MIP_UPDATE_RAM_FLAG;

#if SHARP_MIP_USE_DMA
  /* --- GPDMA 전송 시작 --- */

  /* 전송 완료 콜백에서 사용할 수 있도록 드라이버 주소 백업 */
  active_disp_drv = disp_drv;

  /* CS 핀 활성화 (HIGH) 후 셋업타임 확보 (tsSCS >= 6us) */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_SET);
  sharp_mip_delay_us(6);

  /* 전송 감시 시작 (완료 인터럽트 유실 대비) */
  dma_start_tick = HAL_GetTick();
  dma_busy = true;

  /* SPI DMA 전송 (완료 처리는 sharp_mip_tx_complete_isr에서 수행) */
  if (HAL_SPI_Transmit_DMA(&hspi1, buf, buf_size) != HAL_OK) {
    /* DMA 시작 실패 시 블로킹 전송으로 폴백 (LVGL이 멈추지 않도록) */
    dma_busy = false;
    HAL_SPI_Transmit(&hspi1, buf, buf_size, HAL_MAX_DELAY);
    sharp_mip_delay_us(2);
    HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);
    active_disp_drv = NULL;
    lv_disp_flush_ready(disp_drv);
  }
#else
  /* --- 블로킹 전송 (DMA 미사용) --- */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_SET);
  sharp_mip_delay_us(6);                       /* tsSCS  >= 6us */
  HAL_SPI_Transmit(&hspi1, buf, buf_size, HAL_MAX_DELAY);
  sharp_mip_delay_us(2);                       /* thSCS  >= 2us */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);
  sharp_mip_delay_us(6);                       /* twSCSL >= 6us */
  lv_disp_flush_ready(disp_drv);
#endif
}

/**
 * @brief  SPI 전송 완료 처리 (ISR 컨텍스트)
 * @note   main.c의 HAL_SPI_TxCpltCallback()에서 호출하세요.
 *         DMA를 쓰지 않는 빌드에서는 아무 일도 하지 않습니다.
 */
void sharp_mip_tx_complete_isr(void) {
#if SHARP_MIP_USE_DMA
  if (!dma_busy) {
    return;                 /* 이미 워치독이 처리했거나 우리 전송이 아님 */
  }
  dma_busy = false;

  /* SCS 하강 전 홀드타임 확보 (thSCS >= 2us, 160MHz 기준 대략 4us) */
  for (volatile uint32_t i = 0; i < 160; i++) { }
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);

  if (active_disp_drv != NULL) {
    lv_disp_flush_ready(active_disp_drv);
    active_disp_drv = NULL;
  }
#endif
}

/**
 * @brief  전송 워치독. 메인 루프에서 매 회 호출하세요.
 * @note   완료 인터럽트가 유실되면 active_disp_drv가 계속 남아 LVGL이
 *         flush 대기 상태로 영구히 멈춥니다. 그 경우를 타임아웃으로 잡아
 *         SPI/DMA를 정리하고 LVGL을 풀어줍니다.
 *         -> 최악의 경우가 '영구 정지'가 아니라 '한 프레임 깨짐'이 됩니다.
 */
void sharp_mip_service(void) {
#if SHARP_MIP_USE_DMA
  if (!dma_busy) {
    return;
  }
  if ((HAL_GetTick() - dma_start_tick) < SHARP_MIP_DMA_TIMEOUT_MS) {
    return;
  }

  /* 소유권 확보: 검사와 동시에 ISR이 끼어들 수 있으므로 짧게 잠근다 */
  __disable_irq();
  bool takeover = dma_busy;
  dma_busy = false;
  __enable_irq();

  if (!takeover) {
    return;                 /* 방금 ISR이 정상 처리함 */
  }

  dma_timeouts++;

  HAL_SPI_Abort(&hspi1);    /* SPI/DMA 상태 정리 후 다음 전송 가능하게 */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_RESET);

  if (active_disp_drv != NULL) {
    lv_disp_flush_ready(active_disp_drv);
    active_disp_drv = NULL;
  }
#endif
}

/**
 * @brief  유실된 완료 인터럽트 횟수 (진단용). 정상이면 계속 0 입니다.
 */
uint32_t sharp_mip_get_dma_timeouts(void) {
#if SHARP_MIP_USE_DMA
  return dma_timeouts;
#else
  return 0;
#endif
}

void sharp_mip_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa) {

  (void) disp_drv;
  (void) buf_w;
  (void) opa;

  /* LVGL 논리 좌표(x,y) -> 패널 물리 좌표(px,py) 변환
   * (회전 모드에서는 rounder가 전체 화면을 강제하므로 x,y가 절대좌표와 동일) */
#if SHARP_MIP_ROTATION == 90
  lv_coord_t px = (SHARP_MIP_PANEL_HOR - 1) - y;   /* 시계방향 90도 */
  lv_coord_t py = x;
#elif SHARP_MIP_ROTATION == 270
  lv_coord_t px = y;                                /* 반시계방향 90도 */
  lv_coord_t py = (SHARP_MIP_PANEL_VER - 1) - x;
#else
  lv_coord_t px = x;
  lv_coord_t py = y;
#endif

  if (lv_color_to1(color) != 0) {
    buf[BUFIDX(px, py)] |=  PIXIDX(px);  /*Set draw_buf pixel bit to 1 for other colors than BLACK*/
  } else {
    buf[BUFIDX(px, py)] &= ~PIXIDX(px);  /*Set draw_buf pixel bit to 0 for BLACK color*/
  }
}

void sharp_mip_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area) {
  (void) disp_drv;

#if SHARP_MIP_ROTATION == 90 || SHARP_MIP_ROTATION == 270
  /* 회전 모드: set_px에서 절대좌표가 필요하므로 항상 전체 화면(세로 240x320)으로 확장 */
  area->x1 = 0;
  area->x2 = SHARP_MIP_PANEL_VER - 1;   /* LVGL 가로 = 240 */
  area->y1 = 0;
  area->y2 = SHARP_MIP_PANEL_HOR - 1;   /* LVGL 세로 = 320 */
#else
  /* Round area to a whole line */
  area->x1 = 0;
  area->x2 = SHARP_MIP_PANEL_HOR - 1;
#endif
}

#if SHARP_MIP_SOFT_COM_INVERSION
void sharp_mip_com_inversion(void) {
  uint8_t inversion_header[2] = {0};

  /* Set inversion header */
  if (com_output_state) {
    com_output_state = false;
  } else {
    inversion_header[0] |= SHARP_MIP_COM_INVERSION_FLAG;
    com_output_state = true;
  }

  /* Write inversion header on display memory */
  LV_DRV_DISP_SPI_CS(1);
  LV_DRV_DISP_SPI_WR_ARRAY(inversion_header, 2);
  LV_DRV_DISP_SPI_CS(0);
}
#endif

#endif
