
#ifndef _MX6_DISPLAY_H_
#define _MX6_DISPLAY_H_

enum {
	MX6_LCD0 = 0,
	MX6_LCD1,
	MX6_HDMI,
	MX6_LVDS0,
	MX6_LVDS1,
	MX6_LVDSD,
	MX6_DSI0,
	MX6_DISPCH_INVALID
};

struct mx6_display_controls {
	void (*hdmi_enable)(int onoff);
	void (*lvds0_enable)(int onoff);
	void (*lvds1_enable)(int onoff);
	void (*lcd0_enable)(int onoff);
	void (*lcd1_enable)(int onoff);
	void (*dsi_enable)(int onoff);
	void (*hdmi_pads)(void);
	void (*lvds0_pads)(void);
	void (*lvds1_pads)(void);
	void (*lcd0_ipu1_pads)(void);
	void (*lcd0_ipu2_pads)(void);
	void (*lcd1_ipu1_pads)(void);
	void (*lcd1_ipu2_pads)(void);
	void (*dsi_pads)(void);
	void (*hdmi_ddc_pads_enable)(void);
	void (*hdmi_ddc_pads_disable)(void);
	int hdmi_i2c;
	int hdcp_enable;
	int lvds0_i2c;
	int lvds1_i2c;
	int lcd0_i2c;
	int lcd1_i2c;
	int dsi_i2c;
};

extern struct mx6_display_controls *mx6_disp_ctrls;

void mx6_init_display(void);
void mx6_display_ch_capability_setup(int lcd0, int lcd1, int hdmi, int lvds0,
						int lvds1, int lvdsd, int dsi0);

#endif
