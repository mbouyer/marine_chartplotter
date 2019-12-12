/*
 *  Copyright (c) 2019 Manuel Bouyer.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <unistd.h>
#include "lvgl/lvgl.h"
#include "lv_drivers/display/edisplay.h"
#include "lv_drivers/indev/edisplay_buttons.h"
#include "hal.h"

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
void hal_init(lv_group_t *encg)
{
    edisplay_init();

    /*Create a display buffer*/
    static lv_disp_buf_t disp_buf1;
    static lv_color_t buf1_1[LV_HOR_RES_MAX*LV_VER_RES_MAX];
    lv_disp_buf_init(&disp_buf1, buf1_1, NULL, LV_HOR_RES_MAX*LV_VER_RES_MAX);

    /*Create a display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.buffer = &disp_buf1;
    disp_drv.flush_cb = edisplay_flush;
    disp_drv.set_px_cb = edisplay_set_px;
    disp_drv.rounder_cb = edisplay_rounder;
    lv_disp_drv_register(&disp_drv);

    /* add buttons as input device */
    edisplay_buttons_init();

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_BUTTON;
    indev_drv.read_cb = edisplay_buttons_read;
    lv_indev_t * buttons_indev = lv_indev_drv_register(&indev_drv);

    static const lv_point_t points_array[] = {
	{2, 2},
	{2, LV_VER_RES_MAX -2},
	{LV_HOR_RES_MAX - 2, LV_VER_RES_MAX -2},
	{LV_HOR_RES_MAX -2, 2},
    };
    lv_indev_set_button_points(buttons_indev, points_array);

    lv_indev_drv_t indev_enc_drv;
    lv_indev_drv_init(&indev_enc_drv);
    indev_enc_drv.type = LV_INDEV_TYPE_ENCODER;
    indev_enc_drv.read_cb = edisplay_encoder_read;
    lv_indev_t * indev_enc = lv_indev_drv_register(&indev_enc_drv);
    lv_indev_set_group(indev_enc, encg);
}

void
hal_set_backlight(bool on, bool inv, int dimm)
{
	edisplay_set_backlight(on, inv, dimm);
}
