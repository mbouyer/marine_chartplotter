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
#include <sys/atomic.h>
#include "lvgl/lvgl.h"
#include "edisplay_pages.h"
#include "hal.h"

static lv_obj_t *capp_value;
static lv_obj_t *distp_value;
static lv_obj_t *route_value;
static lv_obj_t *capf1_value;
static lv_obj_t *vittf1_value;

static volatile int received_cog;
static volatile int received_sog;
static volatile uint cogsog_gen;
static volatile int received_xte;
static volatile uint xte_gen;
static volatile int received_capp;
static volatile int received_distp;
static volatile uint32_t received_wp;
static volatile uint navdata_gen;

static lv_task_t *set_cogsog_task;
static lv_task_t *set_xte_task;
static lv_task_t *set_navdata_task;

static void edisp_create_navdata(void);
edisp_page_t epage_navdata = {
        edisp_create_navdata,
	activate_page,
	"cog/sog",
	true,
	false,
	NULL
};

#define edisp_page (epage_navdata.epage_page)


static void
edisp_set_cogsog_timeout(lv_task_t *task)
{
	edisp_autopilot_set_cogsog("---" DEGSTR, "---n");
	lv_label_set_text(capf1_value, "---" DEGSTR);
	lv_label_set_text(vittf1_value, "---n");
}

void
edisp_set_cogsog(int cog, int sog)
{
	cogsog_gen++;
	membar_producer();
	received_cog = cog;
	received_sog = sog;
	membar_producer();
	cogsog_gen++;
}

static void
edisp_cogsog_update(void)
{
	static uint gen = 0;
	int cog, sog;

	if (gen == cogsog_gen)
		return;

	while (cogsog_gen != gen || (gen % 1) != 0) {
		gen = cogsog_gen;
		membar_consumer();
		sog = received_sog;
		cog = received_cog;
		membar_consumer();
	}

	int kn = sog * 360 / 1852; /* kn * 10 */
	char cogs[6];
	char sogs[6];
	snprintf(cogs, 6, "%3d" DEGSTR, cog);

	if (kn > 100) {
		snprintf(sogs, 6, "%3dn", kn / 10);
	} else {
		snprintf(sogs, 6, "%d.%dn", kn / 10, kn % 10);
	}
	edisp_autopilot_set_cogsog(cogs, sogs);
	lv_label_set_text(capf1_value, cogs);
	lv_label_set_text(vittf1_value, sogs);

	lv_task_reset(set_cogsog_task);
}

static void
edisp_set_xte_timeout(lv_task_t *task)
{
	lv_label_set_text(route_value, "  ----mn  ");
}

void
edisp_set_xte(int xte)
{
	xte_gen++;
	membar_producer();
	received_xte = xte;
	membar_producer();
	xte_gen++;
}

static void
edisp_xte_update(void)
{
	static uint gen = 0;
	char buf[11];
	char l, r;
	int xte;

	if (gen == xte_gen)
		return;

	while (xte_gen != gen || (gen % 1) != 0) {
		xte = received_xte;
		membar_consumer();
		gen = xte_gen;
		membar_consumer();
	}

	l = r = ' ';

	if (xte < 0) {
		r = '>';
		xte = -xte;
	} else {
		l = '<';
	}
	if (xte > 1852000) { /* 10mn */
		snprintf(buf, 11, "%c %4dmn %c", l, xte / 185200, r);
	} else if (xte > 18520UL) { /* 0.1mn */
		snprintf(buf, 11, "%c %1d.%02dmn %c", l, (xte / 185200),
		    (xte % 185200) / 1852, r);
	} else {
		snprintf(buf, 11, "%c  %3dm  %c", l, xte / 100, r);
	}
	lv_label_set_text(route_value, buf);

	lv_task_reset(set_xte_task);
}

static uint32_t old_wp;
static bool wp_valid = 0;

static void
edisp_set_navdata_timeout(lv_task_t *task)
{
	lv_label_set_text(capp_value, "---" DEGSTR);
	lv_label_set_text(distp_value, "----mn");
	wp_valid = 0;
}

void
edisp_set_navdata(int capp, int distp, uint32_t wp)
{
	navdata_gen++;
	membar_producer();
	received_capp = capp;
	received_distp = distp;
	received_wp = wp;
	membar_producer();
	navdata_gen++;
}

static void
edisp_navdata_update(void)
{
	static uint gen = 0;
	char buf[10];
	int distp, capp;
	uint32_t wp;

	if (gen == navdata_gen)
		return;

	while (navdata_gen != gen || (gen % 1) != 0) {
		gen = navdata_gen;
		membar_consumer();
		capp = received_capp;
		distp = received_distp;
		wp = received_wp;
		membar_consumer();
	}

	if (wp_valid == 0 || old_wp != wp) {
		old_wp = wp;
		wp_valid = 1;
		switch_to_page_o(edisp_page);
	}

	snprintf(buf, 10, "%3d" DEGSTR, capp);
	lv_label_set_text(capp_value, buf);
	if (distp > 1852000) { /* 10mn */
		snprintf(buf, 10, "%4dmn", distp / 185200);
	} else if (distp > 18520UL) { /* 0.1mn */
		snprintf(buf, 10, "%1d.%02dmn", (distp / 185200),
		    (distp % 185200) / 1852);
	} else {
		snprintf(buf, 10, " %3dm ", distp / 100);
	}
	lv_label_set_text(distp_value, buf);
	lv_task_reset(set_navdata_task);
}

static void
edisp_create_navdata()
{
	capp_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(capp_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(capp_value, "---" DEGSTR);
	int w = lv_obj_get_width(capp_value);
	int h = lv_obj_get_height(capp_value);
	lv_obj_align(capp_value, NULL, LV_ALIGN_OUT_TOP_LEFT, 5, h + 10);

	distp_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(distp_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(distp_value, "----mn");
	w = lv_obj_get_width(distp_value);
	h = lv_obj_get_height(distp_value);
	lv_obj_align(distp_value, capp_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	lv_obj_t *route_label = lv_label_create(edisp_page, NULL);
	lv_label_set_style(route_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(route_label, "route");
	route_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(route_value, LV_LABEL_STYLE_MAIN, &style_medium_text);
	lv_label_set_text(route_value, "  ----mn  ");
	w = lv_obj_get_width(route_value);
	h = lv_obj_get_height(route_value);
	lv_obj_align(route_label, capp_value, LV_ALIGN_OUT_BOTTOM_LEFT, 5, h - 5);
	lv_obj_align(route_value, route_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	capf1_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(capf1_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(capf1_value, "---" DEGSTR);
	lv_obj_align(capf1_value, capp_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, h + 10);

	lv_obj_t *fond_label = lv_label_create(edisp_page, NULL);
	lv_label_set_style(fond_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(fond_label, "fond");

	vittf1_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(vittf1_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(vittf1_value, "---n");
	w = lv_obj_get_width(vittf1_value);
	h = lv_obj_get_height(vittf1_value);
	lv_obj_align(fond_label, capf1_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_align(vittf1_value, fond_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	set_xte_task = lv_task_create(
	    edisp_set_xte_timeout, 5000, LV_TASK_PRIO_MID, NULL);
	set_navdata_task = lv_task_create(
	    edisp_set_navdata_timeout, 5000, LV_TASK_PRIO_MID, NULL);
	set_cogsog_task = lv_task_create(
	    edisp_set_cogsog_timeout, 5000, LV_TASK_PRIO_MID, NULL);
}

void
edisp_update_navdata(void)
{
	edisp_cogsog_update();
	edisp_xte_update();
	edisp_navdata_update();
}
