/**
 * from lv_slider.c
 * @file lv_eslider.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "lv_eslider.h"


/*********************
 *      DEFINES
 *********************/
#define LV_ESLIDER_SIZE_MIN 4 /*hor. pad and ver. pad cannot make the bar or indicator smaller then this [px]*/
#define LV_ESLIDER_NOT_PRESSED INT16_MIN

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool lv_eslider_design(lv_obj_t * eslider, const lv_area_t * mask, lv_design_mode_t mode);
static lv_res_t lv_eslider_signal(lv_obj_t * eslider, lv_signal_t sign, void * param);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_design_cb_t ancestor_design_f;
static lv_signal_cb_t ancestor_signal;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Create a eslider objects
 * @param par pointer to an object, it will be the parent of the new eslider
 * @param copy pointer to a eslider object, if not NULL then the new object will be copied from it
 * @return pointer to the created eslider
 */
lv_obj_t *
lv_eslider_create(lv_obj_t * par, const lv_obj_t * copy)
{
    static const uint16_t _steps[] = {1};
    LV_LOG_TRACE("eslider create started");

    /*Create the ancestor eslider*/
    lv_obj_t * new_eslider = lv_bar_create(par, copy);
    lv_mem_assert(new_eslider);
    if(new_eslider == NULL) return NULL;

    if(ancestor_design_f == NULL) ancestor_design_f = lv_obj_get_design_cb(new_eslider);
    if(ancestor_signal == NULL) ancestor_signal = lv_obj_get_signal_cb(new_eslider);

    /*Allocate the eslider type specific extended data*/
    lv_eslider_ext_t * ext = lv_obj_allocate_ext_attr(new_eslider, sizeof(lv_eslider_ext_t));
    lv_mem_assert(ext);
    if(ext == NULL) return NULL;

    /*Initialize the allocated 'ext' */
    ext->es_steps = _steps;
    ext->es_nsteps = 1;
    ext->es_cstep = 0;

    /*The signal and design functions are not copied so set them here*/
    lv_obj_set_signal_cb(new_eslider, lv_eslider_signal);
    lv_obj_set_design_cb(new_eslider, lv_eslider_design);

    /*Init the new eslider eslider*/
    if(copy == NULL) {
	lv_obj_set_click(new_eslider, true);
	lv_obj_set_protect(new_eslider, LV_PROTECT_PRESS_LOST);

	/*Set the default styles*/
	lv_theme_t * th = lv_theme_get_current();
	if(th) {
	    lv_eslider_set_style(new_eslider, LV_ESLIDER_STYLE_BG, th->style.slider.bg);
	    lv_eslider_set_style(new_eslider, LV_ESLIDER_STYLE_INDIC, th->style.slider.indic);
	}
    }
    /*Copy an existing eslider*/
    else {
	lv_eslider_ext_t * copy_ext = lv_obj_get_ext_attr(copy);
	/*Refresh the style with new signal function*/
	lv_obj_refresh_style(new_eslider);
    }

    LV_LOG_INFO("eslider created");

    return new_eslider;
}

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a style of a eslider
 * @param eslider pointer to a eslider object
 * @param type which style should be set
 * @param style pointer to a style
 */
void
lv_eslider_set_style(lv_obj_t * eslider, lv_eslider_style_t type, const lv_style_t * style)
{
    lv_eslider_ext_t * ext = lv_obj_get_ext_attr(eslider);

    switch(type) {
	case LV_ESLIDER_STYLE_BG: lv_bar_set_style(eslider, LV_BAR_STYLE_BG, style); break;
	case LV_ESLIDER_STYLE_INDIC: lv_bar_set_style(eslider, LV_BAR_STYLE_INDIC, style); break;
    }
}

/**
 * Set steps array of a eslider
 * @param eslider pointer to a eslider object
 * @param array of steps
 * @param number of steps
 */
void lv_eslider_set_steps(lv_obj_t * eslider, const uint16_t *steps, int n)
{
    lv_eslider_ext_t * ext = lv_obj_get_ext_attr(eslider);
    ext->es_steps = steps;
    ext->es_nsteps = n;
}

/**
 * Set current step index
 * @param eslider pointer to a eslider object
 * @param index 
 */
void lv_eslider_set_cteps(lv_obj_t * eslider, int n)
{
    lv_eslider_ext_t * ext = lv_obj_get_ext_attr(eslider);
    if (n >= ext->es_nsteps || n < 0)
	return;
    ext->es_cstep = n;
    lv_obj_invalidate(eslider);
}


/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a eslider
 * @param eslider pointer to a eslider object
 * @return the value of the eslider
 */
int16_t
lv_eslider_get_value(const lv_obj_t * eslider)
{
    lv_eslider_ext_t * ext = lv_obj_get_ext_attr(eslider);

    return lv_bar_get_value(eslider);
}

/**
 * Get a style of a eslider
 * @param eslider pointer to a eslider object
 * @param type which style should be get
 * @return style pointer to a style
 */
const lv_style_t *
lv_eslider_get_style(const lv_obj_t * eslider, lv_eslider_style_t type)
{
    const lv_style_t * style = NULL;
    lv_eslider_ext_t * ext    = lv_obj_get_ext_attr(eslider);

    switch(type) {
	case LV_ESLIDER_STYLE_BG: style = lv_bar_get_style(eslider, LV_BAR_STYLE_BG); break;
	case LV_ESLIDER_STYLE_INDIC: style = lv_bar_get_style(eslider, LV_BAR_STYLE_INDIC); break;
	default: style = NULL; break;
    }

    return style;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Handle the drawing related tasks of the esliders
 * @param eslider pointer to an object
 * @param mask the object will be drawn only in this area
 * @param mode LV_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *				  (return 'true' if yes)
 *	     LV_DESIGN_DRAW: draw the object (always return 'true')
 *	     LV_DESIGN_DRAW_POST: drawing after every children are drawn
 * @param return true/false, depends on 'mode'
 */
static bool
lv_eslider_design(lv_obj_t * eslider, const lv_area_t * mask,
    lv_design_mode_t mode)
{
    /*Return false if the object is not covers the mask_p area*/
    if(mode == LV_DESIGN_COVER_CHK) {
	return false;
    }
    /*Draw the object*/
    else if(mode == LV_DESIGN_DRAW_MAIN) {
	lv_eslider_ext_t * ext = lv_obj_get_ext_attr(eslider);

	const lv_style_t * style_bg    = lv_eslider_get_style(eslider, LV_ESLIDER_STYLE_BG);
	const lv_style_t * style_indic = lv_eslider_get_style(eslider, LV_ESLIDER_STYLE_INDIC);

	lv_opa_t opa_scale = lv_obj_get_opa_scale(eslider);

	lv_coord_t es_w = lv_area_get_width(&eslider->coords);
	lv_coord_t es_h = lv_area_get_height(&eslider->coords);

	bool vertical = (es_h > es_w);

	/* get size of labels */
	lv_coord_t cur_value = lv_eslider_get_value(eslider);
	lv_coord_t min_value = lv_eslider_get_min_value(eslider);
	lv_coord_t max_value = lv_eslider_get_max_value(eslider);

	lv_point_t value_size;
	lv_point_t step_size = {0};
	char value_txt[16];
	char step_txt[16];
	snprintf(value_txt, sizeof(value_txt), "%d", max_value);
	lv_txt_get_size(&value_size, value_txt, style_bg->text.font,
	    style_bg->text.letter_space, style_bg->text.line_space,
	    LV_COORD_MAX, LV_TXT_FLAG_NONE);
	for (int i = 0; i < ext->es_nsteps; i++) {
		lv_point_t tmp_size;
		snprintf(step_txt, sizeof(step_txt), "x%d", ext->es_steps[i]);
		lv_txt_get_size(&tmp_size, step_txt, style_bg->text.font,
		    style_bg->text.letter_space, style_bg->text.line_space,
		    LV_COORD_MAX, LV_TXT_FLAG_NONE);
		if (tmp_size.x > step_size.x)
			step_size.x = tmp_size.x;
		if (tmp_size.y > step_size.y)
			step_size.y = tmp_size.y;
	}

	/* start drawing */
	lv_area_t area_bg;
	lv_area_copy(&area_bg, &eslider->coords);

	/*Be sure at least LV_ESLIDER_SIZE_MIN  size will remain*/
	lv_coord_t pad_top_bg    = style_bg->body.padding.top;
	lv_coord_t pad_bottom_bg = style_bg->body.padding.bottom;
	lv_coord_t pad_left_bg   = style_bg->body.padding.left;
	lv_coord_t pad_right_bg  = style_bg->body.padding.right;
	if(pad_top_bg + pad_bottom_bg + LV_ESLIDER_SIZE_MIN > lv_area_get_height(&area_bg)) {
	    pad_top_bg    = (lv_area_get_height(&area_bg) - LV_ESLIDER_SIZE_MIN) >> 1;
	    pad_bottom_bg = pad_top_bg;
	}
	if(pad_left_bg + pad_right_bg + LV_ESLIDER_SIZE_MIN > lv_area_get_width(&area_bg)) {
	    pad_left_bg  = (lv_area_get_width(&area_bg) - LV_ESLIDER_SIZE_MIN) >> 1;
	    pad_right_bg = (lv_area_get_width(&area_bg) - LV_ESLIDER_SIZE_MIN) >> 1;
	}

	area_bg.x1 += vertical ? pad_left_bg : 0;   /*Pad only for vertical eslider*/
	area_bg.x2 -= vertical ? pad_right_bg : 0;  /*Pad only for vertical eslider*/
	area_bg.y1 += !vertical  ? pad_top_bg : 0;    /*Pad only for horizontal eslider*/
	area_bg.y2 -= !vertical ? pad_bottom_bg : 0; /*Pad only for horizontal eslider*/

	/* clear background */
	lv_style_t style_tmp;
	lv_style_copy(&style_tmp, style_bg);
	style_tmp.body.border.width = 0;
	style_tmp.body.radius = 0;
	lv_draw_rect(&area_bg, mask, &style_tmp, lv_obj_get_opa_scale(eslider));

	/* draw the value */
	lv_area_t area_v;
	lv_area_copy(&area_v, &area_bg);

	// printf("txt %dx%d area %d, %d->%d, %d\n", value_size.x, value_size.y, area_bg.x1, area_bg.y1, area_bg.x2, area_bg.y2);
	area_v.x2 = area_v.x1 + value_size.x;
	int middle = (area_v.y1 + area_v.y2) / 2;
	area_v.y1 = middle - value_size.y / 2;
	area_v.y2 = middle + value_size.y / 2;
	// printf("txt middle %d txt area %d %d -> %d %d\n", middle, area_v.x1, area_v.y1, area_v.x2, area_v.y2);
	snprintf(value_txt, sizeof(value_txt), "%d", cur_value);
	lv_draw_label(&area_v, mask, style_bg, lv_obj_get_opa_scale(eslider),
	    value_txt, LV_TXT_FLAG_NONE, NULL, -1, -1, NULL);

	area_bg.x1 += value_size.x;
	area_bg.x1 = area_v.x2 + style_bg->body.padding.inner;

	/* draw the steps */
	lv_area_t area_s;
	lv_area_copy(&area_s, &area_bg);

	// printf("txt %dx%d area %d, %d->%d, %d\n", step_size.x, step_size.y, area_bg.x1, area_bg.y1, area_bg.x2, area_bg.y2);
	area_s.x1 = area_s.x2 - step_size.x;
	middle = (area_s.y1 + area_s.y2) / 2;
	area_s.y1 = middle - step_size.y / 2;
	area_s.y2 = middle + step_size.y / 2;
	// printf("txt middle %d txt area %d %d -> %d %d\n", middle, area_s.x1, area_s.y1, area_s.x2, area_s.y2);
	snprintf(step_txt, sizeof(step_txt), "x%d", ext->es_steps[ext->es_cstep]);
	lv_draw_label(&area_s, mask, style_bg, lv_obj_get_opa_scale(eslider),
	    step_txt, LV_TXT_FLAG_NONE, NULL, -1, -1, NULL);

	area_bg.x2 = area_s.x1 - style_bg->body.padding.inner;

#if LV_USE_GROUP == 0
	lv_draw_rect(&area_bg, mask, style_bg, lv_obj_get_opa_scale(eslider));
#else
	/* Draw the borders later if the eslider is focused.
	 * At value = 100% the indicator can cover to whole background and the focused style won't
	 * be visible*/
	if(lv_obj_is_focused(eslider)) {
	    lv_style_copy(&style_tmp, style_bg);
	    style_tmp.body.border.width = 0;
	    lv_draw_rect(&area_bg, mask, &style_tmp, opa_scale);
	} else {
	    lv_draw_rect(&area_bg, mask, style_bg, opa_scale);
	}
#endif
	/*Draw the indicator*/
	lv_area_t area_indic;
	lv_area_copy(&area_indic, &area_bg);

	/*Be sure at least ver pad/hor pad width indicator will remain*/
	lv_coord_t pad_top_indic    = style_indic->body.padding.top;
	lv_coord_t pad_bottom_indic = style_indic->body.padding.bottom;
	lv_coord_t pad_left_indic   = style_indic->body.padding.left;
	lv_coord_t pad_right_indic  = style_indic->body.padding.right;
	if(pad_top_indic + pad_bottom_indic + LV_ESLIDER_SIZE_MIN > lv_area_get_height(&area_bg)) {
	    pad_top_indic    = (lv_area_get_height(&area_bg) - LV_ESLIDER_SIZE_MIN) >> 1;
	    pad_bottom_indic = pad_top_indic;
	}
	if(pad_left_indic + pad_right_indic + LV_ESLIDER_SIZE_MIN > lv_area_get_width(&area_bg)) {
	    pad_left_indic  = (lv_area_get_width(&area_bg) - LV_ESLIDER_SIZE_MIN) >> 1;
	    pad_right_indic = pad_left_indic;
	}

	area_indic.x1 += pad_left_indic;
	area_indic.x2 -= pad_right_indic;
	area_indic.y1 += pad_top_indic;
	area_indic.y2 -= pad_bottom_indic;

	if(!vertical) {
	    lv_coord_t indic_w = lv_area_get_width(&area_indic);
#if LV_USE_ANIMATION
	    if(ext->bar.anim_state != LV_BAR_ANIM_STATE_INV) {
		/*Calculate the coordinates of anim. start and end*/
		lv_coord_t anim_start_x =
		    (int32_t)((int32_t)indic_w * (ext->bar.anim_start - min_value)) / (max_value - min_value);
		lv_coord_t anim_end_x =
		    (int32_t)((int32_t)indic_w * (ext->bar.anim_end - min_value)) / (max_value - min_value);

		/*Calculate the real position based on `anim_state` (between `anim_start` and
		 * `anim_end`)*/
		area_indic.x2 = anim_start_x + (((anim_end_x - anim_start_x) * ext->bar.anim_state) >> 8);
	    } else
#endif
	    {
		area_indic.x2 = (int32_t)((int32_t)indic_w * (cur_value - min_value)) / (max_value - min_value);
	    }
	    area_indic.x2 = area_indic.x1 + area_indic.x2 - 1;

	    /*Draw the indicator but don't draw an ugly 1px wide rectangle on the left on min.
	     * value*/
	    if(area_indic.x1 != area_indic.x2) lv_draw_rect(&area_indic, mask, style_indic, opa_scale);

	} else {
	    lv_coord_t indic_h = lv_area_get_height(&area_indic);
#if LV_USE_ANIMATION
	    if(ext->bar.anim_state != LV_BAR_ANIM_STATE_INV) {
		/*Calculate the coordinates of anim. start and end*/
		lv_coord_t anim_start_y =
		    (int32_t)((int32_t)indic_h * (ext->bar.anim_start - min_value)) / (max_value - min_value);
		lv_coord_t anim_end_y =
		    (int32_t)((int32_t)indic_h * (ext->bar.anim_end - min_value)) / (max_value - min_value);

		/*Calculate the real position based on `anim_state` (between `anim_start` and
		 * `anim_end`)*/
		area_indic.y1 = anim_start_y + (((anim_end_y - anim_start_y) * ext->bar.anim_state) >> 8);
	    } else
#endif
	    {
		area_indic.y1 = (int32_t)((int32_t)indic_h * (cur_value - min_value)) / (max_value - min_value);
	    }
	    area_indic.y1 = area_indic.y2 - area_indic.y1 + 1;

	    /*Draw the indicator but don't draw an ugly 1px height rectangle on the bottom on min.
	     * value*/
	    if(area_indic.x1 != area_indic.x2) lv_draw_rect(&area_indic, mask, style_indic, opa_scale);
	}

	/* add the border if required*/
#if LV_USE_GROUP
	/* Draw the borders later if the bar is focused.
	 * At value = 100% the indicator can cover to whole background and the focused style won't
	 * be visible*/
	if(lv_obj_is_focused(eslider)) {
	    lv_style_copy(&style_tmp, style_bg);
	    style_tmp.body.opa	  = LV_OPA_TRANSP;
	    style_tmp.body.shadow.width = 0;
	    lv_draw_rect(&area_bg, mask, &style_tmp, opa_scale);
	}
#endif
    }
    /*Post draw when the children are drawn*/
    else if(mode == LV_DESIGN_DRAW_POST) {
    }

    return true;
}

/**
 * Signal function of the eslider
 * @param eslider pointer to a eslider object
 * @param sign a signal type from lv_signal_t enum
 * @param param pointer to a signal specific variable
 * @return LV_RES_OK: the object is not deleted in the function; LV_RES_INV: the object is deleted
 */
static lv_res_t
lv_eslider_signal(lv_obj_t * eslider, lv_signal_t sign, void * param)
{
    lv_res_t res;

    /* Include the ancient signal function */
    res = ancestor_signal(eslider, sign, param);
    if(res != LV_RES_OK) return res;

    lv_eslider_ext_t * ext = lv_obj_get_ext_attr(eslider);
    lv_point_t p;
    lv_coord_t w = lv_obj_get_width(eslider);
    lv_coord_t h = lv_obj_get_height(eslider);

    if (sign == LV_SIGNAL_CONTROL) {
	char c = *((char *)param);

	if(c == LV_KEY_RIGHT || c == LV_KEY_UP) {
	    lv_eslider_set_value(eslider,
		lv_eslider_get_value(eslider) + ext->es_steps[ext->es_cstep],
		true);
	    res = lv_event_send(eslider, LV_EVENT_VALUE_CHANGED, NULL);
	    if(res != LV_RES_OK) return res;
	} else if(c == LV_KEY_LEFT || c == LV_KEY_DOWN) {
	    lv_eslider_set_value(eslider,
		lv_eslider_get_value(eslider) - ext->es_steps[ext->es_cstep],
		true);
	    res = lv_event_send(eslider, LV_EVENT_VALUE_CHANGED, NULL);
	    if(res != LV_RES_OK) return res;
	} else if (c == LV_KEY_ENTER) {
	    int c = ext->es_cstep + 1;
	    if (c >= ext->es_nsteps)
		    c = 0;
	    lv_eslider_set_cteps(eslider, c);
	    res = lv_event_send(eslider, LV_EVENT_VALUE_CHANGED, NULL);
	    if(res != LV_RES_OK) return res;

	    if (c == 0) {
		    res = lv_event_send(eslider, LV_EVENT_APPLY, NULL);
		    if(res != LV_RES_OK) return res;
	    }
	}
    } else if(sign == LV_SIGNAL_GET_EDITABLE) {
	bool * editable = (bool *)param;
	*editable       = true;
    } else if(sign == LV_SIGNAL_GET_TYPE) {
	lv_obj_type_t * buf = param;
	uint8_t i;
	for(i = 0; i < LV_MAX_ANCESTOR_NUM - 1; i++) { /*Find the last set data*/
	   if(buf->type[i] == NULL) break;
	}
	buf->type[i] = "lv_eslider";
    } else {
	printf("eslider: signal %d\n", sign);
    }

    return res;
}
