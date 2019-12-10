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

#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <sys/time.h>
#include "lvgl/lvgl.h"
#include "edisplay.h"
#include "edisplay_font.h"
#include "hal.h"
#include "lv_drivers/indev/mousewheel.h"

static void page_list(void);
static void light_slide(void);

static lv_obj_t *pages[4];
static int current_page = 0;

lv_obj_t *cap_value;
lv_obj_t *autocap_value;
lv_obj_t *roll_value;
lv_obj_t *capf0_value;
lv_obj_t *vittf0_value;

lv_obj_t *capp_value;
lv_obj_t *distp_value;
lv_obj_t *route_value;
lv_obj_t *capf1_value;
lv_obj_t *vittf1_value;

lv_task_t *set_attitude_task;
lv_task_t *set_cogsog_task;
lv_task_t *set_xte_task;
lv_task_t *set_navdata_task;

static lv_obj_t *lv_top;
static lv_obj_t *lv_top_trs; /* transient top displays (menu, err msg, ...) */
static lv_group_t *encg;

static struct button {
	lv_obj_t *button;
	lv_obj_t *label;
} buttons[4];

#define DEGSTR "°"

static void
print_ev(lv_event_t event)
{
	switch(event) {
	case LV_EVENT_PRESSED:
		printf("LV_EVENT_PRESSED");
		break;
	case LV_EVENT_PRESSING:
		printf("LV_EVENT_PRESSING");
		break;
	case LV_EVENT_PRESS_LOST:
		printf("LV_EVENT_PRESS_LOST");
		break;
	case LV_EVENT_SHORT_CLICKED:
		printf("LV_EVENT_SHORT_CLICKED");
		break;
	case LV_EVENT_LONG_PRESSED:
		printf("LV_EVENT_LONG_PRESSED");
		break;
	case LV_EVENT_LONG_PRESSED_REPEAT:
		printf("LV_EVENT_LONG_PRESSED_REPEAT");
		break;
	case LV_EVENT_CLICKED:
		printf("LV_EVENT_CLICKED");
		break;
	case LV_EVENT_RELEASED:
		printf("LV_EVENT_RELEASED");
		break;
	case LV_EVENT_DRAG_BEGIN:
		printf("LV_EVENT_DRAG_BEGIN");
		break;
	case LV_EVENT_DRAG_END:
		printf("LV_EVENT_DRAG_END");
		break;
	case LV_EVENT_DRAG_THROW_BEGIN:
		printf("LV_EVENT_DRAG_THROW_BEGIN");
		break;
	case LV_EVENT_KEY:
		printf("LV_EVENT_KEY");
		break;
	case LV_EVENT_FOCUSED:
		printf("LV_EVENT_FOCUSED");
		break;
	case LV_EVENT_DEFOCUSED:
		printf("LV_EVENT_DEFOCUSED");
		break;
	case LV_EVENT_VALUE_CHANGED:
		printf("LV_EVENT_VALUE_CHANGED");
		break;
	case LV_EVENT_INSERT:
		printf("LV_EVENT_INSERT");
		break;
	case LV_EVENT_REFRESH:
		printf("LV_EVENT_REFRESH");
		break;
	case LV_EVENT_APPLY:
		printf("LV_EVENT_APPLY");
		break;
	case LV_EVENT_CANCEL:
		printf("LV_EVENT_CANCEL");
		break;
	case LV_EVENT_DELETE:
		printf("LV_EVENT_DELETE");
		break;
	default:
		printf("unkown %d", event);
	}
}

static void back_click_action(lv_obj_t * obj, lv_event_t event)
{

	switch(event) {
	case LV_EVENT_SHORT_CLICKED:
		page_list();
		return;
	}
	   printf("back event ");
	   print_ev(event);
	printf("\n");
}

static void
btn1_click_action(lv_obj_t * btn, lv_event_t event)
{

	   printf("buttons[1].button event ");
	   print_ev(event);
	printf("\n");
}

static void
btn2_click_action(lv_obj_t * btn, lv_event_t event)
{

	   switch(event) {
	   case LV_EVENT_LONG_PRESSED:
		light_slide();
		return;
	   }
	   printf("buttons[2].button event ");
	   print_ev(event);
	printf("\n");
}

static void
btn3_click_action(lv_obj_t * btn, lv_event_t event)
{

	   printf("buttons[3].button event ");
	   print_ev(event);
	printf("\n");
}

static void
btn4_click_action(lv_obj_t * btn, lv_event_t event)
{

	   printf("buttons[4].button event ");
	   print_ev(event);
	printf("\n");
}

static void
enc_group_close(lv_obj_t *obj)
{
	lv_group_set_editing(encg, false);
	lv_group_focus_freeze(encg, false);
	lv_group_remove_obj(obj);
	lv_obj_del_async(obj);
	lv_group_set_editing(encg, true);
	lv_group_focus_freeze(encg, true);
}

static void
light_action(lv_obj_t *slide, lv_event_t event)
{
	switch(event) {
	case LV_EVENT_SHORT_CLICKED:
		printf("new light %d\n", lv_slider_get_value(slide));
		enc_group_close(slide);
		break;
	default:
		   printf("light_action: ");
		   print_ev(event);
		   printf("\n");
	}
}

static void
light_slide(void)
{
	lv_obj_t *slide = lv_slider_create(lv_top_trs, NULL);
	lv_slider_set_range(slide, 10, 100);
	lv_obj_set_event_cb(slide, light_action);
	lv_obj_move_foreground(slide);
	lv_obj_align(slide, lv_top_trs, LV_ALIGN_CENTER, 0, 0);
	lv_group_set_editing(encg, false);
	lv_group_focus_freeze(encg, false);
	lv_group_add_obj(encg, slide);
	lv_group_focus_obj(slide);
	lv_group_set_editing(encg, true);
	lv_group_focus_freeze(encg, true);
}

static void
page_action(lv_obj_t *list, lv_event_t event)
{
	switch(event) {
	case LV_EVENT_VALUE_CHANGED:
		{
		int value = lv_ddlist_get_selected(list);
		printf("new page %d\n", value);
		enc_group_close(list);
		switch(value) {
		case 3:
			light_slide();
			break;
		default:
			current_page = value;
			lv_scr_load(pages[current_page]);
		}
		break;
		}
	}
}

static void
page_list(void)
{
	lv_obj_t *list = lv_ddlist_create(lv_top_trs, NULL);
	lv_ddlist_set_options(list,
		"cap/auto\n"
		"cog/sog\n"
		"waypoint\n"
		"retro-eclairage"
		);
	lv_ddlist_set_selected(list, current_page);
	lv_obj_set_event_cb(list, page_action);
	lv_ddlist_set_stay_open(list, true);
	lv_obj_move_foreground(list);
	lv_obj_align(list, lv_top_trs, LV_ALIGN_CENTER, 0, 0);
	lv_group_set_editing(encg, false);
	lv_group_focus_freeze(encg, false);
	lv_group_add_obj(encg, list);
	lv_group_focus_obj(list);
	lv_group_set_editing(encg, true);
	lv_group_focus_freeze(encg, true);
}

static void
edisplay_buttons_create(void)
{
	static lv_style_t style_btn;
	static lv_style_t style_btn_pr;
	lv_style_copy(&style_btn, &lv_style_transp_tight);
	lv_style_copy(&style_btn_pr, &lv_style_transp_tight);
	style_btn_pr.glass = 0;
	style_btn_pr.body.opa = LV_OPA_COVER;
	style_btn_pr.body.main_color = LV_COLOR_BLACK;
	style_btn_pr.body.grad_color = LV_COLOR_BLACK;
	style_btn_pr.text.color = LV_COLOR_WHITE;
	style_btn_pr.image.color = LV_COLOR_WHITE;
	style_btn_pr.line.color = LV_COLOR_WHITE;

	buttons[1].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[1].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[1].button, btn1_click_action);
	lv_btn_set_style(buttons[1].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[1].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[1].label = lv_label_create(buttons[1].button, NULL);
	lv_label_set_static_text(buttons[1].label, "MoB");
	lv_coord_t w = lv_obj_get_width(buttons[1].button);
	lv_coord_t h = lv_obj_get_height(buttons[1].button);
	lv_obj_align(buttons[1].button, NULL, LV_ALIGN_OUT_TOP_LEFT, 0, h);

	buttons[2].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[2].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[2].button, btn2_click_action);
	lv_btn_set_style(buttons[2].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[2].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[2].label = lv_label_create(buttons[2].button, NULL);
	lv_label_set_static_text(buttons[2].label, "light");
	w = lv_obj_get_width(buttons[2].button);
	h = lv_obj_get_height(buttons[2].button);
	lv_obj_align(buttons[2].button, NULL, LV_ALIGN_OUT_TOP_RIGHT, 0, h);

	buttons[3].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[3].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[3].button, btn3_click_action);
	lv_btn_set_style(buttons[3].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[3].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[3].label = lv_label_create(buttons[3].button, NULL);
	lv_label_set_static_text(buttons[3].label, "page");
	w = lv_obj_get_width(buttons[3].button);
	h = lv_obj_get_height(buttons[3].button);
	lv_obj_align(buttons[3].button, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -h);

	buttons[4].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[4].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[4].button, btn4_click_action);
	lv_btn_set_style(buttons[4].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[4].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[4].label = lv_label_create(buttons[4].button, NULL);
	lv_label_set_static_text(buttons[4].label, "cancel");
	w = lv_obj_get_width(buttons[4].button);
	h = lv_obj_get_height(buttons[4].button);
	lv_obj_align(buttons[4].button, NULL, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -h);
}


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
edisp_set_cogsog_timeout(lv_task_t *task)
{
	lv_label_set_text(capf0_value, "---" DEGSTR);
	lv_label_set_text(capf1_value, "---" DEGSTR);
	lv_label_set_text(vittf0_value, "---n");
	lv_label_set_text(vittf1_value, "---n");
}
void
edisp_set_cogsog(int cog, int sog)
{
	int kn = sog * 360 / 1852; /* kn * 10 */
	char buf[6];
	snprintf(buf, 6, "%3d" DEGSTR, cog);
	lv_label_set_text(capf0_value, buf);
	lv_label_set_text(capf1_value, buf);

	if (kn > 100) {
		snprintf(buf, 6, "%3dn", kn / 10);
	} else {
		snprintf(buf, 6, "%d.%dn", kn / 10, kn % 10);
	}
	lv_label_set_text(vittf0_value, buf);
	lv_label_set_text(vittf1_value, buf);

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
	char buf[11];
	char l, r;
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

static void
edisp_set_navdata_timeout(lv_task_t *task)
{
	lv_label_set_text(capp_value, "---" DEGSTR);
	lv_label_set_text(distp_value, "----mn");
}
void
edisp_set_navdata(int capp, int distp)
{
	char buf[10];
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
/**
 * Create a edisplay application
 */
void
edisplay_app_init(void)
{
	int i;

	lv_init();

	encg = lv_group_create();
	lv_group_set_refocus_policy(encg, LV_GROUP_REFOCUS_POLICY_PREV);

	hal_init(encg);

	lv_coord_t hres = lv_disp_get_hor_res(NULL);
	lv_coord_t vres = lv_disp_get_ver_res(NULL);
	lv_theme_t * th = lv_theme_mono_init(20, NULL);
	lv_theme_set_current(th);

	static lv_style_t style_small_text;
	lv_style_copy(&style_small_text, &lv_style_transp_tight);
	style_small_text.text.font = &lv_font_unscii_8;

	static lv_style_t style_medium_text;
	lv_style_copy(&style_medium_text, &lv_style_transp_tight);
	style_medium_text.text.font = &lib16;

	static lv_style_t style_large_text;
	lv_style_copy(&style_large_text, &lv_style_transp_tight);
	style_large_text.text.font = &lib24;

	lv_top = lv_disp_get_layer_top(NULL);
	lv_obj_set_click(lv_top, 1);
	lv_obj_set_event_cb(lv_top, back_click_action);
	lv_group_add_obj(encg, lv_top);
	lv_group_focus_obj(lv_top);
	lv_group_set_editing(encg, true);
	lv_group_focus_freeze(encg, true);

	lv_top_trs = lv_obj_create(lv_top, lv_top);

	edisplay_buttons_create();
	
	for (i = 0; i < sizeof(pages) / sizeof(pages[0]); i++) {
		pages[i] = lv_obj_create(NULL, NULL);
		lv_obj_set_style(pages[i], lv_obj_get_style(lv_scr_act()));
	}

	/* create page0 */

	cap_value = lv_label_create(pages[0], NULL);
	lv_label_set_style(cap_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(cap_value, "---" DEGSTR);
	int w = lv_obj_get_width(cap_value);
	int h = lv_obj_get_height(cap_value);
	lv_obj_align(cap_value, NULL, LV_ALIGN_OUT_TOP_LEFT, 0, h + 10);

	lv_obj_t *autocap_label = lv_label_create(pages[0], NULL);
	lv_label_set_style(autocap_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(autocap_label, "auto");
	autocap_value = lv_label_create(pages[0], NULL);
	lv_label_set_style(autocap_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(autocap_value, "---" DEGSTR);
	w = lv_obj_get_width(autocap_value);
	h = lv_obj_get_height(autocap_value);
	lv_obj_align(autocap_label, cap_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_align(autocap_value, autocap_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	lv_obj_t *roll_label = lv_label_create(pages[0], NULL);
	lv_label_set_style(roll_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(roll_label, "gite");
	roll_value = lv_label_create(pages[0], NULL);
	lv_label_set_style(roll_value, LV_LABEL_STYLE_MAIN, &style_medium_text);
	lv_label_set_text(roll_value, "---" DEGSTR);
	w = lv_obj_get_width(roll_value);
	h = lv_obj_get_height(roll_value);
	lv_obj_align(roll_label, cap_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
	lv_obj_align(roll_value, roll_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	capf0_value = lv_label_create(pages[0], NULL);
	lv_label_set_style(capf0_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(capf0_value, "---" DEGSTR);
	w = lv_obj_get_width(capf0_value);
	h = lv_obj_get_height(capf0_value);
	lv_obj_align(capf0_value, roll_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

	lv_obj_t *fond_label = lv_label_create(pages[0], NULL);
	lv_label_set_style(fond_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(fond_label, "fond");

	vittf0_value = lv_label_create(pages[0], NULL);
	lv_label_set_style(vittf0_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(vittf0_value, "---n");
	w = lv_obj_get_width(vittf0_value);
	h = lv_obj_get_height(vittf0_value);
	lv_obj_align(fond_label, capf0_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	lv_obj_align(vittf0_value, fond_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
	set_attitude_task = lv_task_create(
	    edisp_set_attitude_timeout, 2000, LV_TASK_PRIO_MID, NULL);
	set_cogsog_task = lv_task_create(
	    edisp_set_cogsog_timeout, 5000, LV_TASK_PRIO_MID, NULL);

	/* create page1 */

	capp_value = lv_label_create(pages[1], NULL);
	lv_label_set_style(capp_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(capp_value, "---" DEGSTR);
	w = lv_obj_get_width(capp_value);
	h = lv_obj_get_height(capp_value);
	lv_obj_align(capp_value, NULL, LV_ALIGN_OUT_TOP_LEFT, 5, h + 10);

	distp_value = lv_label_create(pages[1], NULL);
	lv_label_set_style(distp_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(distp_value, "----mn");
	w = lv_obj_get_width(distp_value);
	h = lv_obj_get_height(distp_value);
	lv_obj_align(distp_value, capp_value, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	lv_obj_t *route_label = lv_label_create(pages[1], NULL);
	lv_label_set_style(route_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(route_label, "route");
	route_value = lv_label_create(pages[1], NULL);
	lv_label_set_style(route_value, LV_LABEL_STYLE_MAIN, &style_medium_text);
	lv_label_set_text(route_value, "  ----mn  ");
	w = lv_obj_get_width(route_value);
	h = lv_obj_get_height(route_value);
	lv_obj_align(route_label, capp_value, LV_ALIGN_OUT_BOTTOM_LEFT, 5, h - 5);
	lv_obj_align(route_value, route_label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

	capf1_value = lv_label_create(pages[1], NULL);
	lv_label_set_style(capf1_value, LV_LABEL_STYLE_MAIN, &style_large_text);
	lv_label_set_text(capf1_value, "---" DEGSTR);
	lv_obj_align(capf1_value, capp_value, LV_ALIGN_OUT_BOTTOM_LEFT, 0, h + 10);

	fond_label = lv_label_create(pages[1], NULL);
	lv_label_set_style(fond_label, LV_LABEL_STYLE_MAIN, &style_small_text);
	lv_label_set_static_text(fond_label, "fond");

	vittf1_value = lv_label_create(pages[1], NULL);
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

	lv_scr_load(pages[0]);
	current_page = 0;
}

void edisplay_app_run(void)
{
	struct timeval tv, tv_prev, tv_diff;
	time_t ticktime;

	if (gettimeofday(&tv_prev, NULL) < 0) {
		err(1, "gettimeofday");
	}

	while(1) {
		lv_task_handler();
		if (gettimeofday(&tv, NULL) < 0) {
			err(1, "gettimeofday");
		}
		timersub(&tv, &tv_prev, &tv_diff);
		tv_prev = tv;
		ticktime = tv_diff.tv_sec * 1000 + tv_diff.tv_usec / 1000;
		lv_tick_inc(ticktime);
		usleep(10 * 1000);
	}
}
