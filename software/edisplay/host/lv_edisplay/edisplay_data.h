/**
 * @file edisplay_data.h
 *
 */

#ifndef EDISPLAY_DATA_H
#define EDISPLAY_DATA_H

#ifdef __cplusplus
extern "C" {
#endif

void edisp_set_attitude(int, int);
void edisp_set_cogsog(int, int);
void edisp_set_xte(int);
void edisp_set_navdata(int, int);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*EDISPLAY_DATA_H*/
