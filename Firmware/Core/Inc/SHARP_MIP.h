/**
 * @file SHARP_MIP.h
 *
 */

#ifndef SHARP_MIP_H
#define SHARP_MIP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifndef LV_DRV_NO_CONF
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_drv_conf.h"
#else
#include "lv_drv_conf.h"
#endif
#endif

#if USE_SHARP_MIP

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl.h"
#endif


/*********************
 *      DEFINES
 *********************/

/* 패널 물리 해상도 (LS044Q7DH01 고정, LVGL 논리 해상도와 무관) */
#define SHARP_MIP_PANEL_HOR   320
#define SHARP_MIP_PANEL_VER   240

/* 화면 회전 설정: 0 = 회전 없음(320x240 가로),
 * 90 = 시계방향 90도, 270 = 반시계방향 90도 (둘 다 LVGL은 240x320 세로).
 * 화면이 위아래 뒤집혀 보이면 90 <-> 270으로 바꾸세요. */
#define SHARP_MIP_ROTATION    90

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void sharp_mip_init(void);
void sharp_mip_clear(void);
void sharp_mip_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
void sharp_mip_rounder(lv_disp_drv_t * disp_drv, lv_area_t * area);
void sharp_mip_set_px(lv_disp_drv_t * disp_drv, uint8_t * buf, lv_coord_t buf_w, lv_coord_t x, lv_coord_t y, lv_color_t color, lv_opa_t opa);

/* DMA 경로 지원 (SHARP_MIP_USE_DMA = 0 이면 전부 no-op) */
void sharp_mip_tx_complete_isr(void);      /* HAL_SPI_TxCpltCallback에서 호출 */
void sharp_mip_service(void);              /* 메인 루프에서 매 회 호출 (워치독) */
uint32_t sharp_mip_get_dma_timeouts(void); /* 진단용, 정상이면 0 */
#if SHARP_MIP_SOFT_COM_INVERSION
void sharp_mip_com_inversion(void);
#endif

/**********************
 *      MACROS
 **********************/

#endif /* USE_SHARP_MIP */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* SHARP_MIP_H */
