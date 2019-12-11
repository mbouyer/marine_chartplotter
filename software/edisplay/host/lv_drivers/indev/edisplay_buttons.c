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

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif
#include "../display/edisplay.h"
#include "edisplay_buttons.h"

#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <err.h>
#include <unistd.h>

static pthread_mutex_t edispi_mtx;
static int edisp_buttons_ev;
static int edisp_enc_diff;

static pthread_t edispi_t;
static void * edisp_input(void *);

void
edisplay_buttons_init(void)
{
	if (pthread_mutex_init(&edispi_mtx, NULL) != 0) {
		err(1, "edisplayi mutex init failed");
	}
	edisp_buttons_ev = 0;
	edisp_enc_diff = 0;
	if (pthread_create(&edispi_t, NULL, edisp_input, NULL) != 0) {
		err(1, "pthread_create(edisp_input) failed");
	}
}

#define USB_EP 1

static void *
edisp_input(void *a)
{
	edisplay_ctx_t *ctx;
	int read, r;
	signed char buf[64];

	while (1) {
		ctx = edisplay_get();
		if (ctx == NULL) {
			sleep(1);
			continue;
		}
		read =  0;
		r = libusb_interrupt_transfer(ctx->edctx_dev,
		    (USB_EP | LIBUSB_ENDPOINT_IN), buf, 64, &read, 0);
		if (r != 0) {
			warnx("libusb_interrupt_transfer IN: error %d", r);
			if (r == LIBUSB_ERROR_NO_DEVICE)
				ctx->edctx_fail++;
		}
		edisplay_put(ctx);
		if (read > 0) {
			pthread_mutex_lock(&edispi_mtx);
			if (read == 2)
				edisp_enc_diff += (int)buf[1];
			edisp_buttons_ev = buf[0];
			pthread_mutex_unlock(&edispi_mtx);
		}
	}
}

bool
edisplay_buttons_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
	static int last_button = 0;

	data->state = LV_INDEV_STATE_REL;
	pthread_mutex_lock(&edispi_mtx);
	for (int i = 0; i < 4; i++) {
		if (edisp_buttons_ev & (1 << i)) {
			last_button = i;
			data->state = LV_INDEV_STATE_PR;
			break;
		}
	}
	data->btn_id = last_button;
	pthread_mutex_unlock(&edispi_mtx);
	return false;
}

bool
edisplay_encoder_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
	static int last_button = 0;
	pthread_mutex_lock(&edispi_mtx);
	if (edisp_buttons_ev & (1 << 4)) {
		data->state = LV_INDEV_STATE_PR;
	} else {
		data->state = LV_INDEV_STATE_REL;
	}
	data->enc_diff += edisp_enc_diff;
	edisp_enc_diff = 0;
	pthread_mutex_unlock(&edispi_mtx);
	return false;
}

