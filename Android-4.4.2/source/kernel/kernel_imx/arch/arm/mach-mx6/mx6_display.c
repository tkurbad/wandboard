
/*
    MX6 DISPLAY CONTROL FRAMEWORK.
    Copyright (C) 2012,2013,2014 TechNexion Ltd.
    Edward.lin <edward.lin@technexion.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include <mach/hardware.h>
#include <mach/iomux-mx6dl.h>
#include <mach/iomux-mx6q.h>
#include <linux/edm.h>
#include <linux/edm_display.h>
#include "devices-imx6q.h"
#include "mx6_display.h"

#define MX6_DISPLAY_DEBUG_ON	0

/****************************************************************************
 *
 * IPU / Transmitter Controls Index Table
 *
 ****************************************************************************/

#define	SIN0	LDB_SIN0
#define	SIN1	LDB_SIN1
#define	DUL0	LDB_DUL_DI0
#define	DUL1	LDB_DUL_DI1
#define	SEP0	LDB_SEP0
#define	SEP1	LDB_SEP1
#define	SPL0	LDB_SPL_DI0
#define	SPL1	LDB_SPL_DI0

struct mx6_ldb_ipu_settings {
	int ipu_id;
	int disp_id;
	int mode;
	int sec_ipu_id;
	int sec_disp_id;
};

struct mx6_ipu_settings {
	int ipu_id;
	int disp_id;
};

struct mx6_display_array {
	int avaiable;
	int disp_ch[MX6_DISPCH_INVALID];
	char *desc;
	struct mx6_ipu_settings s_lcd0;
	struct mx6_ipu_settings s_lcd1;
	struct mx6_ipu_settings s_hdmi;
	struct mx6_ldb_ipu_settings s_ldb;
	struct mx6_ipu_settings s_dsi0;
};

struct mx6_display_array mx6_1_disp_ref_setting[] = {
/* 1 display */
/*READY, { LCD0,  LCD1, HDMI0, LVDS0, LVDS1, LVDSD,  DSI0}  --Comment of display Combinations--  LCD0 ,  LCD1 , HDMI0 , LVDS 1 + MODE + 2 ,  DSI0 */
{     0, {    1,     0,     0,     0,     0,     0,     0},                         "  lcd0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     0,     0,     0},                         "  lcd1 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     0,     0,     0},                         " hdmi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     0,     0,     1,     0,     0,     0},                         " lvds0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     0,     0,     0,     1,     0,     0},                         " lvds1 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN1, 0, 1}, {0, 0} },
{     0, {    0,     0,     0,     0,     0,     1,     0},                         " lvdsd ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    0,     0,     0,     0,     0,     0,     1},                         "  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
/* total 7 displays for 1 display */
{    -1, {    1,     1,     1,     1,     1,     1,     1},               "  Stop condition ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
};


struct mx6_display_array mx6_2_disp_1ipu_ref_setting[] = {
/* 2 displays */
/*READY, { LCD0,  LCD1, HDMI0, LVDS0, LVDS1, LVDSD,  DSI0}  --Comment of display Combinations--  LCD0 ,  LCD1 , HDMI0 , LVDS 1 + MODE + 2 ,  DSI0 */
{     0, {    1,     1,     0,     0,     0,     0,     0},                 "  lcd0 +  lcd1 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     0,     1,     0,     0,     0,     0},                 "  lcd0 + hdmi0 ", {0, 0}, {0, 0}, {0, 1}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     1,     0,     0,     0},                 "  lcd0 + lvds0 ", {0, 0}, {0, 0}, {0, 0}, {0, 1, DUL1, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     0,     1,     0,     0},                 "  lcd0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {0, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     0,     0,     1,     0},                 "  lcd0 + lvdsd ", {0, 0}, {0, 0}, {0, 0}, {0, 1, SPL1, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     0,     0,     0,     1},                 "  lcd0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 1} },
{     0, {    0,     1,     1,     0,     0,     0,     0},                 "  lcd1 + hdmi0 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     1,     0,     0,     0},                 "  lcd1 + lvds0 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     1,     0,     0},                 "  lcd1 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, DUL0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     0,     1,     0},                 "  lcd1 + lvdsd ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     0,     0,     1},                 "  lcd1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     1,     0,     0,     0},                 " hdmi0 + lvds0 ", {0, 0}, {0, 0}, {0, 1}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     1,     0,     0},                 " hdmi0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {0, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     0,     1,     0},                 " hdmi0 + lvdsd ", {0, 0}, {0, 0}, {0, 0}, {0, 1, SPL1, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     0,     0,     1},                 " hdmi0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 1} },
{     0, {    0,     0,     0,     1,     1,     0,     0},                 " lvds0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SEP0, 0, 1}, {0, 0} },
{     0, {    0,     0,     0,     1,     0,     0,     1},                 " lvds0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 1} },
{     0, {    0,     0,     0,     0,     1,     0,     1},                 " lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    0,     0,     0,     0,     0,     1,     1},                 " lvdsd +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SPL0, 0, 0}, {0, 1} },
/* total 19 displays for 2 display */
{    -1, {    1,     1,     1,     1,     1,     1,     1},               "  Stop condition ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
};

struct mx6_display_array mx6_2_disp_2ipu_ref_setting[] = {
/* 2 displays */
/*READY, { LCD0,  LCD1, HDMI0, LVDS0, LVDS1, LVDSD,  DSI0}  --Comment of display Combinations--  LCD0 ,  LCD1 , HDMI0 , LVDS 1 + MODE + 2 ,  DSI0 */
{     0, {    1,     1,     0,     0,     0,     0,     0},                 "  lcd0 +  lcd1 ", {0, 0}, {1, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     0,     1,     0,     0,     0,     0},                 "  lcd0 + hdmi0 ", {0, 0}, {0, 0}, {1, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     1,     0,     0,     0},                 "  lcd0 + lvds0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     0,     1,     0,     0},                 "  lcd0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {1, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     0,     0,     1,     0},                 "  lcd0 + lvdsd ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    1,     0,     0,     0,     0,     0,     1},                 "  lcd0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {1, 0} },
{     0, {    0,     1,     1,     0,     0,     0,     0},                 "  lcd1 + hdmi0 ", {0, 0}, {0, 1}, {1, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     1,     0,     0,     0},                 "  lcd1 + lvds0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     1,     0,     0},                 "  lcd1 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {1, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     0,     1,     0},                 "  lcd1 + lvdsd ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    0,     1,     0,     0,     0,     0,     1},                 "  lcd1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {1, 0} },
{     0, {    0,     0,     1,     1,     0,     0,     0},                 " hdmi0 + lvds0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     1,     0,     0},                 " hdmi0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {1, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     0,     1,     0},                 " hdmi0 + lvdsd ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     0,     0,     0,     1},                 " hdmi0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {1, 0} },
{     0, {    0,     0,     0,     1,     1,     0,     0},                 " lvds0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SEP0, 0, 1}, {0, 0} },
{     0, {    0,     0,     0,     1,     0,     0,     1},                 " lvds0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {1, 0} },
{     0, {    0,     0,     0,     0,     1,     0,     1},                 " lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 1, SIN1, 0, 0}, {1, 0} },
{     0, {    0,     0,     0,     0,     0,     1,     1},                 " lvdsd +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SPL0, 0, 0}, {1, 0} },
/* total 19 displays for 2 display */
{    -1, {    1,     1,     1,     1,     1,     1,     1},               "  Stop condition ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
};


struct mx6_display_array mx6_3_disp_ref_setting[] = {
/* 3 displays */
/*READY, { LCD0,  LCD1, HDMI0, LVDS0, LVDS1, LVDSD,  DSI0}  --Comment of display Combinations--  LCD0 ,  LCD1 , HDMI0 , LVDS 1 + MODE + 2 ,  DSI0 */
{     0, {    1,     1,     1,     0,     0,     0,     0},         "  lcd0 +  lcd1 + hdmi0 ", {0, 0}, {0, 1}, {1, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     1,     0,     1,     0,     0,     0},         "  lcd0 +  lcd1 + lvds0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     1,     0,     0,     1,     0,     0},         "  lcd0 +  lcd1 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {1, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    1,     1,     0,     0,     0,     1,     0},         "  lcd0 +  lcd1 + lvdsd ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    1,     1,     0,     0,     0,     0,     1},         "  lcd0 +  lcd1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {1, 0} },
{     0, {    1,     0,     1,     1,     0,     0,     0},         "  lcd0 + hdmi0 + lvds0 ", {0, 0}, {0, 0}, {1, 0}, {0, 1, DUL1, 0, 0}, {0, 0} },
{     0, {    1,     0,     1,     0,     1,     0,     0},         "  lcd0 + hdmi0 + lvds1 ", {0, 0}, {0, 0}, {1, 0}, {0, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    1,     0,     1,     0,     0,     1,     0},         "  lcd0 + hdmi0 + lvdsd ", {0, 0}, {0, 0}, {1, 0}, {0, 1, SPL1, 0, 0}, {0, 0} },
{     0, {    1,     0,     1,     0,     0,     0,     1},         "  lcd0 + hdmi0 +  dsi0 ", {0, 0}, {0, 0}, {1, 0}, {0, 0, SIN0, 0, 0}, {0, 1} },
{     0, {    1,     0,     0,     1,     1,     0,     0},         "  lcd0 + lvds0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 0} },
{     0, {    1,     0,     0,     1,     0,     0,     1},         "  lcd0 + lvds0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 1} },
{     0, {    1,     0,     0,     0,     1,     0,     1},         "  lcd0 + lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 1, SIN1, 0, 0}, {0, 1} },
{     0, {    1,     0,     0,     0,     0,     1,     1},         "  lcd0 + lvdsd +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 1} },
{     0, {    0,     1,     1,     1,     0,     0,     0},         "  lcd1 + hdmi0 + lvds0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    0,     1,     1,     0,     1,     0,     0},         "  lcd1 + hdmi0 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {1, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    0,     1,     1,     0,     0,     1,     0},         "  lcd1 + hdmi0 + lvdsd ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    0,     1,     1,     0,     0,     0,     1},         "  lcd1 + hdmi0 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {0, 0, SIN0, 0, 0}, {1, 0} },
{     0, {    0,     1,     0,     1,     1,     0,     0},         "  lcd1 + lvds0 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 0} },
{     0, {    0,     1,     0,     1,     0,     0,     1},         "  lcd1 + lvds0 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SIN0, 0, 0}, {1, 1} },
{     0, {    0,     1,     0,     0,     1,     0,     1},         "  lcd1 + lvds1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 1, SIN1, 0, 0}, {1, 0} },
{     0, {    0,     1,     0,     0,     0,     1,     1},         "  lcd1 + lvdsd +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 0} },
{     0, {    0,     0,     1,     1,     1,     0,     0},         " hdmi0 + lvds0 + lvds1 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 0} },
{     0, {    0,     0,     1,     1,     0,     0,     1},         " hdmi0 + lvds0 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SIN0, 0, 0}, {1, 1} },
{     0, {    0,     0,     1,     0,     1,     0,     1},         " hdmi0 + lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 1, SIN1, 0, 0}, {1, 0} },
{     0, {    0,     0,     1,     0,     0,     1,     1},         " hdmi0 + lvdsd +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SPL0, 0, 0}, {0, 1} },
{     0, {    0,     0,     0,     1,     1,     0,     1},         " lvds0 + lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SEP0, 0, 1}, {1, 0} },
/* total 26 displays for 3 display */
{    -1, {    1,     1,     1,     1,     1,     1,     1},               "  Stop condition ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
};

struct mx6_display_array mx6_4_disp_ref_setting[] = {
/* 4 displays */
/*READY, { LCD0,  LCD1, HDMI0, LVDS0, LVDS1, LVDSD,  DSI0}  --Comment of display Combinations--  LCD0 ,  LCD1 , HDMI0 , LVDS 1 + MODE + 2 ,  DSI0 */
{     0, {    1,     1,     1,     1,     0,     0,     0},  "  lcd0 +  lcd1 + hdmi0 + lvds0 ", {0, 0}, {0, 1}, {1, 1}, {1, 0, SIN0, 0, 0}, {0, 0} },
{     0, {    1,     1,     1,     0,     1,     0,     0},  "  lcd0 +  lcd1 + hdmi0 + lvds1 ", {0, 0}, {0, 1}, {1, 0}, {1, 1, SIN1, 0, 0}, {0, 0} },
{     0, {    1,     1,     1,     0,     0,     1,     0},  "  lcd0 +  lcd1 + hdmi0 + lvdsd ", {0, 0}, {0, 1}, {1, 0}, {1, 1, SPL1, 0, 0}, {0, 0} },
{     0, {    1,     1,     1,     0,     0,     0,     1},  "  lcd0 +  lcd1 + hdmi0 +  dsi0 ", {0, 0}, {0, 1}, {1, 0}, {0, 0, SIN0, 0, 0}, {1, 1} },
{     0, {    1,     1,     0,     1,     1,     0,     0},  "  lcd0 +  lcd1 + lvds0 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 0} },
{     0, {    1,     1,     0,     1,     0,     0,     1},  "  lcd0 +  lcd1 + lvds0 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SIN0, 0, 0}, {1, 1} },
{     0, {    1,     1,     0,     0,     1,     0,     1},  "  lcd0 +  lcd1 + lvds1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 1, SIN1, 0, 0}, {1, 0} },
{     0, {    1,     1,     0,     0,     0,     1,     1},  "  lcd0 +  lcd1 + lvdsd +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 0, 0}, {1, 1} },
{     0, {    1,     0,     1,     1,     1,     0,     0},  "  lcd0 + hdmi0 + lvds0 + lvds1 ", {0, 0}, {0, 0}, {0, 1}, {1, 0, SEP0, 1, 1}, {0, 0} },
{     0, {    1,     0,     1,     1,     0,     0,     1},  "  lcd0 + hdmi0 + lvds0 +  dsi0 ", {0, 0}, {0, 0}, {0, 1}, {1, 0, SIN0, 0, 0}, {1, 1} },
{     0, {    1,     0,     1,     0,     1,     0,     1},  "  lcd0 + hdmi0 + lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 1}, {1, 1, SIN1, 0, 0}, {1, 0} },
{     0, {    1,     0,     1,     0,     0,     1,     1},  "  lcd0 + hdmi0 + lvdsd +  dsi0 ", {0, 0}, {0, 0}, {0, 1}, {1, 0, SPL0, 0, 0}, {1, 1} },
{     0, {    1,     0,     0,     1,     1,     0,     1},  "  lcd0 + lvds0 + lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 1} },
{     0, {    0,     1,     1,     1,     1,     0,     0},  "  lcd1 + hdmi0 + lvds0 + lvds1 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 1, 1}, {0, 0} },
{     0, {    0,     1,     1,     1,     0,     0,     1},  "  lcd1 + hdmi0 + lvds0 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SIN0, 0, 0}, {0, 1} },
{     0, {    0,     1,     1,     0,     1,     0,     1},  "  lcd1 + hdmi0 + lvds1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 1, SIN1, 0, 0}, {1, 0} },
{     0, {    0,     1,     1,     0,     0,     1,     1},  "  lcd1 + hdmi0 + lvdsd +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SPL0, 0, 0}, {1, 1} },
{     0, {    0,     1,     0,     1,     1,     0,     1},  "  lcd1 + lvds0 + lvds1 +  dsi0 ", {0, 0}, {0, 1}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 0} },
{     0, {    0,     0,     1,     1,     1,     0,     1},  " hdmi0 + lvds0 + lvds1 +  dsi0 ", {0, 0}, {0, 0}, {0, 0}, {1, 0, SEP0, 1, 1}, {0, 1} },
/* total 19 displays for 4 display */
{    -1, {    1,     1,     1,     1,     1,     1,     1},                "  Stop condition ", {0, 0}, {0, 0}, {0, 0}, {0, 0, SIN0, 0, 0}, {0, 0} },
};

#if MX6_DISPLAY_DEBUG_ON
static void mx6_display_ch_avaiability_list(struct mx6_display_array *ary_lst, char *tags)
{
	int count, i;
	i = 0;
	count = 0;
	if (tags == NULL)
		tags = "";

	printk("Avaiable Combination for %s :\n", tags);
	while (ary_lst[i].avaiable >= 0) {
		if (ary_lst[i].avaiable == 1) {
			printk("%34s\n", ary_lst[i].desc);
			count++;
		}
		i++;
	}
	printk("Total %d combination for %s.\n\n", count, tags);
}
#endif

static void mx6_display_ch_avaiability_enable_list(struct mx6_display_array *ary_lst, int *ch_status)
{
	int i, j;
	i = 0;
	while (ary_lst[i].avaiable >= 0) {
		int invalid = 0;
		int *ch = (int *)ary_lst[i].disp_ch;
		for (j = 0; j < MX6_DISPCH_INVALID; j++) {
			if ((ch_status[j] == 0) && (ch[j] == 1))
				invalid++;
		}
		if (invalid == 0)
			ary_lst[i].avaiable = 1;
		i++;
	}
}

static struct mx6_display_array *mx6_display_find_match_ref_settings(struct mx6_display_array *ary_lst, int *ch_status)
{
	int i = 0;
	while (ary_lst[i].avaiable >= 0) {
		if (ary_lst[i].avaiable == 1) {
			int j;
			int diff = 0;
			for (j = 0; j < MX6_DISPCH_INVALID; j++) {
				if (ch_status[j] != ary_lst[i].disp_ch[j])
					diff++;
			}
			if (diff == 0) {
				return &ary_lst[i];
			}
		}
		i++;
	}
	return NULL;
}


/****************************************************************************
 *
 * IPU
 *
 ****************************************************************************/

#define MX6_MAX_DISPLAYS	4 /* For imx6dl/solo, it should be 2 */
static unsigned int mx6_max_displays = 2;
static unsigned int mx6_nof_edm_displays = 0;

static struct ipuv3_fb_platform_data mx6_ipuv3_fb_pdata[MX6_MAX_DISPLAYS];

#if 0
static struct imx_ipuv3_platform_data mx6_ipu_data[] = {
	{
		.rev		= 4,
		.csi_clk[0]	= "ccm_clk0",
	}, {
		.rev		= 4,
		.csi_clk[0]	= "ccm_clk0",
	},
};

/* ------------------------------------------------------------------------ */

static __init void mx6_init_ipu(void)
{
	imx6q_add_ipuv3(0, &mx6_ipu_data[0]);
	if (cpu_is_mx6q()) {
		imx6q_add_ipuv3(1, &mx6_ipu_data[1]);
		mx6_max_displays = 4;
	}
}
#endif

/****************************************************************************
 *
 * HDMI Transmitter
 *
 ****************************************************************************/

/* ------------------------------------------------------------------------ */

static void mx6_hdmi_init(int ipu_id, int disp_id)
{
	if ((unsigned)ipu_id > 1)
		ipu_id = 0;
	if ((unsigned)disp_id > 1)
		disp_id = 0;

	mxc_iomux_set_gpr_register(3, 2, 2, 2*ipu_id + disp_id);
}

/* ------------------------------------------------------------------------ */

static struct fsl_mxc_hdmi_platform_data mx6_hdmi_data = {
	.init = mx6_hdmi_init,
};

/* ------------------------------------------------------------------------ */

static struct fsl_mxc_hdmi_core_platform_data mx6_hdmi_core_data = {
	.ipu_id		= 0,
	.disp_id	= 1,
	.hdmi_timing 	= NULL,
};

/* ------------------------------------------------------------------------ */

static const struct i2c_board_info mx6_hdmi_i2c_info = {
	I2C_BOARD_INFO("mxc_hdmi_i2c", 0x50),
};

/* ------------------------------------------------------------------------ */

static void __init mx6_init_transmitter_hdmi(void)
{
	int mx6_hdmi_ddc = 1;
	int mx6_hdcp_enable = 0;
	void (*ddc_enable)(void);
	void (*ddc_disable)(void);
	ddc_enable = NULL;
	ddc_disable = NULL;

	if (mx6_disp_ctrls != NULL) {
		if (mx6_disp_ctrls->hdmi_pads != NULL)
			mx6_disp_ctrls->hdmi_pads();
		if (mx6_disp_ctrls->hdmi_i2c > 0)
			mx6_hdmi_ddc = mx6_disp_ctrls->hdmi_i2c;
		if (mx6_disp_ctrls->hdmi_ddc_pads_enable != NULL)
			ddc_enable = mx6_disp_ctrls->hdmi_ddc_pads_enable;
		if (mx6_disp_ctrls->hdmi_ddc_pads_disable != NULL)
			ddc_disable = mx6_disp_ctrls->hdmi_ddc_pads_disable;
	}

	i2c_register_board_info(mx6_hdmi_ddc, &mx6_hdmi_i2c_info, 1);

	if ((mx6_hdcp_enable == 1) &&
		(ddc_enable != NULL) &&
		(ddc_disable != NULL)) {
		mx6_hdmi_data.enable_pins = ddc_enable;
		mx6_hdmi_data.disable_pins = ddc_disable;
		mx6_hdmi_data.phy_reg_vlev = 0x0294;
		mx6_hdmi_data.phy_reg_cksymtx = 0x800d;
	}
	imx6q_add_mxc_hdmi_core(&mx6_hdmi_core_data);
	imx6q_add_mxc_hdmi(&mx6_hdmi_data);

	/* Enable HDMI audio */
	imx6q_add_hdmi_soc();
	imx6q_add_hdmi_soc_dai();
	mxc_iomux_set_gpr_register(0, 0, 1, 1);
}

/****************************************************************************
 *
 * LCD / DPI / TTL / RAW
 *
 ****************************************************************************/

static struct fsl_mxc_lcd_platform_data mx6_lcdif_data = {
	.ipu_id = 0,
	.disp_id = 0,
	.default_ifmt = IPU_PIX_FMT_RGB666,
	.lcd0_timing = NULL,
	.lcd1_timing = NULL,
};

/* ------------------------------------------------------------------------ */

static void __init mx6_init_transmitter_raw(void)
{
	/* TTL */
	if (mx6_disp_ctrls != NULL) {
		if (mx6_lcdif_data.ipu_id == 0) {
			if (mx6_disp_ctrls->lcd0_ipu1_pads != NULL)
				mx6_disp_ctrls->lcd0_ipu1_pads();
		} else {
			if (mx6_disp_ctrls->lcd0_ipu2_pads != NULL)
				mx6_disp_ctrls->lcd0_ipu2_pads();
		}
	}
	imx6q_add_lcdif(&mx6_lcdif_data);
}

/* ------------------------------------------------------------------------ */

/****************************************************************************
 *
 * LVDS Transmitter
 *
 ****************************************************************************/

static struct fsl_mxc_ldb_platform_data mx6_ldb_data = {
	.ipu_id = 0,
	.disp_id = 0,
	.ext_ref = 1,
	.mode = LDB_SIN0,
	.sec_ipu_id = 0,
	.sec_disp_id = 1,
	.lvds0_timing = NULL,
	.lvds1_timing = NULL,
};

static void __init mx6_init_transmitter_lvds(void)
{
	/* LVDS */
	if (mx6_disp_ctrls != NULL) {
		if (mx6_disp_ctrls->lvds0_pads != NULL)
				mx6_disp_ctrls->lvds0_pads();
		if (mx6_disp_ctrls->lvds1_pads != NULL)
				mx6_disp_ctrls->lvds1_pads();
	}
	imx6q_add_ldb(&mx6_ldb_data);
}


/****************************************************************************
 *
 * DSI Transmitter
 *
 ****************************************************************************/

static void mx6_reset_mipi_dsi(void)
{
#if 0
	gpio_set_value(MX6_DISP_PWR_EN, 1);
	gpio_set_value(MX6_DISP_RST_B, 1);
	udelay(10);
	gpio_set_value(MX6_DISP_RST_B, 0);
	udelay(50);
	gpio_set_value(MX6_DISP_RST_B, 1);

	/*
	 * it needs to delay 120ms minimum for reset complete
	 */
	msleep(120);
#endif
}


static struct mipi_dsi_platform_data mx6_dsi_pdata = {
	.ipu_id		= 0,
	.disp_id	= 1,
	.lcd_panel	= "TRULY-WVGA",
	.reset		= mx6_reset_mipi_dsi,
};

static void __init mx6_init_transmitter_dsi(void)
{
	if (mx6_disp_ctrls != NULL) {
		if (mx6_disp_ctrls->dsi_pads != NULL)
				mx6_disp_ctrls->dsi_pads();
	}
	imx6q_add_mipi_dsi(&mx6_dsi_pdata);
}

/* ------------------------------------------------------------------------ */

/****************************************************************************
 *
 * DISPLAY CAPABILITY SETTING
 *
 ****************************************************************************/

/*LCD0, LCD1, HDMI0, LVDS0, LVDS1, LVDSD, DSI0*/
static int mx6_disp_ch_status[MX6_DISPCH_INVALID] = {1, 0, 1, 1, 0, 0, 1};

void mx6_display_ch_capability_setup(int lcd0, int lcd1, int hdmi, int lvds0,
						int lvds1, int lvdsd, int dsi0)
{
	int total_ch = 0;
	static int setup_done = 0;
	int *ch_status = mx6_disp_ch_status;

	if (setup_done == 0) {
		setup_done = 1;
	} else {
		return;
	}

	ch_status[MX6_LCD0] = lcd0;
	ch_status[MX6_LCD1] = lcd1;
	ch_status[MX6_HDMI] = hdmi;
	ch_status[MX6_LVDS0] = lvds0;
	ch_status[MX6_LVDS1] = lvds1;
	ch_status[MX6_LVDSD] = lvdsd;
	ch_status[MX6_DSI0] = dsi0;
	total_ch = ch_status[MX6_LCD0] + ch_status[MX6_LCD1]
		+ ch_status[MX6_HDMI] + ch_status[MX6_LVDS0]
		+ ch_status[MX6_LVDS1] + ch_status[MX6_DSI0];

	if ((total_ch <= 0) || (total_ch >= MX6_DISPCH_INVALID))
		return;

	if (total_ch > 0)
		mx6_display_ch_avaiability_enable_list(mx6_1_disp_ref_setting, ch_status);

	if (total_ch > 1) {
		mx6_display_ch_avaiability_enable_list(mx6_2_disp_1ipu_ref_setting, ch_status);
		mx6_display_ch_avaiability_enable_list(mx6_2_disp_2ipu_ref_setting, ch_status);
	}

	if (total_ch > 2)
		mx6_display_ch_avaiability_enable_list(mx6_3_disp_ref_setting, ch_status);

	if (total_ch > 3)
		mx6_display_ch_avaiability_enable_list(mx6_4_disp_ref_setting, ch_status);

#if MX6_DISPLAY_DEBUG_ON
	mx6_display_ch_avaiability_list(mx6_1_disp_ref_setting, "mx6_1_disp_ref_setting");
	if (cpu_is_mx6dl()) {
		mx6_display_ch_avaiability_list(mx6_2_disp_1ipu_ref_setting, "mx6_2_disp_1ipu_ref_setting");
	} else if (cpu_is_mx6q()) {
		mx6_display_ch_avaiability_list(mx6_2_disp_2ipu_ref_setting, "mx6_2_disp_2ipu_ref_setting");
		mx6_display_ch_avaiability_list(mx6_3_disp_ref_setting, "mx6_3_disp_ref_setting");
		mx6_display_ch_avaiability_list(mx6_4_disp_ref_setting, "mx6_4_disp_ref_setting");
	}
#endif

}
EXPORT_SYMBOL(mx6_display_ch_capability_setup);

/****************************************************************************
 *
 * Transmitter Settings
 *
 ****************************************************************************/

static struct fsl_video_timing *timing_str_to_fsl_timing(char *timing_str)
{
#if defined(CONFIG_EDM)
	struct fsl_video_timing *fsl_timing;
	struct edm_video_timing edm_timing;
	if ((timing_str == NULL) || (strlen(timing_str) == 0))
		return NULL;

	fsl_timing = kmalloc(sizeof(struct fsl_video_timing), GFP_KERNEL);
	edm_disp_str_to_timing(&edm_timing, timing_str);
	fsl_timing->pixclock 	= edm_timing.pixclock;
	fsl_timing->hres 	= edm_timing.hres;
	fsl_timing->hfp 	= edm_timing.hfp;
	fsl_timing->hbp 	= edm_timing.hbp;
	fsl_timing->hsw 	= edm_timing.hsw;
	fsl_timing->vres 	= edm_timing.vres;
	fsl_timing->vfp 	= edm_timing.vfp;
	fsl_timing->vbp 	= edm_timing.vbp;
	fsl_timing->vsw 	= edm_timing.vsw;
	if ((fsl_timing->pixclock == 0) ||
		(fsl_timing->hres == 0) ||
		(fsl_timing->vres == 0))
		return NULL;

	return fsl_timing;
#else
	return NULL;
#endif
}

struct mx6_display_controls *mx6_disp_ctrls = NULL;

static void __init mx6_init_display_transmitter(struct ipuv3_fb_platform_data *ipufbs)
{
	int i;
	int disp_ch[MX6_DISPCH_INVALID] = {0};
	struct mx6_display_array *ref_setting = NULL;
	char *string = NULL;
	int first_lvds = -1;

	if (mx6_nof_edm_displays == 0)
		goto enable_transmitter;

	for (i = 0; i < mx6_nof_edm_displays; i++) {
		struct ipuv3_fb_platform_data *ipufb;
		char *str = NULL;
		ipufb = &ipufbs[i];
		str = ipufb->timing_str;
		switch (ipufb->disp_ch) {
		case MX6_HDMI:
			if (mx6_disp_ch_status[MX6_HDMI] == 1) {
				disp_ch[MX6_HDMI] = 1;
				mx6_hdmi_core_data.ipu_id = 0;
				mx6_hdmi_core_data.disp_id = 0;
				mx6_hdmi_core_data.hdmi_timing =
					timing_str_to_fsl_timing(str);
			}
			break;
		case MX6_LVDS0:
			if (mx6_disp_ch_status[MX6_LVDS0] == 1) {
				disp_ch[MX6_LVDS0] = 1;
				mx6_ldb_data.ipu_id = 0;
				mx6_ldb_data.disp_id = 0;
				mx6_ldb_data.mode = LDB_SIN0;
				mx6_ldb_data.lvds0_timing =
					timing_str_to_fsl_timing(str);
				if (first_lvds < 0)
					first_lvds = 0;
			}
			break;
		case MX6_LVDS1:
			if (mx6_disp_ch_status[MX6_LVDS1] == 1) {
				disp_ch[MX6_LVDS1] = 1;
				mx6_ldb_data.ipu_id = 0;
				mx6_ldb_data.disp_id = 1;
				mx6_ldb_data.mode = LDB_SIN1;
				mx6_ldb_data.lvds1_timing =
					timing_str_to_fsl_timing(str);
				if (first_lvds < 0)
					first_lvds = 1;
			}
			break;
		case MX6_LVDSD:
			if (mx6_disp_ch_status[MX6_LVDSD] == 1) {
				disp_ch[MX6_LVDSD] = 1;
				mx6_ldb_data.ipu_id = 0;
				mx6_ldb_data.disp_id = 0;
				mx6_ldb_data.mode = LDB_SPL_DI0;
				mx6_ldb_data.lvds0_timing =
					timing_str_to_fsl_timing(str);
			}
			break;
		case MX6_LCD0:
			if (mx6_disp_ch_status[MX6_LCD0] == 1) {
				disp_ch[MX6_LCD0] = 1;
				mx6_lcdif_data.ipu_id = 0;
				mx6_lcdif_data.disp_id = 0;
				mx6_lcdif_data.lcd0_timing =
					timing_str_to_fsl_timing(str);
			}
			break;
		case MX6_DSI0:
			if (mx6_disp_ch_status[MX6_DSI0] == 1) {
				disp_ch[MX6_DSI0] = 1;
				mx6_dsi_pdata.ipu_id = 0;
				mx6_dsi_pdata.disp_id = 0;
			}
			break;
		default:
			break;
		}
	}

	if (mx6_nof_edm_displays == 1) {
		ref_setting = mx6_display_find_match_ref_settings(mx6_1_disp_ref_setting, disp_ch);
		string = "mx6_1_disp_ref_setting, disp_ch";
	} else if ((mx6_nof_edm_displays == 2) && (cpu_is_mx6q())) {
		/*2 displays with mx6qd*/
		ref_setting = mx6_display_find_match_ref_settings(mx6_2_disp_2ipu_ref_setting, disp_ch);
		string = "mx6_2_disp_2ipu_ref_setting, disp_ch";
	} else if (mx6_nof_edm_displays == 2) {
		/*2 displays with mx6dl/solo*/
		ref_setting = mx6_display_find_match_ref_settings(mx6_2_disp_1ipu_ref_setting, disp_ch);
		string = "mx6_2_disp_1ipu_ref_setting, disp_ch";
	} else if (mx6_nof_edm_displays == 3) {
		ref_setting = mx6_display_find_match_ref_settings(mx6_3_disp_ref_setting, disp_ch);
		string = "mx6_3_disp_ref_setting, disp_ch";
	} else if (mx6_nof_edm_displays == 4) {
		ref_setting = mx6_display_find_match_ref_settings(mx6_4_disp_ref_setting, disp_ch);
		string = "mx6_4_disp_ref_setting, disp_ch";
	}

	if (ref_setting != NULL) {
	#if MX6_DISPLAY_DEBUG_ON
		printk("\n%s : %s from %s\n\n", __FUNCTION__, ref_setting->desc, string);
	#endif
		mx6_hdmi_core_data.ipu_id 	= ref_setting->s_hdmi.ipu_id;
		mx6_hdmi_core_data.disp_id 	= ref_setting->s_hdmi.disp_id;
		mx6_ldb_data.ipu_id 		= ref_setting->s_ldb.ipu_id;
		mx6_ldb_data.disp_id 		= ref_setting->s_ldb.disp_id;
		mx6_ldb_data.mode 		= ref_setting->s_ldb.mode;
		mx6_ldb_data.sec_ipu_id 	= ref_setting->s_ldb.sec_ipu_id;
		mx6_ldb_data.sec_disp_id 	= ref_setting->s_ldb.sec_disp_id;
		mx6_lcdif_data.ipu_id 		= ref_setting->s_lcd0.ipu_id;
		mx6_lcdif_data.disp_id 		= ref_setting->s_lcd0.disp_id;
		mx6_dsi_pdata.ipu_id 		= ref_setting->s_dsi0.ipu_id;
		mx6_dsi_pdata.disp_id		= ref_setting->s_dsi0.disp_id;
		if ((mx6_ldb_data.mode == SEP0) || (mx6_ldb_data.mode == SEP1)) {
			if ((first_lvds >= 0) && (first_lvds == 0))
				mx6_ldb_data.mode = SEP0;
			else if ((first_lvds >= 0) && (first_lvds == 1))
				mx6_ldb_data.mode = SEP1;
		}

	} else {
		/* This is basically impossible for mx6s, mx6dl, mx6d, and mx6q */
		printk("\n\n%s : Cannot find any match reference display settings!!\n\n", __FUNCTION__);
	}

enable_transmitter:

	imx6q_add_vdoa();
	if (mx6_disp_ch_status[MX6_DSI0] == 1) {
		mx6_init_transmitter_dsi();
		if ((mx6_disp_ctrls != NULL) &&
			(mx6_disp_ctrls->dsi_enable != NULL))
			mx6_disp_ctrls->dsi_enable(1);
	}
	if ((mx6_disp_ch_status[MX6_LCD0] == 1) ||
		 (mx6_disp_ch_status[MX6_LCD1] == 1)) {
		mx6_init_transmitter_raw();
		if (mx6_disp_ctrls != NULL) {
			if (mx6_disp_ctrls->lcd0_enable != NULL)
				mx6_disp_ctrls->lcd0_enable(1);
			if (mx6_disp_ctrls->lcd1_enable != NULL)
				mx6_disp_ctrls->lcd1_enable(1);
		}

	}
	if ((mx6_disp_ch_status[MX6_LVDS0] == 1) ||
		(mx6_disp_ch_status[MX6_LVDS1] == 1) ||
		(mx6_disp_ch_status[MX6_LVDSD] == 1)) {
		mx6_init_transmitter_lvds();
		if (mx6_disp_ctrls != NULL) {
			if (mx6_disp_ctrls->lvds0_enable != NULL)
				mx6_disp_ctrls->lvds0_enable(1);
			if (mx6_disp_ctrls->lvds1_enable != NULL)
				mx6_disp_ctrls->lvds1_enable(1);
		}
	}
	if (mx6_disp_ch_status[MX6_HDMI] == 1) {
		mx6_init_transmitter_hdmi();
		if ((mx6_disp_ctrls != NULL) &&
			(mx6_disp_ctrls->hdmi_enable != NULL))
			mx6_disp_ctrls->hdmi_enable(1);
	}

	printk("mx6_display : list of transmitter settings:\n");
	if (disp_ch[MX6_HDMI] == 1)
		printk("mx6_display : hdmi : ipu%d, di%d\n", mx6_hdmi_core_data.ipu_id, mx6_hdmi_core_data.disp_id);

	if ((disp_ch[MX6_LVDS0] == 1) || (disp_ch[MX6_LVDS1] == 1) || (disp_ch[MX6_LVDSD] == 1)) {
		char string[200];
		int i;
		printk("mx6_display : lvds mode : ");
		switch (mx6_ldb_data.mode) {
		case LDB_SIN0:
			printk("single channel on di0\n");
			break;
		case LDB_SIN1:
			printk("single channel on di1\n");
			break;
		case LDB_SEP0:
			printk("two separate channels, lvds0 on di0\n");
			break;
		case LDB_SEP1:
			printk("two separate channels, lvds1 on di1\n");
			break;
		case LDB_DUL_DI0:
			printk("duplicate di0 to both channels\n");
			break;
		case LDB_DUL_DI1:
			printk("duplicate di1 to both channels\n");
			break;
		case LDB_SPL_DI0:
			printk("split di0 to both channels, odd/even\n");
			break;
		case LDB_SPL_DI1:
			printk("split di1 to both channels, odd/even\n");
			break;
		default:
			printk("unknown settins for lvds\n");
			break;
		}
		if ((disp_ch[MX6_LVDS0] == 1) || (disp_ch[MX6_LVDSD] == 1)) {
			printk("mx6_display : lvds 1st : ipu%d, di%d\n", mx6_ldb_data.ipu_id, mx6_ldb_data.disp_id);
			for (i = 0; i < 200; i++)
				string[i] = '\0';
			if (mx6_ldb_data.lvds0_timing != NULL) {
				sprintf(string, "mx6_display : custom lvds0 timing : %u,%u,%u,%u,%u,%u,%u,%u,%u",
					mx6_ldb_data.lvds0_timing->pixclock,
					mx6_ldb_data.lvds0_timing->hres,
					mx6_ldb_data.lvds0_timing->hfp,
					mx6_ldb_data.lvds0_timing->hbp,
					mx6_ldb_data.lvds0_timing->hsw,
					mx6_ldb_data.lvds0_timing->vres,
					mx6_ldb_data.lvds0_timing->vfp,
					mx6_ldb_data.lvds0_timing->vbp,
					mx6_ldb_data.lvds0_timing->vsw);
				printk("%s\n", string);
			}
		}
		if ((disp_ch[MX6_LVDS1] == 1) || (disp_ch[MX6_LVDSD] == 1)) {
			printk("mx6_display : lvds 2nd : ipu%d, di%d\n", mx6_ldb_data.sec_ipu_id, mx6_ldb_data.sec_disp_id);
			for (i = 0; i < 200; i++)
				string[i] = '\0';
			if (mx6_ldb_data.lvds1_timing != NULL) {
				sprintf(string, "mx6_display : custom lvds1 timing : %u,%u,%u,%u,%u,%u,%u,%u,%u",
					mx6_ldb_data.lvds1_timing->pixclock,
					mx6_ldb_data.lvds1_timing->hres,
					mx6_ldb_data.lvds1_timing->hfp,
					mx6_ldb_data.lvds1_timing->hbp,
					mx6_ldb_data.lvds1_timing->hsw,
					mx6_ldb_data.lvds1_timing->vres,
					mx6_ldb_data.lvds1_timing->vfp,
					mx6_ldb_data.lvds1_timing->vbp,
					mx6_ldb_data.lvds1_timing->vsw);
				printk("%s\n", string);
			}
		}

	}

	if (disp_ch[MX6_LCD0] == 1)
		printk("mx6_display : lcd0 : ipu%d, di%d\n", mx6_lcdif_data.ipu_id, mx6_lcdif_data.disp_id);

	if (disp_ch[MX6_DSI0] == 1)
		printk("mx6_display : dsi0 : ipu%d, di%d\n", mx6_dsi_pdata.ipu_id, mx6_dsi_pdata.disp_id);
}

/****************************************************************************
 *
 * IPUV3_FB / Framebuffer Setting
 *
 ****************************************************************************/

static struct ipuv3_fb_platform_data mx6_preset_fb_data[] = {
	{ /*fb0*/
	.disp_dev = "ldb",
	.interface_pix_fmt = IPU_PIX_FMT_RGB24,
	.mode_str = "LDB-WSVGA",
	.disp_ch = MX6_LVDS0,
	.default_bpp = 32,
	.int_clk = false,
	.late_init = false,
	}, {
	.disp_dev = "hdmi",
	.interface_pix_fmt = IPU_PIX_FMT_RGB32,
	.mode_str = "1920x1080M@60",
	.disp_ch = MX6_HDMI,
	.default_bpp = 32,
	.int_clk = false,
	.late_init = false,
	}, {
	.disp_dev = "lcd",
	.interface_pix_fmt = IPU_PIX_FMT_RGB666,
	.mode_str = "SEIKO-WVGA",
	.disp_ch = MX6_LCD0,
	.default_bpp = 32,
	.int_clk = false,
	.late_init = false,
	},
};


static struct edm_display_device edm_display_devices[MX6_MAX_DISPLAYS];

static void __init mx6_init_ipuv3_fb(void)
{
	int i;

	if (edm_display_devices == NULL)
		goto fallback_preset_ipufb_init;

	for (i = 0; i < mx6_max_displays; i++) {
		struct edm_display_device *dev;
		struct ipuv3_fb_platform_data *ipufb;
		ipufb = &mx6_ipuv3_fb_pdata[i];
		dev = edm_display_devices + i;

		if (dev->disp_dev == EDM_DEV_INVALID)
			break;

		switch (dev->disp_dev) {
		case EDM_HDMI0:
			strncpy(ipufb->disp_dev, "hdmi", 5);
			ipufb->disp_ch = MX6_HDMI;
			break;
		case EDM_HDMI1:
			strncpy(ipufb->disp_dev, "sii902x_hdmi", 13);
			ipufb->disp_ch = MX6_LCD0;
			break;
		case EDM_LVDS0:
			strncpy(ipufb->disp_dev, "ldb", 4);
			ipufb->disp_ch = MX6_LVDS0;
			break;
		case EDM_LVDSD_0_1:
			strncpy(ipufb->disp_dev, "ldb", 4);
			ipufb->disp_ch = MX6_LVDSD;
			break;
		case EDM_LVDS1:
			strncpy(ipufb->disp_dev, "ldb", 4);
			ipufb->disp_ch = MX6_LVDS1;
			break;
		case EDM_LCD0:
			strncpy(ipufb->disp_dev, "lcd", 4);
			ipufb->disp_ch = MX6_LCD0;
			break;
		case EDM_DSI0:
			strncpy(ipufb->disp_dev, "mipi_dsi", 9);
			ipufb->disp_ch = MX6_DSI0;
			break;
		case EDM_DSI1:
		case EDM_LCD1:
		default:
			printk("Unsupport Transmitter type!!\n");
			break;
		}
		if (!strncmp(dev->if_fmt, "RGB24", 5))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_RGB24;
		else if (!strncmp(dev->if_fmt, "BGR24", 5))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_BGR24;
		else if (!strncmp(dev->if_fmt, "GBR24", 5))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_GBR24;
		else if (!strncmp(dev->if_fmt, "RGB565", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_RGB565;
		else if (!strncmp(dev->if_fmt, "RGB666", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_RGB666;
		else if (!strncmp(dev->if_fmt, "YUV444", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_YUV444;
		else if (!strncmp(dev->if_fmt, "LVDS666", 7))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_LVDS666;
		else if (!strncmp(dev->if_fmt, "YUYV16", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_YUYV;
		else if (!strncmp(dev->if_fmt, "UYVY16", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_UYVY;
		else if (!strncmp(dev->if_fmt, "YVYU16", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_YVYU;
		else if (!strncmp(dev->if_fmt, "VYUY16", 6))
			ipufb->interface_pix_fmt = IPU_PIX_FMT_VYUY;

		if (dev->timing.pixclock != 0) {
			char string[200];
			int i;
			for (i = 0; i < 200; i++)
				string[i] = '\0';
			sprintf(string, "%u,%u,%u,%u,%u,%u,%u,%u,%u",
					dev->timing.pixclock,
					dev->timing.hres,
					dev->timing.hfp,
					dev->timing.hbp,
					dev->timing.hsw,
					dev->timing.vres,
					dev->timing.vfp,
					dev->timing.vbp,
					dev->timing.vsw);
			ipufb->timing_str = kstrdup(string, GFP_KERNEL);
			if ((ipufb->disp_ch == MX6_HDMI) ||
				(ipufb->disp_ch == MX6_LVDS0) ||
				(ipufb->disp_ch == MX6_LVDS1) ||
				(ipufb->disp_ch == MX6_LVDSD) ||
				(ipufb->disp_ch == MX6_LCD0)) {
				char *str = NULL;
				switch (ipufb->disp_ch) {
				case MX6_HDMI:
					str = kstrdup("HDMI-USER", GFP_KERNEL);
					break;
				case MX6_LVDS0:
				case MX6_LVDSD:
					str = kstrdup("LDB-USER1", GFP_KERNEL);
					break;
				case MX6_LVDS1:
					str = kstrdup("LDB-USER2", GFP_KERNEL);
					break;
				case MX6_LCD0:
					str = kstrdup("LCD-USER", GFP_KERNEL);
					break;
				default:
					break;
				}
				ipufb->mode_str = str;
			}
		}
		if ((dev->mode_string != NULL) && (ipufb->mode_str == NULL))
			ipufb->mode_str = kstrdup(dev->mode_string, GFP_KERNEL);

		if (dev->bpp != 0)
			ipufb->default_bpp = dev->bpp;

		ipufb->int_clk = false;
		ipufb->late_init = false;
		mx6_nof_edm_displays++;
	}
	if (mx6_nof_edm_displays > 0) {
		mx6_init_display_transmitter(mx6_ipuv3_fb_pdata);
		for (i = 0; i < mx6_nof_edm_displays; i++)
			imx6q_add_ipuv3fb(i, &mx6_ipuv3_fb_pdata[i]);
		return;
	}

	/* fall back to pre-defined */

fallback_preset_ipufb_init:

	if (cpu_is_mx6q()) {
		mx6_hdmi_core_data.ipu_id 	= 0;
		mx6_hdmi_core_data.disp_id 	= 1;
		mx6_ldb_data.ipu_id 		= 1;
		mx6_ldb_data.disp_id 		= 0;
		mx6_ldb_data.mode 		= SIN0;
		mx6_ldb_data.sec_ipu_id 	= 0;
		mx6_ldb_data.sec_disp_id 	= 0;
		mx6_lcdif_data.ipu_id 		= 0;
		mx6_lcdif_data.disp_id 		= 0;
		mx6_dsi_pdata.ipu_id 		= 1;
		mx6_dsi_pdata.disp_id		= 1;

	} else {
		/*
		 * support the following:
		 * lcd + hdmi
		 * lvds + hdmi
		 * dsi + hdmi
		 */
		mx6_hdmi_core_data.ipu_id 	= 0;
		mx6_hdmi_core_data.disp_id 	= 1;
		mx6_ldb_data.ipu_id 		= 0;
		mx6_ldb_data.disp_id 		= 0;
		mx6_ldb_data.mode 		= SIN0;
		mx6_ldb_data.sec_ipu_id 	= 0;
		mx6_ldb_data.sec_disp_id 	= 0;
		mx6_lcdif_data.ipu_id 		= 0;
		mx6_lcdif_data.disp_id 		= 0;
		mx6_dsi_pdata.ipu_id 		= 0;
		mx6_dsi_pdata.disp_id		= 0;
	}

	mx6_init_display_transmitter(mx6_preset_fb_data);
	imx6q_add_ipuv3fb(0, &mx6_preset_fb_data[0]);
	imx6q_add_ipuv3fb(1, &mx6_preset_fb_data[1]);
	if (cpu_is_mx6q())
		imx6q_add_ipuv3fb(2, &mx6_preset_fb_data[2]);
}

void mx6_init_display(void)
{
	int i;
	if (cpu_is_mx6q())
		mx6_max_displays = 4;

	for (i = 0; i < MX6_MAX_DISPLAYS; i++) {
		mx6_ipuv3_fb_pdata[i].disp_dev[0] = '\0';
		mx6_ipuv3_fb_pdata[i].mode_str = NULL;
		mx6_ipuv3_fb_pdata[i].timing_str = NULL;
	}

	/* This will be only setup once */
	mx6_display_ch_capability_setup(1, 0, 1, 1, 0, 0, 1);

#if defined(CONFIG_EDM)
	edm_display_init(saved_command_line, edm_display_devices, mx6_max_displays);
#endif
	mx6_init_ipuv3_fb();

}
EXPORT_SYMBOL(mx6_init_display);



