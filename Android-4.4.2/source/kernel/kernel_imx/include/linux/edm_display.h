/*
    EDM DISPLAY CONTROL FRAMEWORK.
    Copyright (C) 2012,2013 TechNexion Ltd.

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


#ifndef _EDM_DISPLAY_H_
#define _EDM_DISPLAY_H_

enum disp_dev_type {
	EDM_HDMI0 = 0,
	EDM_HDMI1,
	EDM_LVDS0,
	EDM_LVDS1,
	EDM_LVDS2,
	EDM_LVDS3,
	EDM_LVDSD_0_1,
	EDM_LVDSD_2_3,
	EDM_DSI0,
	EDM_DSI1,
	EDM_LCD0,
	EDM_LCD1,
	EDM_DEV_INVALID /*also number of display devices*/
};

struct edm_video_timing {
	u32 pixclock;
	u16 hres;
	u16 hfp;
	u16 hbp;
	u16 hsw;
	u16 vres;
	u16 vfp;
	u16 vbp;
	u16 vsw;
};

struct edm_display_device {
	int display;
	enum disp_dev_type disp_dev;/*hdmi0*/
	char *mode_string;/*"1280x720MR-24@60", custom, LDB-XGA,...*/
	struct edm_video_timing timing; /* "33260,800,210,46,1,480,22,23,1" */
	char *if_fmt;/*"RGB24"*/
	u16 bpp; /* 32, 24, 18, 16, 12 ...*/
};

void edm_disp_str_to_timing(struct edm_video_timing *timing, char *timing_str);
unsigned int edm_display_init(char *cmdline,
				struct edm_display_device *display_devices,
						unsigned int max_displays);
void edm_disp_list_dev(struct edm_display_device *display_devices,
						int num_displays);
#endif

