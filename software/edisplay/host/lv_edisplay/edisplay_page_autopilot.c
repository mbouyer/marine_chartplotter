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
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/atomic.h>
#include "lvgl/lvgl.h"
#include "edisplay_pages.h"
#include "edisplay_data.h"
#include "hal.h"

static lv_obj_t *cap_value;
static lv_obj_t *autocap_value;
static lv_obj_t *autoslot_value;
static lv_obj_t *roll_value;
static lv_obj_t *capf0_value;
static lv_obj_t *vittf0_value;

static lv_task_t *set_attitude_task;
static lv_task_t *set_auto_task;

static volatile int received_heading;
static volatile int received_roll;
static volatile uint attitude_gen;

static uint8_t auto_mode;
#define AUTO_INVALID 0xff
static double auto_heading;
static uint8_t auto_slot;

static volatile uint8_t new_auto_mode;
static volatile double new_auto_heading;
static volatile uint8_t new_auto_slot;
static volatile uint auto_gen;

typedef struct _auto_factors_t {
	int err;
	int diff;
	int diff2;
} auto_factors_t;

auto_factors_t factors_values[NPARAMS] = { 0 };

pthread_cond_t factors_cond;
static pthread_mutex_t factors_mtx;
static pthread_mutexattr_t factors_mtx_attr;

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
edisp_factors_lock(void)
{
	if (pthread_mutex_lock(&factors_mtx)) {
		fprintf(stderr, "pthread_mutex_lock(&factors_mtx) failed\n");
		abort();
	}
}

static void
edisp_factors_unlock(void)
{
	if (pthread_mutex_unlock(&factors_mtx)) {
		fprintf(stderr,       
		    "pthread_mutex_unlock(&factors_mtx) failed\n");  
		abort();
	}
}

static int
edisp_auto_get_factor(int slot)
{
	int ret;
	struct timeval now;
	struct timespec ts;
	for (int i = 0; i < 5; i++) {
		(void)gettimeofday(&now, NULL);
		TIMEVAL_TO_TIMESPEC(&now, &ts);
		ts.tv_sec++;
		edisp_factors_lock();
		factors_values[slot].diff = -1;
		if (!n2ks_auto_factors_request(slot)) {
			edisp_factors_unlock();
			return EIO;
		}
		ret = pthread_cond_timedwait(&factors_cond, &factors_mtx, &ts);
		edisp_factors_unlock();
		if (ret == 0 && factors_values[slot].diff >= 0)
			return 0;
		ret = ETIMEDOUT;
	}
	return ret;
}

void
edisp_set_auto_factors(uint8_t slot, int err, int dif, int dif2)
{
	edisp_factors_lock();
	factors_values[slot].err = err;
	factors_values[slot].diff = dif;
	factors_values[slot].diff2 = dif2;
	pthread_cond_signal(&factors_cond);
	edisp_factors_unlock();
}


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
	attitude_gen++;
	membar_producer();
	received_heading = heading;
	received_roll = roll;
	membar_producer();
}

static void
edisp_attitude_update(void)
{
	static uint gen = 0;
	char buf[6];
	int heading, roll;

	if (gen == attitude_gen)
		return;

	while (attitude_gen != gen || (gen % 1) != 0) {
		gen = attitude_gen;
		membar_consumer(); 
		heading = received_heading;
		roll = received_roll;
		membar_consumer();    
	}

	snprintf(buf, 6, "%3d" DEGSTR, heading);
	lv_label_set_text(cap_value, buf);

	snprintf(buf, 6, "%3d" DEGSTR, roll);
	lv_label_set_text(roll_value, buf);
	lv_task_reset(set_attitude_task);
}

static void
edisp_set_auto_timeout(lv_task_t *task)
{
	lv_label_set_text(autocap_value, "---" DEGSTR);
	lv_label_set_text(autoslot_value, "--");
	auto_mode = AUTO_INVALID;
	auto_heading = -1;
}

void
edisp_set_auto_status(uint8_t mode, double heading, uint8_t error, uint8_t slot)
{
	auto_gen++;
	membar_producer();
	new_auto_mode = mode;
	new_auto_heading = heading;
	new_auto_slot = slot;
	membar_producer();
	auto_gen++;
}

static void
edisp_autocap_update(void)
{
	static uint gen = 0;
	char buf[6];
	int mode, slot;
	double heading;

	if (gen == auto_gen)
		return;

	while (auto_gen != gen || (gen % 1) != 0) {
		gen = auto_gen;
		membar_consumer(); 
		mode = new_auto_mode;
		heading = new_auto_heading;
		slot = new_auto_slot;
		membar_consumer();    
	}

	lv_task_reset(set_auto_task);
	/* XXX handle errors */
	if (mode == auto_mode && heading == auto_heading && slot == auto_slot) {
		return; /* nothing changed */
	}
	snprintf(buf, 6, "P%1d", slot);
	lv_label_set_text(autoslot_value, buf);
	auto_slot = slot;
	auto_mode = mode;
	switch(mode) {
	case AUTO_OFF:
		auto_heading = -1;
		lv_label_set_text(autocap_value, "OFF ");
		break;
	case AUTO_STANDBY:
		lv_label_set_text(autocap_value, "STBY");
		break;
	case AUTO_HEAD:
		auto_heading = heading;
		snprintf(buf, 6, "%3d" DEGSTR, (int)heading);
		lv_label_set_text(autocap_value, buf);
		break;
	}
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
		if (auto_heading >= 0 && active) {
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
auto_slot_action(lv_obj_t *list, lv_event_t event)
{
	int key;
	int value;
	switch(event) {
	case LV_EVENT_VALUE_CHANGED:
		value = lv_ddlist_get_selected(list);
		transient_close(list);
		n2ks_auto_engage(auto_heading, auto_mode, value);
		break;
	case LV_EVENT_KEY:
		key = *((uint32_t *)lv_event_get_data());
		if (key == LV_KEY_ESC) {
			transient_close(list);
		}
		break;
	}
}

static void
auto_slot_list(void)
{
	char slotnames[500];

	slotnames[0] = '\0';

	for (int i = 0; i < NPARAMS; i++) {
		if (i > 0)
			strlcat(slotnames, "\n", sizeof(slotnames));
		edisp_auto_get_factor(i);
		snprintf(&slotnames[strlen(slotnames)], 
		    sizeof(slotnames) - strlen(slotnames),
		    "P%d %04d %04d %04d", i,
		    factors_values[i].err,
		    factors_values[i].diff,
		    factors_values[i].diff2
		    );
	}
	transient_list(slotnames, auto_slot, auto_slot_action);
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
			default:
				return;
			}
			n2ks_auto_engage(new_head, AUTO_HEAD, auto_slot);
		}
		return;
	case LV_EVENT_LONG_PRESSED:
		auto_slot_list();
		return;
	}
	printf("autopilot event ");
	print_ev(event);
	printf("\n");
}

static void
edisp_create_autopilot()
{
	if (pthread_cond_init(&factors_cond, NULL) != 0) {
		errx(1, "pthread_cond_init failed");
	}
	pthread_mutexattr_init(&factors_mtx_attr);
	pthread_mutexattr_settype(&factors_mtx_attr, PTHREAD_MUTEX_ERRORCHECK);
	if (pthread_mutex_init(&factors_mtx, &factors_mtx_attr)) {
		fprintf(stderr,
		    "pthread_mutex_init(&factors_mtx) failed\n");
		abort(); 
	}

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

	autoslot_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(autoslot_value, LV_LABEL_STYLE_MAIN, &style_medium_text);
	lv_label_set_text(autoslot_value, "--");
	lv_obj_align(autoslot_value, autocap_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
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
	lv_event_send(autocap_value, LV_EVENT_REFRESH, NULL);
}

void
edisp_update_autopilot(void)
{
	edisp_attitude_update();
	edisp_autocap_update();
}
