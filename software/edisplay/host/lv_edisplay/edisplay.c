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
#include "edisplay.h"
#include "edisplay_pages.h"
#include "edisplay_font.h"
#include "hal.h"

static void page_list(void);
static void light_slide(edisp_page_t *);

lv_style_t style_large_text;
lv_style_t style_medium_text;
lv_style_t style_small_text;

edisp_page_t epage_retro = {
	NULL,
	light_slide,
	"retro-eclairage",
	false,
	NULL
};

static edisp_page_t *epages[] = {             
	&epage_autopilot,
	&epage_navdata,
	&epage_retro,
}; 

static int current_page = 0;

static lv_obj_t *lv_top;
static lv_obj_t *lv_top_trs; /* transient top displays (menu, err msg, ...) */
static lv_group_t *encg;

static struct button {
	lv_obj_t *button;
	lv_obj_t *label;
} buttons[4];

static enum {
	OFF = 0,
	ON,
	INV
} backlight_status = ON;
static int backlight_pwm = 50;

static void
set_backlight(void)
{
	switch (backlight_status) {
	case OFF:
		hal_set_backlight(0, 0, backlight_pwm);
		break;
	case ON:
		hal_set_backlight(1, 0, backlight_pwm);
		break;
	case INV:
		hal_set_backlight(1, 1, backlight_pwm);
		break;
	}
}

void
activate_page(edisp_page_t *epage)
{
	if (epage->epage_page != NULL) {
		lv_scr_load(epage->epage_page);
	}
}

static void
disp_refresh(void *p)
{
	activate_page(epages[current_page]);
	set_backlight();
}

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
	case LV_EVENT_REFRESH:
		lv_async_call(disp_refresh, NULL);
		return;
	}
	printf("back event ");
	print_ev(event);
	printf("\n");
}

static void
btn1_click_action(lv_obj_t * btn, lv_event_t event)
{

	   printf("buttons[0].button event ");
	   print_ev(event);
	printf("\n");
}

static void
btn2_click_action(lv_obj_t * btn, lv_event_t event)
{

	   switch(event) {
	   case LV_EVENT_LONG_PRESSED:
		light_slide(&epage_retro);
		return;
	   case LV_EVENT_SHORT_CLICKED:
		switch (backlight_status) {
		case OFF:
			backlight_status = ON;
			break;
		case ON:
			backlight_status = INV;
			break;
		case INV:
			backlight_status = OFF;
			break;
		}
		set_backlight();
		return;
	   }
	   printf("buttons[1].button event ");
	   print_ev(event);
	printf("\n");
}

static void
btn3_click_action(lv_obj_t * btn, lv_event_t event)
{

	   printf("buttons[2].button event ");
	   print_ev(event);
	printf("\n");
}

static void
btn4_click_action(lv_obj_t * btn, lv_event_t event)
{

	   printf("buttons[3].button event ");
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
		enc_group_close(slide);
		break;
	case LV_EVENT_VALUE_CHANGED:
		backlight_pwm = lv_slider_get_value(slide);
		set_backlight();
		break;
	default:
		   printf("light_action: ");
		   print_ev(event);
		   printf("\n");
	}
}

static void
light_slide(edisp_page_t *epage)
{
	lv_obj_t *slide = lv_slider_create(lv_top_trs, NULL);
	lv_slider_set_range(slide, 10, 100);
	lv_slider_set_value(slide, backlight_pwm, LV_ANIM_OFF);
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
		if (epages[value] != &epage_retro) 
			current_page = value;
		epages[value]->epage_activate(epages[value]);
		break;
		}
	}
}

static void
page_list(void)
{
	char pagesnames[100];

	pagesnames[0] = '\0';

	for (int i = 0; i < sizeof(epages) / sizeof(epages[0]); i++) {
		if (i > 0)
			strlcat(pagesnames, "\n", sizeof(pagesnames));
		strlcat(pagesnames, epages[i]->epage_menu, sizeof(pagesnames));
	}

	lv_obj_t *list = lv_ddlist_create(lv_top_trs, NULL);
	lv_ddlist_set_options(list, pagesnames);
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

	buttons[0].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[0].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[0].button, btn1_click_action);
	lv_btn_set_style(buttons[0].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[0].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[0].label = lv_label_create(buttons[0].button, NULL);
	lv_label_set_static_text(buttons[0].label, "MoB");
	lv_coord_t w = lv_obj_get_width(buttons[0].button);
	lv_coord_t h = lv_obj_get_height(buttons[0].button);
	lv_obj_align(buttons[0].button, NULL, LV_ALIGN_OUT_TOP_LEFT, 0, h);

	buttons[1].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[1].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[1].button, btn2_click_action);
	lv_btn_set_style(buttons[1].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[1].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[1].label = lv_label_create(buttons[1].button, NULL);
	lv_label_set_static_text(buttons[1].label, "light");
	w = lv_obj_get_width(buttons[1].button);
	h = lv_obj_get_height(buttons[1].button);
	lv_obj_align(buttons[1].button, NULL, LV_ALIGN_OUT_TOP_RIGHT, 0, h);

	buttons[2].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[2].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[2].button, btn3_click_action);
	lv_btn_set_style(buttons[2].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[2].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[2].label = lv_label_create(buttons[2].button, NULL);
	lv_label_set_static_text(buttons[2].label, "page");
	w = lv_obj_get_width(buttons[2].button);
	h = lv_obj_get_height(buttons[2].button);
	lv_obj_align(buttons[2].button, NULL, LV_ALIGN_OUT_BOTTOM_LEFT, 0, -h);

	buttons[3].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[3].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[3].button, btn4_click_action);
	lv_btn_set_style(buttons[3].button, LV_BTN_STYLE_REL, &style_btn);
	lv_btn_set_style(buttons[3].button, LV_BTN_STYLE_PR, &style_btn_pr);

	buttons[3].label = lv_label_create(buttons[3].button, NULL);
	lv_label_set_static_text(buttons[3].label, "cancel");
	w = lv_obj_get_width(buttons[3].button);
	h = lv_obj_get_height(buttons[3].button);
	lv_obj_align(buttons[3].button, NULL, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -h);
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

	lv_style_copy(&style_small_text, &lv_style_transp_tight);
	style_small_text.text.font = &lv_font_unscii_8;

	lv_style_copy(&style_medium_text, &lv_style_transp_tight);
	style_medium_text.text.font = &lib16;

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
	
	for (i = 0; i < sizeof(epages) / sizeof(epages[0]); i++) {
		epages[i]->epage_page = lv_obj_create(NULL, NULL);
		lv_obj_set_style(epages[i]->epage_page,
		    lv_obj_get_style(lv_scr_act()));
		if (epages[i]->epage_init != NULL)
			epages[i]->epage_init();
	}

	current_page = 0;
	epages[current_page]->epage_activate(epages[current_page]);
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
