# Copyright (c) 2019 Manuel Bouyer
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
# ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

CMAKE_MINIMUM_REQUIRED(VERSION 3.1.1)

PROJECT(wxpilot)

SET(CMAKE_VERBOSE_MAKEFILE ON)
message(STATUS "cmake version: ${CMAKE_VERSION}")

SET(CMAKE_BUILD_TYPE Debug)

SET(PACKAGE_NAME edisplay)
SET(CMAKE_MODULE_PATH 
  ${CMAKE_SOURCE_DIR}/build
  ${CMAKE_SOURCE_DIR}/
  ${CMAKE_SOURCE_DIR}/cmake
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if (MONITOR) 
  find_package(SDL2 REQUIRED SDL2)
  SET(DRVHDRS
    lv_drivers/display/monitor.h
    lv_drivers/indev/mouse.h
    lv_drivers/indev/mousewheel.h
  )
  SET(DRVSRCS
    lv_drivers/display/monitor.c
    lv_drivers/indev/mouse.c
    lv_drivers/indev/mousewheel.c
    lv_edisplay/hal_sdl.c
  )
else(MONITOR)
  find_path(LIBUSB_INCLUDE_DIR
    NAMES libusb.h
    PATH_SUFFIXES "include" "libusb" "libusb-1.0")
  find_library(LIBUSB_LIBRARY
     NAMES usb-1.0
     PATH_SUFFIXES "lib" "lib32" "lib64")
  SET(DRVHDRS
    lv_drivers/display/edisplay.h
    lv_drivers/indev/edisplay_buttons.h
    ../pic/comms.h
  )
  SET(DRVSRCS
    lv_drivers/display/edisplay.c
    lv_drivers/indev/edisplay_buttons.c
    lv_edisplay/hal_edisplay.c
  )
endif(MONITOR)

set (CMAKE_CXX_STANDARD 11)

SET(PKGINC /usr/pkg/include)
SET(PKGLIB /usr/pkg/lib)

INCLUDE_DIRECTORIES(${PKGINC} ${SDL2_INCLUDE_DIRS} ${LIBUSB_INCLUDE_DIR})
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${PKGLIB} -Wl,-R${PKGLIB}")


MESSAGE (STATUS "*** Will install to ${CMAKE_INSTALL_PREFIX}  ***")
SET(PREFIX_BIN bin)
SET(PREFIX_INCLUDE include)
SET(PREFIX_DATA share)
SET(PREFIX_PKGDATA ${PREFIX_DATA}/${PACKAGE_NAME})
SET(PREFIX_LIB "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

SET(N2KHDRS
    N2K/NMEA2000.h
    N2K/nmea2000_defs_rx.h
    N2K/nmea2000_frame.h
    N2K/nmea2000_defs.h
    N2K/nmea2000_defs_tx.h
)

SET(LVHDRS
    lvgl/lvgl.h
    lvgl/src/lv_conf_checker.h
    lvgl/src/lv_core/lv_disp.h
    lvgl/src/lv_core/lv_group.h
    lvgl/src/lv_core/lv_indev.h
    lvgl/src/lv_core/lv_obj.h
    lvgl/src/lv_core/lv_refr.h
    lvgl/src/lv_core/lv_style.h
    lvgl/src/lv_draw/lv_draw.h
    lvgl/src/lv_draw/lv_draw_arc.h
    lvgl/src/lv_draw/lv_draw_basic.h
    lvgl/src/lv_draw/lv_draw_img.h
    lvgl/src/lv_draw/lv_draw_label.h
    lvgl/src/lv_draw/lv_draw_line.h
    lvgl/src/lv_draw/lv_draw_rect.h
    lvgl/src/lv_draw/lv_draw_triangle.h
    lvgl/src/lv_draw/lv_img_cache.h
    lvgl/src/lv_draw/lv_img_decoder.h
    lvgl/src/lv_font/lv_font.h
    lvgl/src/lv_font/lv_font_fmt_txt.h
    lvgl/src/lv_font/lv_symbol_def.h
    lvgl/src/lv_hal/lv_hal.h
    lvgl/src/lv_hal/lv_hal_disp.h
    lvgl/src/lv_hal/lv_hal_indev.h
    lvgl/src/lv_hal/lv_hal_tick.h
    lvgl/src/lv_misc/lv_anim.h
    lvgl/src/lv_misc/lv_area.h
    lvgl/src/lv_misc/lv_async.h
    lvgl/src/lv_misc/lv_circ.h
    lvgl/src/lv_misc/lv_color.h
    lvgl/src/lv_misc/lv_fs.h
    lvgl/src/lv_misc/lv_gc.h
    lvgl/src/lv_misc/lv_ll.h
    lvgl/src/lv_misc/lv_log.h
    lvgl/src/lv_misc/lv_math.h
    lvgl/src/lv_misc/lv_mem.h
    lvgl/src/lv_misc/lv_task.h
    lvgl/src/lv_misc/lv_templ.h
    lvgl/src/lv_misc/lv_txt.h
    lvgl/src/lv_misc/lv_types.h
    lvgl/src/lv_misc/lv_utils.h
    lvgl/src/lv_objx/lv_arc.h
    lvgl/src/lv_objx/lv_bar.h
    lvgl/src/lv_objx/lv_btn.h
    lvgl/src/lv_objx/lv_btnm.h
    lvgl/src/lv_objx/lv_calendar.h
    lvgl/src/lv_objx/lv_canvas.h
    lvgl/src/lv_objx/lv_cb.h
    lvgl/src/lv_objx/lv_chart.h
    lvgl/src/lv_objx/lv_cont.h
    lvgl/src/lv_objx/lv_ddlist.h
    lvgl/src/lv_objx/lv_gauge.h
    lvgl/src/lv_objx/lv_img.h
    lvgl/src/lv_objx/lv_imgbtn.h
    lvgl/src/lv_objx/lv_kb.h
    lvgl/src/lv_objx/lv_label.h
    lvgl/src/lv_objx/lv_led.h
    lvgl/src/lv_objx/lv_line.h
    lvgl/src/lv_objx/lv_list.h
    lvgl/src/lv_objx/lv_lmeter.h
    lvgl/src/lv_objx/lv_mbox.h
    lvgl/src/lv_objx/lv_objx_templ.h
    lvgl/src/lv_objx/lv_page.h
    lvgl/src/lv_objx/lv_preload.h
    lvgl/src/lv_objx/lv_roller.h
    lvgl/src/lv_objx/lv_slider.h
    lvgl/src/lv_objx/lv_spinbox.h
    lvgl/src/lv_objx/lv_sw.h
    lvgl/src/lv_objx/lv_ta.h
    lvgl/src/lv_objx/lv_table.h
    lvgl/src/lv_objx/lv_tabview.h
    lvgl/src/lv_objx/lv_tileview.h
    lvgl/src/lv_objx/lv_win.h
    lvgl/src/lv_themes/lv_theme.h
    lvgl/src/lv_themes/lv_theme_alien.h
    lvgl/src/lv_themes/lv_theme_default.h
    lvgl/src/lv_themes/lv_theme_material.h
    lvgl/src/lv_themes/lv_theme_mono.h
    lvgl/src/lv_themes/lv_theme_nemo.h
    lvgl/src/lv_themes/lv_theme_night.h
    lvgl/src/lv_themes/lv_theme_templ.h
    lvgl/src/lv_themes/lv_theme_zen.h
    lvgl/src/lv_version.h
    lv_conf.h
    lv_drv_conf.h
)

SET(HDRS
    ${LVHDRS}
    ${N2KHDRS}
    ${DRVHDRS}
    lv_edisplay/hal.h
)

SET(N2KSRCS
    N2K/NMEA2000.cpp
    N2K/nmea2000_rxtx.cpp
    N2K/nmea2000_data_rx.cpp
)

SET(LVSRCS
    lv_edisplay/edisplay.c
    lv_edisplay/font_lib16.c
    lv_edisplay/font_lib24.c
    lvgl/src/lv_core/lv_group.c
    lvgl/src/lv_core/lv_indev.c
    lvgl/src/lv_core/lv_disp.c
    lvgl/src/lv_core/lv_obj.c
    lvgl/src/lv_core/lv_refr.c
    lvgl/src/lv_core/lv_style.c
    lvgl/src/lv_hal/lv_hal_disp.c
    lvgl/src/lv_hal/lv_hal_indev.c
    lvgl/src/lv_hal/lv_hal_tick.c
    lvgl/src/lv_objx/lv_arc.c
    lvgl/src/lv_objx/lv_bar.c
    lvgl/src/lv_objx/lv_cb.c
    lvgl/src/lv_objx/lv_ddlist.c
    lvgl/src/lv_objx/lv_kb.c
    lvgl/src/lv_objx/lv_line.c
    lvgl/src/lv_objx/lv_mbox.c
    lvgl/src/lv_objx/lv_preload.c
    lvgl/src/lv_objx/lv_roller.c
    lvgl/src/lv_objx/lv_table.c
    lvgl/src/lv_objx/lv_tabview.c
    lvgl/src/lv_objx/lv_tileview.c
    lvgl/src/lv_objx/lv_btn.c
    lvgl/src/lv_objx/lv_calendar.c
    lvgl/src/lv_objx/lv_chart.c
    lvgl/src/lv_objx/lv_canvas.c
    lvgl/src/lv_objx/lv_gauge.c
    lvgl/src/lv_objx/lv_label.c
    lvgl/src/lv_objx/lv_list.c
    lvgl/src/lv_objx/lv_slider.c
    lvgl/src/lv_objx/lv_ta.c
    lvgl/src/lv_objx/lv_spinbox.c
    lvgl/src/lv_objx/lv_btnm.c
    lvgl/src/lv_objx/lv_cont.c
    lvgl/src/lv_objx/lv_img.c
    lvgl/src/lv_objx/lv_imgbtn.c
    lvgl/src/lv_objx/lv_led.c
    lvgl/src/lv_objx/lv_lmeter.c
    lvgl/src/lv_objx/lv_page.c
    lvgl/src/lv_objx/lv_sw.c
    lvgl/src/lv_objx/lv_win.c
    lvgl/src/lv_font/lv_font.c
    lvgl/src/lv_font/lv_font_fmt_txt.c
    lvgl/src/lv_font/lv_font_roboto_12.c
    lvgl/src/lv_font/lv_font_roboto_16.c
    lvgl/src/lv_font/lv_font_roboto_22.c
    lvgl/src/lv_font/lv_font_roboto_28.c
    lvgl/src/lv_font/lv_font_unscii_8.c
    lvgl/src/lv_misc/lv_circ.c
    lvgl/src/lv_misc/lv_area.c
    lvgl/src/lv_misc/lv_task.c
    lvgl/src/lv_misc/lv_fs.c
    lvgl/src/lv_misc/lv_anim.c
    lvgl/src/lv_misc/lv_mem.c
    lvgl/src/lv_misc/lv_ll.c
    lvgl/src/lv_misc/lv_color.c
    lvgl/src/lv_misc/lv_txt.c
    lvgl/src/lv_misc/lv_math.c
    lvgl/src/lv_misc/lv_log.c
    lvgl/src/lv_misc/lv_gc.c
    lvgl/src/lv_misc/lv_utils.c
    lvgl/src/lv_misc/lv_async.c
    lvgl/src/lv_themes/lv_theme.c
    lvgl/src/lv_themes/lv_theme_default.c
    lvgl/src/lv_themes/lv_theme_mono.c
    lvgl/src/lv_draw/lv_draw_basic.c
    lvgl/src/lv_draw/lv_draw.c
    lvgl/src/lv_draw/lv_draw_rect.c
    lvgl/src/lv_draw/lv_draw_label.c
    lvgl/src/lv_draw/lv_draw_line.c
    lvgl/src/lv_draw/lv_draw_img.c
    lvgl/src/lv_draw/lv_draw_arc.c
    lvgl/src/lv_draw/lv_draw_triangle.c
    lvgl/src/lv_draw/lv_img_decoder.c
    lvgl/src/lv_draw/lv_img_cache.c
)

SET(SRCS
    main.cpp
    ${LVSRCS}
    ${N2KSRCS}
    ${DRVSRCS}
)

SET( PACKAGE_HEADERS "" )
ADD_EXECUTABLE(${PACKAGE_NAME} ${HDRS} ${SRCS})
TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${SDL2_LIBRARIES} Threads::Threads ${LIBUSB_LIBRARY})
