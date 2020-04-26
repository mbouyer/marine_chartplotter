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
#include <sys/time.h>
#include "lvgl/lvgl.h"
#include "edisplay.h"
#include "edisplay_data.h"
#include "edisplay_pages.h"
#include "edisplay_font.h"
#include "hal.h"
#include "lv_eslider.h"

static void page_list(void);
static void light_slide(edisp_page_t *);

void enc_group_focus(lv_obj_t *, bool);
void enc_group_defocus(lv_obj_t *);

lv_style_t style_large_text;
lv_style_t style_medium_text;
lv_style_t style_small_text;

edisp_page_t epage_retro = {
	NULL,
	light_slide,
	"retro-eclairage",
	false,
	true,
	NULL
};

static edisp_page_t *epages[] = {             
	&epage_autopilot,
	&epage_navdata,
	&epage_autoparams,
	&epage_retro,
}; 

static int current_page = 0;

static lv_obj_t *lv_top;
       lv_obj_t *lv_top_trs; /* transient top displays (menu, err msg, ...) */
static lv_group_t *encg;

static struct button {
	lv_obj_t *button;
	lv_obj_t *label;
} buttons[4];

static enum {
	OFF = 0,
	ON,
	REV
} backlight_status = ON;
static int backlight_pwm = 50;
static int old_backlight_pwm = 50;

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
	case REV:
		hal_set_backlight(1, 1, backlight_pwm);
		break;
	}
}

void
edisp_set_light(int mode)
{
	switch(mode) {
	case LIGHT_MODE_OFF:
		backlight_status = OFF;
		break;
	case LIGHT_MODE_ON:
		backlight_status = ON;
		break;
	case LIGHT_MODE_REV:
		backlight_status = REV;
		break;
	}
	set_backlight();
}

void
activate_page(edisp_page_t *epage)
{
	if (epage->epage_page != NULL) {
		lv_scr_load(epage->epage_page);
		enc_group_focus(epage->epage_page, false);
	}
}

void
deactivate_page(edisp_page_t *epage)
{
	if (epage->epage_page != NULL) {
		enc_group_defocus(epage->epage_page);
	}
}

static void
switch_to_page(int new)
{
	if (!epages[new]->epage_is_transient) {
		deactivate_page(epages[current_page]);
		current_page = new;
	}
	epages[new]->epage_activate(epages[new]);
}

void
switch_to_page_o(lv_obj_t *page)
{
	for (int i = 0; i < sizeof(epages) / sizeof(epages[0]); i++) {
		if (page == epages[i]->epage_page) {
			switch_to_page(i);
			return;
		}
	}
}

static void
disp_refresh(void *p)
{
	activate_page(epages[current_page]);
	set_backlight();
}

void
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
		// LV_KEY_LEFT, LV_KEY_RIGHT
		printf("LV_EVENT_KEY %d", *((uint32_t *)lv_event_get_data()));
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

static void
back_click_action(lv_obj_t * obj, lv_event_t event)
{

	switch(event) {
	case LV_EVENT_REFRESH:
		lv_async_call(disp_refresh, NULL);
		return;
	}
	printf("back event ");
	print_ev(event);
	printf("\n");
}

static void
btn_mob_click_action(lv_obj_t * btn, lv_event_t event)
{
	switch(event) {
	case LV_EVENT_LONG_PRESSED:
		if (!n2ks_control_mob()) 
			printf("MOB failed\n");
		return;
	case LV_EVENT_SHORT_CLICKED:
		edisp_autopilot_startstop(
		    epages[current_page] == &epage_autopilot);
		return;
	}
		
	printf("buttons[BUTTON_MOB].button event ");
	print_ev(event);
	printf("\n");
}

static void
btn_light_click_action(lv_obj_t * btn, lv_event_t event)
{

	   switch(event) {
	   case LV_EVENT_LONG_PRESSED:
		light_slide(&epage_retro);
		return;
	   case LV_EVENT_SHORT_CLICKED:
		switch (backlight_status) {
		case OFF:
			backlight_status = ON;
			n2ks_control_light_mode(LIGHT_MODE_ON);
			break;
		case ON:
			backlight_status = REV;
			n2ks_control_light_mode(LIGHT_MODE_REV);
			break;
		case REV:
			backlight_status = OFF;
			n2ks_control_light_mode(LIGHT_MODE_OFF);
			break;
		}
		set_backlight();
		return;
	   }
	   printf("buttons[BUTTON_LIGHT].button event ");
	   print_ev(event);
	printf("\n");
}

void
edisp_control_page(int move)
{
	int new_page;
	new_page = current_page;
	if (move > 0) {
		do {
			new_page++;
			if (new_page >= sizeof(epages) / sizeof(epages[0]))
				new_page = 0;
		} while (epages[new_page]->epage_in_page == false);
	} else if (move < 0) {
		do {
			new_page--;
			if (new_page < 0)
				new_page =
				    sizeof(epages) / sizeof(epages[0]) - 1;
		} while (epages[new_page]->epage_in_page == false);
	}
	switch_to_page(new_page);
}

static void
btn_page_click_action(lv_obj_t * btn, lv_event_t event)
{
	switch(event) {
	   case LV_EVENT_LONG_PRESSED:
		page_list();
		return;
	   case LV_EVENT_SHORT_CLICKED:
		edisp_control_page(1);
		return;
	}
		
	printf("buttons[BUTTON_PAGE].button event ");
	print_ev(event);
	printf("\n");
}

static void
btn_cancel_click_action(lv_obj_t * btn, lv_event_t event)
{

	switch(event) {
	   case LV_EVENT_SHORT_CLICKED:
		lv_group_send_data(encg, LV_KEY_ESC);
		return;
	}
	printf("buttons[BUTTON_CANCEL].button event ");
	print_ev(event);
	printf("\n");
}

void
enc_group_focus(lv_obj_t *obj, bool force)
{
	lv_group_set_editing(encg, false);
	lv_group_focus_freeze(encg, false);
	lv_group_add_obj(encg, obj);
	if (lv_group_get_focused(encg) == NULL || force)
		lv_group_focus_obj(obj);
	lv_group_set_editing(encg, true);
	lv_group_focus_freeze(encg, true);
}

void
enc_group_defocus(lv_obj_t *obj)
{
	lv_group_set_editing(encg, false);
	lv_group_focus_freeze(encg, false);
	lv_group_remove_obj(obj);
	if (lv_group_get_focused(encg) != NULL) {
		lv_group_set_editing(encg, true);
		lv_group_focus_freeze(encg, true);
	}
}

void
transient_open(lv_obj_t *obj)
{
	lv_obj_move_foreground(obj);
	enc_group_focus(obj, true);
}

void
transient_close(lv_obj_t *obj)
{
	enc_group_defocus(obj);
	lv_obj_del_async(obj);
}

lv_obj_t *
transient_list(const char *names, int select,
    void (*action)(lv_obj_t *, lv_event_t))
{
	lv_obj_t *list = lv_ddlist_create(lv_top_trs, NULL);
	lv_ddlist_set_options(list, names);
	lv_ddlist_set_selected(list, select);
	lv_obj_set_event_cb(list, action);
	lv_ddlist_set_stay_open(list, true);
	lv_coord_t h = lv_obj_get_height(list);
	transient_open(list);
	lv_obj_align(list, lv_top_trs, LV_ALIGN_CENTER, 0, -h/2 + 5);
	return list;
}

static void
light_action(lv_obj_t *slide, lv_event_t event)
{
	int key;
	switch(event) {
	case LV_EVENT_APPLY:
		transient_close(slide);
		break;
	case LV_EVENT_VALUE_CHANGED:
		backlight_pwm = lv_eslider_get_value(slide);
		set_backlight();
		break;
	case LV_EVENT_KEY:
		key = *((uint32_t *)lv_event_get_data());
		if (key == LV_KEY_ESC) {
			backlight_pwm = old_backlight_pwm;
			set_backlight();
			transient_close(slide);
			return;
		}
	default:
		printf("light_action: ");
		print_ev(event);
		printf("\n");
	}
}

static void
light_slide(edisp_page_t *epage)
{
	static const uint16_t light_steps[] = {10, 1};
	lv_obj_t *slide = lv_eslider_create(lv_top_trs, NULL);
	lv_eslider_set_range(slide, 10, 100);
	lv_eslider_set_value(slide, backlight_pwm, LV_ANIM_OFF);
	lv_eslider_set_steps(slide, light_steps, 2);
	lv_obj_set_event_cb(slide, light_action);
	transient_open(slide);
	lv_obj_align(slide, lv_top_trs, LV_ALIGN_CENTER, 0, 0);
	old_backlight_pwm = backlight_pwm;
	if (backlight_status == OFF) {
		backlight_status = ON;
		n2ks_control_light_mode(LIGHT_MODE_ON);
	}
}

static void
page_action(lv_obj_t *list, lv_event_t event)
{
	int key;
	int value;
	switch(event) {
	case LV_EVENT_VALUE_CHANGED:
		value = lv_ddlist_get_selected(list);
		transient_close(list);
		switch_to_page(value);
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
page_list(void)
{
	char pagesnames[100];

	pagesnames[0] = '\0';

	for (int i = 0; i < sizeof(epages) / sizeof(epages[0]); i++) {
		if (i > 0)
			strlcat(pagesnames, "\n", sizeof(pagesnames));
		strlcat(pagesnames, epages[i]->epage_menu, sizeof(pagesnames));
	}
	transient_list(pagesnames, current_page, page_action);

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

	buttons[BUTTON_MOB].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[BUTTON_MOB].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[BUTTON_MOB].button, btn_mob_click_action);
	lv_btn_set_style(buttons[BUTTON_MOB].button, LV_BTN_STYLE_REL,
	    &style_btn);
	lv_btn_set_style(buttons[BUTTON_MOB].button, LV_BTN_STYLE_PR,
	    &style_btn_pr);

	buttons[BUTTON_MOB].label =
	    lv_label_create(buttons[BUTTON_MOB].button, NULL);
	lv_label_set_static_text(buttons[BUTTON_MOB].label, "MoB");
	lv_coord_t h = lv_obj_get_height(buttons[BUTTON_MOB].button);
	lv_obj_align(buttons[BUTTON_MOB].button, NULL,
	    LV_ALIGN_OUT_TOP_LEFT, 0, h);

	buttons[BUTTON_LIGHT].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[BUTTON_LIGHT].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[BUTTON_LIGHT].button,
	    btn_light_click_action);
	lv_btn_set_style(buttons[BUTTON_LIGHT].button, LV_BTN_STYLE_REL,
	    &style_btn);
	lv_btn_set_style(buttons[BUTTON_LIGHT].button, LV_BTN_STYLE_PR,
	    &style_btn_pr);

	buttons[BUTTON_LIGHT].label =
	    lv_label_create(buttons[BUTTON_LIGHT].button, NULL);
	lv_label_set_static_text(buttons[BUTTON_LIGHT].label, "light");
	h = lv_obj_get_height(buttons[BUTTON_LIGHT].button);
	lv_obj_align(buttons[BUTTON_LIGHT].button, NULL,
	    LV_ALIGN_OUT_TOP_RIGHT, 0, h);

	buttons[BUTTON_PAGE].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[BUTTON_PAGE].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[BUTTON_PAGE].button, btn_page_click_action);
	lv_btn_set_style(buttons[BUTTON_PAGE].button, LV_BTN_STYLE_REL,
	    &style_btn);
	lv_btn_set_style(buttons[BUTTON_PAGE].button, LV_BTN_STYLE_PR,
	    &style_btn_pr);

	buttons[BUTTON_PAGE].label =
	    lv_label_create(buttons[BUTTON_PAGE].button, NULL);
	lv_label_set_static_text(buttons[BUTTON_PAGE].label, "page");
	h = lv_obj_get_height(buttons[BUTTON_PAGE].button);
	lv_obj_align(buttons[BUTTON_PAGE].button, NULL,
	    LV_ALIGN_OUT_BOTTOM_LEFT, 0, -h);

	buttons[BUTTON_CANCEL].button = lv_btn_create(lv_top, NULL);
	lv_cont_set_fit(buttons[BUTTON_CANCEL].button, LV_FIT_TIGHT);
	lv_obj_set_event_cb(buttons[BUTTON_CANCEL].button,
	    btn_cancel_click_action);
	lv_btn_set_style(buttons[BUTTON_CANCEL].button, LV_BTN_STYLE_REL,
	    &style_btn);
	lv_btn_set_style(buttons[BUTTON_CANCEL].button, LV_BTN_STYLE_PR,
	    &style_btn_pr);

	buttons[BUTTON_CANCEL].label =
	    lv_label_create(buttons[BUTTON_CANCEL].button, NULL);
	lv_label_set_static_text(buttons[BUTTON_CANCEL].label, "cancel");
	h = lv_obj_get_height(buttons[BUTTON_CANCEL].button);
	lv_obj_align(buttons[BUTTON_CANCEL].button, NULL,
	    LV_ALIGN_OUT_BOTTOM_RIGHT, 0, -h);
}

/* empty function for focus/edit style */
static void
empty_style_mod_cb(lv_group_t * group, lv_style_t * style)
{
}

/**
 * Create a edisplay application
 */
void
edisplay_app_init(void)
{
	int i;
	static lv_style_t style_page;

	lv_init();

	encg = lv_group_create();
	lv_group_set_refocus_policy(encg, LV_GROUP_REFOCUS_POLICY_PREV);

	hal_init(encg);

	lv_coord_t hres = lv_disp_get_hor_res(NULL);
	lv_coord_t vres = lv_disp_get_ver_res(NULL);
	lv_theme_t * th = lv_theme_mono_init(20, NULL);
	lv_theme_set_current(th);

	lv_group_set_style_mod_cb(encg, empty_style_mod_cb);
	lv_group_set_style_mod_edit_cb(encg, empty_style_mod_cb);

	lv_style_copy(&style_small_text, &lv_style_transp_tight);
	style_small_text.text.font = &lv_font_unscii_8;

	lv_style_copy(&style_medium_text, &lv_style_transp_tight);
	style_medium_text.text.font = &lib16;

	lv_style_copy(&style_large_text, &lv_style_transp_tight);
	style_large_text.text.font = &lib24;

	lv_top = lv_disp_get_layer_top(NULL);
	lv_obj_set_click(lv_top, 1);
	lv_obj_set_event_cb(lv_top, back_click_action);

	lv_top_trs = lv_obj_create(lv_top, lv_top);

	edisplay_buttons_create();

	lv_style_copy(&style_page, lv_obj_get_style(lv_scr_act()));
	
	for (i = 0; i < sizeof(epages) / sizeof(epages[0]); i++) {
		epages[i]->epage_page = lv_obj_create(NULL, NULL);
		lv_obj_set_style(epages[i]->epage_page, &style_page);
		if (epages[i]->epage_init != NULL)
			epages[i]->epage_init();
	}

	current_page = 0;
	epages[current_page]->epage_activate(epages[current_page]);
}

void
edisplay_app_run(void)
{
	struct timeval tv, tv_prev, tv_diff;
	time_t ticktime;

	if (gettimeofday(&tv_prev, NULL) < 0) {
		err(1, "gettimeofday");
	}

	while(1) {
		lv_task_handler();
		edisp_update_navdata();
		edisp_update_autopilot();
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
