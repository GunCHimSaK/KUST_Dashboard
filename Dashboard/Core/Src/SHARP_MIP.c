/**
 * @file SHARP_MIP.c
 *
 */

#include "SHARP_MIP.h"

#if USE_SHARP_MIP

#include <stdbool.h>
#include LV_DRV_DISP_INCLUDE
#include LV_DRV_DELAY_INCLUDE

/*********************
 * DEFINES
 *********************/

#define SHARP_MIP_HEADER              0
#define SHARP_MIP_UPDATE_RAM_FLAG     (1 << 7)
#define SHARP_MIP_COM_INVERSION_FLAG  (1 << 6)
#define SHARP_MIP_CLEAR_SCREEN_FLAG   (1 << 5)

/**********************
 * STATIC VARIABLES
 **********************/

#if SHARP_MIP_SOFT_COM_INVERSION
static bool com_output_state = false;
#endif

/**********************
 * MACROS
 **********************/

#define BUFIDX(x, y)  (((x) >> 3) + ((y) * (2 + (SHARP_MIP_HOR_RES >> 3))) + 2)
#define PIXIDX(x)     SHARP_MIP_REV_BYTE(1 << ((x) & 7))

/**********************
 * GLOBAL VARIABLES (DMA용)
 **********************/

/* main.c에 선언된 SPI 핸들을 가져옵니다. */
extern SPI_HandleTypeDef hspi1;

/* DMA 인터럽트 발생 시 사용할 LVGL 디스플레이 드라이버 포인터 */
lv_disp_drv_t * active_disp_drv = NULL;

/**********************
 * GLOBAL FUNCTIONS
 **********************/

void sharp_mip_init(void) {
  /* These displays have nothing to initialize */
}


void sharp_mip_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {

  /*Return if the area is out the screen*/
  if(area->y2 < 0) return;
  if(area->y1 > SHARP_MIP_VER_RES - 1) return;

  /*Truncate the area to the screen*/
  uint16_t act_y1 = area->y1 < 0 ? 0 : area->y1;
  uint16_t act_y2 = area->y2 > SHARP_MIP_VER_RES - 1 ? SHARP_MIP_VER_RES - 1 : area->y2;

  uint8_t * buf      = (uint8_t *) color_p;                     /*Get the buffer address*/
  uint16_t  buf_h    = (act_y2 - act_y1 + 1);                   /*Number of buffer lines*/
  uint16_t  buf_size = buf_h * (2 + SHARP_MIP_HOR_RES / 8) + 2; /*Buffer size in bytes  */

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

  /* --- GPDMA 전송 시작 --- */

  /* 전송 완료 콜백에서 사용할 수 있도록 드라이버 주소 백업 */
  active_disp_drv = disp_drv;

  /* CS 핀 활성화 (HIGH) */
  HAL_GPIO_WritePin(SPI1_SCS_GPIO_Port, SPI1_SCS_Pin, GPIO_PIN_SET);

  /* SPI DMA 전송 (이후 처리는 main.c의 HAL_SPI_TxCpltCallback에서 수행) */
  HAL_SPI_Transmit_DMA(&hspi1, buf, buf_size);
}

void sharp_mip_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa) {
  (void) disp_drv;
  (void) buf_w;
  (void) opa;

  if (lv_color_to1(color) != 0) {
    buf[BUFIDX(x, y)] |=  PIXIDX(x);  /*Set draw_buf pixel bit to 1 for other colors than BLACK*/
  } else {
    buf[BUFIDX(x, y)] &= ~PIXIDX(x);  /*Set draw_buf pixel bit to 0 for BLACK color*/
  }
}

void sharp_mip_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area) {
  (void) disp_drv;

  /* Round area to a whole line */
  area->x1 = 0;
  area->x2 = SHARP_MIP_HOR_RES - 1;
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
