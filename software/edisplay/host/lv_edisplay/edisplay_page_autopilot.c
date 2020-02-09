/*
 *  Copyright (c) 2020 Manuel Bouyer.
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

#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/time.h>
#include "lvgl/lvgl.h"
#include "edisplay_pages.h"
#include "edisplay_font.h"
#include "hal.h"

static lv_obj_t *cap_value;
static lv_obj_t *autocap_value;
static lv_obj_t *roll_value;
static lv_obj_t *capf0_value;
static lv_obj_t *vittf0_value;

static lv_task_t *set_attitude_task;

static void edisp_create_autopilot(void);

edisp_page_t epage_autopilot = {
	edisp_create_autopilot,
	activate_page,
	"cap/auto",
	true,
	NULL
};

#define edisp_page (epage_autopilot.epage_page)

static void
edisp_set_attitude_timeout(lv_task_t *task)
{
	lv_label_set_text(cap_value, "---" DEGSTR);
	lv_label_set_text(roll_value, "---" DEGSTR);
}

void
edisp_set_attitude(int cap, int roll)
{
	char buf[6];
	snprintf(buf, 6, "%3d" DEGSTR, cap);
	lv_label_set_text(cap_value, buf);

	snprintf(buf, 6, "%3d" DEGSTR, roll);
	lv_label_set_text(roll_value, buf);

	lv_task_reset(set_attitude_task);
}

static void
edisp_autopilot_action(lv_obj_t * obj, lv_event_t event)
{
	        /* switch(event) {
		} */
		printf("autopilot event ");
		print_ev(event);
		printf("\n");
}

static void
edisp_create_autopilot()
{
	cap_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(cap_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(cap_value, "---" DEGSTR);
	int w = lv_obj_get_width(cap_value);
	int h = lv_obj_get_height(cap_value);
	lv_obj_align(cap_value, NULL, LV_ALIGN_OUT_TOP_LEFT, 0, h + 10);

	lv_obj_t *autocap_label = lv_label_create(edisp_page, NULL);
	lv_label_set_style(autocap_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(autocap_label, "auto");
	autocap_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(autocap_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(autocap_value, "---" DEGSTR);
	w = lv_obj_get_width(autocap_value);
	h = lv_obj_get_height(autocap_value);
	lv_obj_align(autocap_label, cap_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_align(autocap_value, autocap_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	lv_obj_t *roll_label = lv_label_create(edisp_page, NULL);
	lv_label_set_style(roll_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(roll_label, "gite");
	roll_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(roll_value, LV_LABEL_STYLE_MAIN, &style_medium_text);
	lv_label_set_text(roll_value, "---" DEGSTR);
	w = lv_obj_get_width(roll_value);
	h = lv_obj_get_height(roll_value);
	lv_obj_align(roll_label, cap_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
	lv_obj_align(roll_value, roll_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	capf0_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(capf0_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(capf0_value, "---" DEGSTR);
	w = lv_obj_get_width(capf0_value);
	h = lv_obj_get_height(capf0_value);
	lv_obj_align(capf0_value, roll_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

	lv_obj_t *fond_label = lv_label_create(edisp_page, NULL);
	lv_label_set_style(fond_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(fond_label, "fond");

	vittf0_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(vittf0_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(vittf0_value, "---n");
	w = lv_obj_get_width(vittf0_value);
	h = lv_obj_get_height(vittf0_value);
	lv_obj_align(fond_label, capf0_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_align(vittf0_value, fond_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	lv_obj_set_click(edisp_page, 1);
	lv_obj_set_event_cb(edisp_page, edisp_autopilot_action);

	set_attitude_task = lv_task_create(
	    edisp_set_attitude_timeout, 2000, LV_TASK_PRIO_MID, NULL);
}

void
edisp_autopilot_set_cogsog(const char *cog, const char *sog)
{
	lv_label_set_text(capf0_value, cog);
	lv_label_set_text(vittf0_value, sog);
}
