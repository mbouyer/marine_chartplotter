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
#include "edisplay_data.h"
#include "hal.h"

static lv_obj_t *cap_value;
static lv_obj_t *autocap_value;
static lv_obj_t *roll_value;
static lv_obj_t *capf0_value;
static lv_obj_t *vittf0_value;

static lv_task_t *set_attitude_task;
static lv_task_t *set_auto_task;

static int received_heading;

static uint8_t auto_mode;
#define AUTO_INVALID 0xff
static double auto_heading;
static double auto_slot;

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
	received_heading = AUTO_INVALID;
}

void
edisp_set_attitude(int heading, int roll)
{
	edisp_lvgl_lock();
	char buf[6];
	snprintf(buf, 6, "%3d" DEGSTR, heading);
	lv_label_set_text(cap_value, buf);
	received_heading = heading;

	snprintf(buf, 6, "%3d" DEGSTR, roll);
	lv_label_set_text(roll_value, buf);

	lv_task_reset(set_attitude_task);
	edisp_lvgl_unlock();
}

static void
edisp_set_auto_timeout(lv_task_t *task)
{
	lv_label_set_text(autocap_value, "---" DEGSTR);
	auto_mode = AUTO_INVALID;
	auto_heading = -1;
}

void
edisp_set_auto_status(uint8_t mode, double heading, uint8_t error, uint8_t slot)
{
	char buf[6];
	edisp_lvgl_lock();
	lv_task_reset(set_auto_task);
	/* XXX handle errors */
	if (mode == auto_mode && heading == auto_heading && slot == auto_slot) {
		edisp_lvgl_unlock();
		return; /* nothing changed */
	}
	switch(mode) {
	case AUTO_OFF:
		auto_slot = slot;
		auto_mode = mode;
		auto_heading = -1;
		lv_label_set_text(autocap_value, "OFF ");
		break;
	case AUTO_STANDBY:
		auto_slot = slot;
		auto_mode = mode;
		lv_label_set_text(autocap_value, "STBY");
		break;
	case AUTO_HEAD:
		auto_heading = heading;
		auto_slot = slot;
		auto_mode = mode;
		snprintf(buf, 6, "%3d" DEGSTR, (int)heading);
		lv_label_set_text(autocap_value, buf);
		break;
	}
	edisp_lvgl_unlock();
}

void
edisp_autopilot_startstop(bool active)
{
	switch(auto_mode) {
	case AUTO_OFF:
		if (received_heading >= 0 && active) {
			auto_heading = received_heading;
			if (!n2ks_auto_engage(auto_heading, AUTO_STANDBY,
			    auto_slot))
				printf("n2ks_auto_engage failed\n");
		}
		break;
	case AUTO_STANDBY:
		if (auto_heading > 0 && active) {
			if (!n2ks_auto_engage(auto_heading, AUTO_HEAD,
			    auto_slot))
				printf("n2ks_auto_engage failed\n");
		}
		break;
	case AUTO_HEAD:
		if (!n2ks_auto_engage(auto_heading, AUTO_STANDBY, auto_slot))
			printf("n2ks_auto_engage failed\n");
		break;
	}
}

static void
edisp_autopilot_action(lv_obj_t * obj, lv_event_t event)
{
	int key;
	double new_head;
	switch(event) {
	case LV_EVENT_KEY:
		key = *((uint32_t *)lv_event_get_data());
		switch(auto_mode) {
		case AUTO_STANDBY:
			switch(key) {
			case LV_KEY_RIGHT:
				n2ks_auto_acuator(1);
				break;
			case LV_KEY_LEFT:
				n2ks_auto_acuator(-1);
				break;
			}
			break;
		case AUTO_HEAD:
			new_head;
			if (auto_heading < 0)
				return;
			switch(key) {
			case LV_KEY_RIGHT:
				new_head = auto_heading + 1;
				if (new_head >= 360)
					new_head -= 360;
				break;
			case LV_KEY_LEFT:
				new_head = auto_heading - 1;
				if (new_head < 0)
					new_head += 360;
				break;
			}
			n2ks_auto_engage(new_head, AUTO_HEAD, auto_slot);
		}
		return;
	}
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
	lv_obj_align(cap_value, NULL, LV_ALIGN_OUT_TOP_LEFT, 5, h + 10);
	received_heading = AUTO_INVALID;

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
	auto_mode = AUTO_INVALID;

	lv_obj_t *roll_label = lv_label_create(edisp_page, NULL);
	lv_label_set_style(roll_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(roll_label, "gite");
	roll_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(roll_value, LV_LABEL_STYLE_MAIN, &style_medium_text);
	lv_label_set_text(roll_value, "---" DEGSTR);
	w = lv_obj_get_width(roll_value);
	h = lv_obj_get_height(roll_value);
	lv_obj_align(roll_label, cap_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
	lv_obj_align(roll_value, roll_label, LV_ALIGN_OUT_RIGHT_MID, 3, 0);

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
	set_auto_task = lv_task_create(
	    edisp_set_auto_timeout, 2000, LV_TASK_PRIO_MID, NULL);

}

void
edisp_autopilot_set_cogsog(const char *cog, const char *sog)
{
	lv_label_set_text(capf0_value, cog);
	lv_label_set_text(vittf0_value, sog);
}
