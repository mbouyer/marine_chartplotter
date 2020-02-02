/**
 * @file lv_drv_conf.h
 *
 */

/*
 * COPY THIS FILE AS lv_drv_conf.h
 */

#if 1 /*Set it to "1" to enable the content*/

#ifndef LV_DRV_CONF_H
#define LV_DRV_CONF_H

#include "lv_conf.h"

/*********************
 * DELAY INTERFACE
 *********************/
#define LV_DRV_DELAY_INCLUDE  <stdint.h>            /*Dummy include by default*/
#define LV_DRV_DELAY_US(us)  /*delay_us(us)*/       /*Delay the given number of microseconds*/
#define LV_DRV_DELAY_MS(ms)  /*delay_ms(ms)*/       /*Delay the given number of milliseconds*/

/*********************
 *  DISPLAY DRIVERS
 *********************/

/*-------------------
 *  Monitor of PC
 *-------------------*/
#ifndef USE_MONITOR
#  define USE_MONITOR         1
#endif

#if USE_MONITOR
#  define MONITOR_HOR_RES     LV_HOR_RES
#  define MONITOR_VER_RES     LV_VER_RES

/* Scale window by this factor (useful when simulating small screens) */
#  define MONITOR_ZOOM        2

/* Used to test true double buffering with only address changing.
 * Set LV_VDB_SIZE = (LV_HOR_RES * LV_VER_RES) and  LV_VDB_DOUBLE = 1 and LV_COLOR_DEPTH = 32" */
#  define MONITOR_DOUBLE_BUFFERED 0

/*Eclipse: <SDL2/SDL.h>    Visual Studio: <SDL.h>*/
#  define MONITOR_SDL_INCLUDE_PATH    <SDL2/SDL.h>

/*Different rendering might be used if running in a Virtual machine*/
#  define MONITOR_VIRTUAL_MACHINE 1

/*Open two windows to test multi display support*/
#  define MONITOR_DUAL            0
#endif


/*---------------------------------------
 * Mouse or touchpad on PC (using SDL)
 *-------------------------------------*/
#ifndef USE_MOUSE
#  define USE_MOUSE           1
#endif

#if USE_MOUSE
/*No settings*/
#endif

/*-------------------------------------------
 * Mousewheel as encoder on PC (using SDL)
 *------------------------------------------*/
#ifndef USE_MOUSEWHEEL
#  define USE_MOUSEWHEEL      1
#endif

#if USE_MOUSEWHEEL
/*No settings*/
#endif

#endif  /*LV_DRV_CONF_H*/

#endif /*End of "Content enable"*/
