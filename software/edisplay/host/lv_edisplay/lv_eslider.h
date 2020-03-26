/**
 * from lv_slider.h
 * @file lv_eslider.h
 *
 */

#ifndef LV_ESLIDER_H
#define LV_ESLIDER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../lv_conf.h"
#endif

/*Testing of dependencies*/
#if LV_USE_BAR == 0
#error "lv_eslider: lv_bar is required. Enable it in lv_conf.h (LV_USE_BAR  1) "
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
/*Data of eslider*/
typedef struct
{
    lv_bar_ext_t bar; /*Ext. of ancestor*/
    /*New data for this type */
    const uint16_t *es_steps;
    int es_nsteps;
    int es_cstep;
} lv_eslider_ext_t;

/** Built-in styles of eslider*/
enum {
    LV_ESLIDER_STYLE_BG, /** Slider background style. */
    LV_ESLIDER_STYLE_INDIC, /** Slider indicator (filled area) style. */
};
typedef uint8_t lv_eslider_style_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Create a eslider objects
 * @param par pointer to an object, it will be the parent of the new eslider
 * @param copy pointer to a eslider object, if not NULL then the new object will be copied from it
 * @return pointer to the created eslider
 */
lv_obj_t * lv_eslider_create(lv_obj_t * par, const lv_obj_t * copy);

/*=====================
 * Setter functions
 *====================*/

/**
 * Set a new value on the eslider
 * @param eslider pointer to a eslider object
 * @param value new value
 * @param anim LV_ANIM_ON: set the value with an animation; LV_ANIM_OFF: change the value immediately
 */
static inline void lv_eslider_set_value(lv_obj_t * eslider, int16_t value, lv_anim_enable_t anim)
{
    lv_bar_set_value(eslider, value, anim);
}

/**
 * Set minimum and the maximum values of a bar
 * @param eslider pointer to the eslider object
 * @param min minimum value
 * @param max maximum value
 */
static inline void lv_eslider_set_range(lv_obj_t * eslider, int16_t min, int16_t max)
{
    lv_bar_set_range(eslider, min, max);
}

/**
 * Set the animation time of the eslider
 * @param eslider pointer to a bar object
 * @param anim_time the animation time in milliseconds.
 */
static inline void lv_eslider_set_anim_time(lv_obj_t * eslider, uint16_t anim_time)
{
    lv_bar_set_anim_time(eslider, anim_time);
}

/**
 * Set a style of a eslider
 * @param eslider pointer to a eslider object
 * @param type which style should be set
 * @param style pointer to a style
 */
void lv_eslider_set_style(lv_obj_t * eslider, lv_eslider_style_t type, const lv_style_t * style);

/**
 * Set steps array of a eslider
 * @param eslider pointer to a eslider object
 * @param array of steps
 * @param number of steps
 */
void lv_eslider_set_steps(lv_obj_t * eslider, const uint16_t *steps, int n);

/**
 * Set current step index
 * @param eslider pointer to a eslider object
 * @param index
 */
void lv_eslider_set_cteps(lv_obj_t * eslider, int n);

/*=====================
 * Getter functions
 *====================*/

/**
 * Get the value of a eslider
 * @param eslider pointer to a eslider object
 * @return the value of the eslider
 */
int16_t lv_eslider_get_value(const lv_obj_t * eslider);

/**
 * Get the minimum value of a eslider
 * @param eslider pointer to a eslider object
 * @return the minimum value of the eslider
 */
static inline int16_t lv_eslider_get_min_value(const lv_obj_t * eslider)
{
    return lv_bar_get_min_value(eslider);
}

/**
 * Get the maximum value of a eslider
 * @param eslider pointer to a eslider object
 * @return the maximum value of the eslider
 */
static inline int16_t lv_eslider_get_max_value(const lv_obj_t * eslider)
{
    return lv_bar_get_max_value(eslider);
}

/**
 * Get a style of a eslider
 * @param eslider pointer to a eslider object
 * @param type which style should be get
 * @return style pointer to a style
 */
const lv_style_t * lv_eslider_get_style(const lv_obj_t * eslider, lv_eslider_style_t type);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_ESLIDER_H*/
