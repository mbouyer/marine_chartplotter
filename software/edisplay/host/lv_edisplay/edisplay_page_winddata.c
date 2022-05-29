/*
 *  Copyright (c) 2022 Manuel Bouyer.
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
#include <sys/time.h>
#include <sys/atomic.h>
#include "lvgl/lvgl.h"
#include "edisplay_pages.h"
#include "hal.h"

static lv_obj_t *dir_value;
static lv_obj_t *aws_value;
static lv_obj_t *awa_value;
static lv_obj_t *tws_value;
static lv_obj_t *twa_value;
static lv_obj_t *vmg_value;

static volatile int received_heading;
static bool heading_valid = 0;
static volatile int received_cog;
static volatile int received_sog;
static bool cogsog_valid = 0;

static volatile int received_awa;
static volatile int received_aws;
static volatile uint wind_gen;
static bool wind_valid = 0;


static lv_task_t *set_heading_task;
static lv_task_t *set_cogsog_task;
static lv_task_t *set_winddata_task;

static void edisp_create_winddata(void);
edisp_page_t epage_winddata = {
        edisp_create_winddata,
	activate_page,
	"wind",
	true,
	false,
	NULL
};

#define edisp_page (epage_winddata.epage_page)

#define GAUGE_N_AWA	0
#define GAUGE_N_TWA	1
#define GAUGE_N_COG	2


static void
edisp_set_cogsog_timeout(lv_task_t *task)
{
#if 0
	edisp_autopilot_set_cogsog("---" DEGSTR, "---n");
	lv_label_set_text(vmg_value, "---" DEGSTR);
	lv_label_set_text(vittf1_value, "---n");
#endif
	cogsog_valid = 0;
}

void
edisp_winddata_set_cogsog(int sog, int cog) /* kn * 10, deg */
{
	/* called from display thread context, no concurent read/update */
	received_cog = cog;
	received_sog = sog;
	lv_task_reset(set_cogsog_task);
	cogsog_valid = 1;
}

static void
edisp_set_heading_timeout(lv_task_t *task)
{
#if 0
	edisp_autopilot_set_cogsog("---" DEGSTR, "---n");
	lv_label_set_text(vmg_value, "---" DEGSTR);
	lv_label_set_text(vittf1_value, "---n");
#endif
	heading_valid = 0;
}

void
edisp_winddata_set_heading(int heading)
{
	/* called from display thread context, no concurent read/update */
	received_heading = heading;
	lv_task_reset(set_heading_task);
	heading_valid = 1;
}

void
edisp_set_winddata(int adir, int aspeed)
{
	wind_gen++;
	membar_producer();
	received_awa = adir;
	received_aws = aspeed;
	membar_producer();
	wind_gen++;
}

static void
edisp_wind_update(void)
{
	static uint gen = 0;
	char buf[11];
	int aws, awa;

	if (gen == wind_gen)
		return;

	while (wind_gen != gen || (gen % 1) != 0) {
		awa = received_awa;
		aws = received_aws;
		membar_consumer();
		gen = wind_gen;
		membar_consumer();
	}
	if (awa > 180) {
		awa = awa - 360;
	}
	lv_gauge_set_value(dir_value, 0, awa);
	snprintf(buf, sizeof(buf), "%3d" DEGSTR "a", abs(awa));
	lv_label_set_text(awa_value, buf);

	int kn = aws * 360 / 1852; /* kn * 10 */
	snprintf(buf, 6, "%3dn", (kn + 5) / 10); 
	lv_label_set_text(aws_value, buf);

	lv_task_reset(set_winddata_task);
}

static void
edisp_set_winddata_timeout(lv_task_t *task)
{
	lv_gauge_set_value(dir_value, GAUGE_N_AWA, 0);
	lv_gauge_set_value(dir_value, GAUGE_N_TWA, 1);
	lv_label_set_text(awa_value, "---" DEGSTR "a");
	lv_label_set_text(twa_value, "---" DEGSTR "v");
	lv_label_set_text(aws_value, "---n");
	lv_label_set_text(tws_value, "---n");
	lv_label_set_text(vmg_value, "----n");
	wind_valid = 0;
}

static void
edisp_create_winddata()
{
	lv_color_t colors[3] = {LV_COLOR_BLACK, LV_COLOR_BLACK, LV_COLOR_BLACK};
	static lv_style_t w_style;
	static lv_style_t text_style;

	lv_style_copy(&text_style, &style_medium_text);
	text_style.body.main_color = LV_COLOR_WHITE;
	text_style.body.opa = LV_OPA_COVER;

	lv_style_copy(&w_style, &lv_style_transp_tight);
	w_style.body.padding.left = 10; /*Scale line length*/
	w_style.body.padding.inner = 8; /*Scale label padding*/
	w_style.line.width = 3;
	w_style.body.radius = 20;

	dir_value = lv_gauge_create(edisp_page, NULL);
	lv_gauge_set_needle_count(dir_value, 3, colors);
	lv_obj_set_size(dir_value, 90, 90);
	lv_obj_align(dir_value, edisp_page, LV_ALIGN_CENTER, 80 - 45 - 10, 0);
	lv_gauge_set_range(dir_value, -180, 180);
	lv_gauge_set_scale(dir_value, 360, 13, 0);
	lv_gauge_set_value(dir_value, 0, 0);
	lv_gauge_set_value(dir_value, 1, 0);
	lv_gauge_set_value(dir_value, 2, 0);

	awa_value = lv_label_create(dir_value, NULL);
	lv_label_set_style(awa_value, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_body_draw(awa_value, 1);
	lv_label_set_text(awa_value, "awa" DEGSTR "a");
	int w = lv_obj_get_width(awa_value);
	int h = lv_obj_get_height(awa_value);
	lv_obj_align(awa_value, dir_value, LV_ALIGN_CENTER, 0, -h / 2);

	twa_value = lv_label_create(dir_value, NULL);
	lv_label_set_style(twa_value, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_body_draw(twa_value, 1);
	lv_label_set_text(twa_value, "twa" DEGSTR "v");
	w = lv_obj_get_width(twa_value);
	h = lv_obj_get_height(twa_value);
	lv_obj_align(twa_value, awa_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

	tws_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(tws_value, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_text(tws_value, "twsn");
	w = lv_obj_get_width(tws_value);
	h = lv_obj_get_height(tws_value);
	lv_obj_align(tws_value, aws_value, LV_ALIGN_OUT_LEFT_MID, 5 + w, h/2);

	aws_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(aws_value, LV_LABEL_STYLE_MAIN, &text_style);
	lv_label_set_text(aws_value, "awsn");
	w = lv_obj_get_width(aws_value);
	h = lv_obj_get_height(aws_value);
	lv_obj_align(aws_value, tws_value, LV_ALIGN_OUT_TOP_LEFT, 0, 0);

	vmg_value = lv_label_create(edisp_page, NULL);
	lv_label_set_style(vmg_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(vmg_value, "vmg-n");
	w = lv_obj_get_width(vmg_value);
	h = lv_obj_get_height(vmg_value);
	lv_obj_align(vmg_value, edisp_page, LV_ALIGN_OUT_BOTTOM_LEFT, 5, -h - 5);

	set_winddata_task = lv_task_create(
	    edisp_set_winddata_timeout, 5000, LV_TASK_PRIO_MID, NULL);
	set_heading_task = lv_task_create(
	    edisp_set_heading_timeout, 5000, LV_TASK_PRIO_MID, NULL);
	set_cogsog_task = lv_task_create(
	    edisp_set_cogsog_timeout, 5000, LV_TASK_PRIO_MID, NULL);
}

void
edisp_update_winddata(void)
{
	edisp_wind_update();
}
