// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/pwm.h>
#include <video/mipi_display.h>
#include <../../../oneplus/include/linux/oem/boot_mode.h>

#include "dsi_panel.h"
#include "dsi_ctrl_hw.h"
#include "dsi_parser.h"
#include <linux/pm_wakeup.h>
#include <linux/msm_drm_notify.h>
#include <linux/notifier.h>
#include <linux/string.h>
#include <linux/input.h>
#include <linux/proc_fs.h>
#include "dsi_drm.h"
#include "dsi_display.h"
#include "sde_dbg.h"
#include "sde_dsc_helper.h"
#include "sde_vdc_helper.h"
#include "sde_crtc.h"
#include "sde_rm.h"
#include "sde_trace.h"
#if defined(CONFIG_PXLW_IRIS)
#include "iris/dsi_iris5_api.h"
#include "iris/dsi_iris5_gpio.h"
#endif
#include <linux/pm_wakeup.h>
#include "project_info.h"
#include "oplus_display_node.h"
#include "oplus_display_private_api.h"
#include "oplus_onscreenfingerprint.h"
#include "oplus_bl.h"
#ifdef CONFIG_PXLW_IRIS
#include "oplus_adfr.h"
#endif
#include "oplus_msd_aod.h"
//#endif

/**
 * topology is currently defined by a set of following 3 values:
 * 1. num of layer mixers
 * 2. num of compression encoders
 * 3. num of interfaces
 */
#define TOPOLOGY_SET_LEN 3
#define MAX_TOPOLOGY 5

#define DSI_PANEL_SAMSUNG_S6E3HC2 0
#define DSI_PANEL_SAMSUNG_S6E3FC2X01 1
#define DSI_PANEL_SAMSUNG_SOFEF03F_M 2
#define DSI_PANEL_SAMSUNG_ANA6705 3
#define DSI_PANEL_SAMSUNG_ANA6706 4
#define DSI_PANEL_SAMSUNG_AMB655XL 5
#define DSI_PANEL_SAMSUNG_AMB655XL08 6
#define DSI_PANEL_SAMSUNG_AMB670YF01 7
#define DSI_PANEL_DEFAULT_LABEL  "Default dsi panel"

#define DEFAULT_PANEL_JITTER_NUMERATOR		2
#define DEFAULT_PANEL_JITTER_DENOMINATOR	1
#define DEFAULT_PANEL_JITTER_ARRAY_SIZE		2
#define MAX_PANEL_JITTER		10
#define DEFAULT_PANEL_PREFILL_LINES	25
#define HIGH_REFRESH_RATE_THRESHOLD_TIME_US	500
#define MIN_PREFILL_LINES      40
static bool   hbm_finger_print = false;
static int    hbm_brightness_flag = 0;
static int    cur_backlight = -1;
static int    cur_fps = 60;
static int    cur_h = 1440;
static struct dsi_panel_cmd_set gamma_cmd_set[2];
char dimming_gamma_120hz[48] = {0};
char dimming_gamma_120hz_500step[48] = {0};
char b9_register_value[320] = {0};
char b9_register_value_500step[320] = {0};
static int parse_count = 0;


char gamma_para[2][413] = {
{
0x39,0x01,0x00,0x00,0x00,0x00,0x03,0xF0,0x5A,0x5A,
0x39,0x01,0x00,0x00,0x00,0x00,0x88,0xC8,
0xAA,0xA9,0x95,0x97,0x44,0xD8,0x52,0x08,
0x91,0x09,0xC7,0x44,0xA2,0x6B,0xDB,0x55,
0x10,0x40,0x55,0x25,0x89,0x28,0xFC,0x59,
0xFD,0xD7,0x29,0xBC,0xA0,0xDF,0x00,0x00,
0x00,0x9C,0x8F,0xB6,0x71,0x79,0x86,0x37,
0x37,0x37,0x00,0x00,0x00,0xAA,0xA9,0x95,
0x52,0x08,0x90,0x52,0x08,0x90,0x12,0xCF,
0x4F,0xAA,0x72,0xE4,0x55,0x54,0x40,0x5B,
0x29,0x8F,0x2E,0x01,0x5D,0x04,0xDD,0x30,
0xC8,0xAF,0xEF,0x00,0x00,0x00,0xAF,0x96,
0xC4,0x9F,0x85,0xA9,0x37,0x37,0x37,0x00,
0x00,0x00,0xAA,0xA9,0x95,0x52,0x08,0x90,
0x52,0x08,0x90,0x14,0xD0,0x51,0xAB,0x73,
0xE5,0x55,0x54,0x40,0x5E,0x2E,0x92,0x32,
0x04,0x5F,0x09,0xE3,0x35,0xD5,0xB9,0xFA,
0x00,0x00,0x00,0xC9,0xA9,0xDD,0xB6,0x9F,
0xBC,0x37,0x37,0x37,0x00,0x00,0x00,
0x39,0x01,0x00,0x00,0x00,0x00,0xB5,0xC9,
0xAA,0xA9,0x95,0x52,0x08,0x90,0x52,0x08,
0x90,0x15,0xD2,0x53,0xAD,0x74,0xE8,0x55,
0x54,0x41,0x63,0x35,0x9B,0x3F,0x14,0x71,
0x18,0xF1,0x46,0xE0,0xC6,0x06,0x00,0x00,
0x00,0xCE,0xC1,0xF0,0xBE,0xB5,0xBF,0x37,
0x37,0x37,0x00,0x00,0x00,0xAA,0xA9,0x95,
0x52,0x08,0x90,0x52,0x08,0x90,0x18,0xD4,
0x56,0xB1,0x78,0xEC,0x55,0x54,0x41,0x66,
0x38,0x9E,0x42,0x17,0x75,0x1A,0xF5,0x4D,
0xEE,0xD3,0x1C,0x04,0x00,0x00,0xDF,0xD7,
0x03,0xC8,0xC3,0xCF,0x37,0x37,0x37,0x00,
0x00,0x00,0xAA,0xA9,0x95,0x52,0x08,0x90,
0x52,0x08,0x90,0x0D,0xCB,0x4B,0xA8,0x73,
0xE4,0x55,0x55,0x51,0x61,0x35,0x97,0x3B,
0x17,0x70,0x20,0x00,0x53,0x01,0xE6,0x2F,
0x04,0x00,0x00,0xDD,0xE1,0x10,0xD2,0xCE,
0xF0,0x37,0x37,0x37,0x00,0x00,0x00,0xAA,
0xA9,0x95,0x52,0x08,0x90,0x52,0x08,0x90,
0x0A,0xCB,0x49,0xAE,0x7A,0xEA,0x55,0x55,
0x45,0x70,0x45,0xA4,0x4C,0x2D,0x82,0x30,
0x13,0x64,0xFF,0x04,0x3B,0x04,0x10,0x00,
0xFF,0xFD,0x36,0xF4,0xC5,0x1A,0x37,0x37,
0x37,0x00,0x00,0x00,
0x15,0x01,0x00,0x00,0x00,0x00,0x02,0xB0,0x02,
0x39,0x01,0x00,0x00,0x00,0x00,0x2E,0xB3,
0xAE,0xA9,0x95,0xC7,0x68,0x08,0x77,0x28,
0xB6,0x27,0xE1,0x63,0xB6,0x7D,0xF1,0x55,
0x54,0x40,0x64,0x31,0x98,0x34,0x07,0x67,
0x06,0xDF,0x33,0xC7,0xA3,0xE5,0x00,0x00,
0x00,0xA3,0x93,0xC6,0x0A,0x01,0x01,0x37,
0x37,0x37,0x00,0x00,0x00,
0x39,0x01,0x00,0x00,0x00,0x00,0x03,0xF0,0xA5,0xA5
},

{
0x39,0x01,0x00,0x00,0x00,0x00,0x03,0xF0,0x5A,0x5A,
0x39,0x01,0x00,0x00,0x00,0x00,0x88,0xC8,
0xAA,0xA9,0x95,0x97,0x44,0xD8,0x52,0x08,
0x90,0x09,0xC7,0x45,0xA2,0x6B,0xDB,0x55,
0x10,0x40,0x55,0x26,0x89,0x29,0xFF,0x59,
0xFE,0xDA,0x29,0xC6,0xAF,0xE5,0x00,0x00,
0x00,0xA6,0x97,0xBC,0x84,0x85,0x94,0x37,
0x37,0x37,0x00,0x00,0x00,0xAA,0xA9,0x95,
0x5A,0x21,0x9B,0x5A,0x21,0x9B,0x17,0xD3,
0x54,0xAB,0x73,0xE4,0x55,0x54,0x40,0x5F,
0x30,0x94,0x32,0x08,0x63,0x09,0xE6,0x35,
0xD9,0xC4,0xFD,0x00,0x00,0x00,0xCD,0xB9,
0xE7,0xAE,0x9F,0xB2,0x37,0x37,0x37,0x00,
0x00,0x00,0xAA,0xA9,0x95,0x5A,0x21,0x9B,
0x5A,0x21,0x9B,0x18,0xD4,0x55,0xAE,0x76,
0xE7,0x55,0x54,0x41,0x63,0x35,0x98,0x39,
0x11,0x68,0x16,0xF3,0x3E,0xF4,0xDD,0x1A,
0x04,0x00,0x00,0xEF,0xCF,0x06,0xD6,0xC4,
0xE5,0x37,0x37,0x37,0x00,0x00,0x00,
0x39,0x01,0x00,0x00,0x00,0x00,0xB5,0xC9,
0xAA,0xA9,0x95,0x5A,0x21,0x9B,0x5A,0x21,
0x9B,0x18,0xD3,0x54,0xB0,0x78,0xE9,0x55,
0x55,0x51,0x67,0x3A,0x9C,0x43,0x1A,0x6F,
0x24,0x00,0x4D,0x0A,0xEB,0x31,0x44,0x10,
0x00,0x07,0xE0,0x23,0xF1,0xD5,0x05,0x37,
0x37,0x37,0x00,0x00,0x00,0xAA,0xA9,0x95,
0x5A,0x21,0x9B,0x5A,0x21,0x9B,0x18,0xD4,
0x55,0xB3,0x7B,0xEB,0x55,0x55,0x51,0x6E,
0x42,0x9F,0x47,0x23,0x75,0x30,0x0E,0x55,
0x1B,0xFA,0x3C,0x44,0x10,0x00,0x0E,0xEF,
0x2D,0xF8,0xD8,0x0C,0x37,0x37,0x37,0x00,
0x00,0x00,0xAA,0xA9,0x95,0x5A,0x21,0x9B,
0x5A,0x21,0x9B,0x0D,0xCB,0x49,0xAF,0x7B,
0xE8,0x55,0x55,0x55,0x72,0x48,0xA2,0x51,
0x2C,0x79,0x3C,0x1A,0x5E,0x2C,0x08,0x46,
0x45,0x10,0x00,0x15,0xF4,0x40,0x05,0xDE,
0x24,0x37,0x37,0x37,0x00,0x00,0x00,0xAA,
0xA9,0x95,0x5A,0x21,0x9B,0x5A,0x21,0x9B,
0x09,0xCC,0x44,0xB7,0x89,0xED,0x55,0x55,
0x55,0x87,0x5F,0xAE,0x6F,0x4C,0x8D,0x63,
0x3F,0x7D,0x56,0x30,0x6C,0x55,0x50,0x00,
0x3B,0x1B,0x62,0x2B,0x03,0x45,0x37,0x37,
0x37,0x00,0x00,0x00,
0x15,0x01,0x00,0x00,0x00,0x00,0x02,0xB0,0x02,
0x39,0x01,0x00,0x00,0x00,0x00,0x2E,0xB3,
0xAE,0xA9,0x95,0xC7,0x68,0x08,0x74,0x26,
0xB3,0x22,0xDC,0x5F,0xB5,0x7C,0xF0,0x55,
0x54,0x40,0x62,0x31,0x98,0x34,0x07,0x66,
0x06,0xDF,0x32,0xC9,0xA9,0xE9,0x00,0x00,
0x00,0xAD,0x9D,0xCE,0x0D,0x01,0x01,0x37,
0x37,0x37,0x00,0x00,0x00,
0x39,0x01,0x00,0x00,0x00,0x00,0x03,0xF0,0xA5,0xA5
}

};
EXPORT_SYMBOL(gamma_para);

const char *gamma_cmd_set_map[DSI_GAMMA_CMD_SET_MAX] = {
	"dsi-gamma-cmd-set-switch-60hz",
	"dsi-gamma-cmd-set-switch-90hz",
};

int gamma_read_flag = GAMMA_READ_SUCCESS;

char dsi_panel_name = DSI_PANEL_SAMSUNG_S6E3FC2X01;
EXPORT_SYMBOL(dsi_panel_name);

extern void pm_print_active_wakeup_sources_queue(bool on);

static void dsi_dce_prepare_pps_header(char *buf, u32 pps_delay_ms)
{
	char *bp;

	bp = buf;
	/* First 7 bytes are cmd header */
	*bp++ = 0x0A;
	*bp++ = 1;
	*bp++ = 0;
	*bp++ = 0;
	*bp++ = pps_delay_ms;
	*bp++ = 0;
	*bp++ = 128;
}

static int dsi_dsc_create_pps_buf_cmd(struct msm_display_dsc_info *dsc,
	char *buf, int pps_id, u32 size)
{
	dsi_dce_prepare_pps_header(buf, dsc->pps_delay_ms);
	buf += DSI_CMD_PPS_HDR_SIZE;
	return sde_dsc_create_pps_buf_cmd(dsc, buf, pps_id,
			size);
}

static int dsi_vdc_create_pps_buf_cmd(struct msm_display_vdc_info *vdc,
	char *buf, int pps_id, u32 size)
{
	dsi_dce_prepare_pps_header(buf, vdc->pps_delay_ms);
	buf += DSI_CMD_PPS_HDR_SIZE;
	return sde_vdc_create_pps_buf_cmd(vdc, buf, pps_id,
			size);
}

static int dsi_panel_vreg_get(struct dsi_panel *panel)
{
	int rc = 0;
	int i;
	struct regulator *vreg = NULL;

	for (i = 0; i < panel->power_info.count; i++) {
		vreg = devm_regulator_get(panel->parent,
					  panel->power_info.vregs[i].vreg_name);
		rc = PTR_RET(vreg);
		if (rc) {
			DSI_ERR("failed to get %s regulator\n",
			       panel->power_info.vregs[i].vreg_name);
			goto error_put;
		}
		panel->power_info.vregs[i].vreg = vreg;
	}

	return rc;
error_put:
	for (i = i - 1; i >= 0; i--) {
		devm_regulator_put(panel->power_info.vregs[i].vreg);
		panel->power_info.vregs[i].vreg = NULL;
	}
	return rc;
}

static int dsi_panel_vreg_put(struct dsi_panel *panel)
{
	int rc = 0;
	int i;

	for (i = panel->power_info.count - 1; i >= 0; i--)
		devm_regulator_put(panel->power_info.vregs[i].vreg);

	return rc;
}

static int dsi_panel_gpio_request(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_panel_reset_config *r_config = &panel->reset_config;

	if (gpio_is_valid(r_config->reset_gpio)) {
		rc = gpio_request(r_config->reset_gpio, "reset_gpio");
		if (rc) {
			DSI_ERR("request for reset_gpio failed, rc=%d\n", rc);
#if defined(CONFIG_PXLW_IRIS)
			if (iris_is_chip_supported()) {
				if (!strcmp(panel->type, "primary"))
					goto error;
				rc = 0;
			} else
#endif
			goto error;
		}
	}

	if (gpio_is_valid(r_config->disp_en_gpio)) {
		rc = gpio_request(r_config->disp_en_gpio, "disp_en_gpio");
		if (rc) {
			DSI_ERR("request for disp_en_gpio failed, rc=%d\n", rc);
			goto error_release_reset;
		}
	}

	if (gpio_is_valid(panel->bl_config.en_gpio)) {
		rc = gpio_request(panel->bl_config.en_gpio, "bklt_en_gpio");
		if (rc) {
			DSI_ERR("request for bklt_en_gpio failed, rc=%d\n", rc);
			goto error_release_disp_en;
		}
	}

	if (gpio_is_valid(r_config->lcd_mode_sel_gpio)) {
		rc = gpio_request(r_config->lcd_mode_sel_gpio, "mode_gpio");
		if (rc) {
			DSI_ERR("request for mode_gpio failed, rc=%d\n", rc);
			goto error_release_mode_sel;
		}
	}

	if (gpio_is_valid(panel->poc)) {
		rc = gpio_request(panel->poc, "platform_poc_gpio");
		if (rc) {
			DSI_ERR("request for platform_poc_gpio failed, rc=%d\n", rc);
			goto error_release_lcd_mode_sel;
		}
	}

	if (gpio_is_valid(panel->vddr_gpio)) {
		rc = gpio_request(panel->vddr_gpio, "vddr_gpio");
		if (rc) {
			DSI_ERR("request for vddr_gpio failed, rc=%d\n", rc);
			goto error_release_poc;
		}
	}

	if (gpio_is_valid(panel->avdd_gpio)) {
		rc = gpio_request(panel->avdd_gpio, "avdd_gpio");
		if (rc) {
			DSI_ERR("request for avdd_gpio failed, rc=%d\n", rc);
			goto error_release_avdd;
		}
	}

	if (gpio_is_valid(panel->tp1v8_gpio)) {
		rc = gpio_request(panel->tp1v8_gpio, "tp1v8_gpio");
		if (rc) {
			DSI_ERR("request for tp1v8_gpio failed, rc=%d\n", rc);
			goto error_release_vddr;
		}
	}

	if (gpio_is_valid(panel->panel_test_gpio)) {
		rc = gpio_request(panel->panel_test_gpio, "panel_test_gpio");
		if (rc) {
			DSI_ERR("request for panel_test_gpio failed, rc=%d\n",
				 rc);
			panel->panel_test_gpio = -1;
			rc = 0;
			goto error_release_tp1v8_gpio;
		}
	}

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		if (gpio_is_valid(panel->vsync_switch_gpio)) {
			rc = gpio_request(panel->vsync_switch_gpio, "vsync_switch_gpio");
			if (rc) {
				DSI_ERR("adfr request for vsync_switch_gpio failed, rc=%d\n", rc);
				goto error_release_test_gpio;
			}
		}
	}
#endif /*(CONFIG_PXLW_IRIS)*/

	goto error;

error_release_test_gpio:
	if (gpio_is_valid(panel->panel_test_gpio))
		gpio_free(panel->panel_test_gpio);
error_release_tp1v8_gpio:
	if (gpio_is_valid(panel->tp1v8_gpio))
		gpio_free(panel->tp1v8_gpio);
error_release_vddr:
	if (gpio_is_valid(panel->vddr_gpio))
		gpio_free(panel->vddr_gpio);
error_release_avdd:
	if (gpio_is_valid(panel->avdd_gpio))
		gpio_free(panel->avdd_gpio);
error_release_poc:
	if (gpio_is_valid(panel->poc))
		gpio_free(panel->poc);
error_release_lcd_mode_sel:
	if (gpio_is_valid(r_config->lcd_mode_sel_gpio))
		gpio_free(r_config->lcd_mode_sel_gpio);
error_release_mode_sel:
	if (gpio_is_valid(panel->bl_config.en_gpio))
		gpio_free(panel->bl_config.en_gpio);
error_release_disp_en:
	if (gpio_is_valid(r_config->disp_en_gpio))
		gpio_free(r_config->disp_en_gpio);
error_release_reset:
	if (gpio_is_valid(r_config->reset_gpio))
		gpio_free(r_config->reset_gpio);
error:
	return rc;
}

static int dsi_panel_gpio_release(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_panel_reset_config *r_config = &panel->reset_config;

	if (gpio_is_valid(r_config->reset_gpio))
		gpio_free(r_config->reset_gpio);

	if (gpio_is_valid(r_config->disp_en_gpio))
		gpio_free(r_config->disp_en_gpio);

	if (gpio_is_valid(panel->bl_config.en_gpio))
		gpio_free(panel->bl_config.en_gpio);

	if (gpio_is_valid(panel->reset_config.lcd_mode_sel_gpio))
		gpio_free(panel->reset_config.lcd_mode_sel_gpio);

	if (gpio_is_valid(panel->poc))
		gpio_free(panel->poc);

	if (gpio_is_valid(panel->vddr_gpio))
		gpio_free(panel->vddr_gpio);

	if (gpio_is_valid(panel->avdd_gpio))
		gpio_free(panel->avdd_gpio);

	if (gpio_is_valid(panel->tp1v8_gpio))
		gpio_free(panel->tp1v8_gpio);

	if (gpio_is_valid(panel->panel_test_gpio))
		gpio_free(panel->panel_test_gpio);

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		if (gpio_is_valid(panel->vsync_switch_gpio))
			gpio_free(panel->vsync_switch_gpio);
	}
#endif /*(CONFIG_PXLW_IRIS)*/

	return rc;
}

int dsi_panel_trigger_esd_attack(struct dsi_panel *panel, bool trusted_vm_env)
{
	if (!panel) {
		DSI_ERR("Invalid panel param\n");
		return -EINVAL;
	}

	/* toggle reset-gpio by writing directly to register in trusted-vm */
	if (trusted_vm_env) {
		struct dsi_tlmm_gpio *gpio = NULL;
		void __iomem *io;
		u32 offset = 0x4;
		int i;

		for (i = 0; i < panel->tlmm_gpio_count; i++)
			if (!strcmp(panel->tlmm_gpio[i].name, "reset-gpio"))
				gpio = &panel->tlmm_gpio[i];

		if (!gpio) {
			DSI_ERR("reset gpio not found\n");
			return -EINVAL;
		}

		io = ioremap(gpio->addr, gpio->size);
		writel_relaxed(0, io + offset);
		iounmap(io);

	} else {
		struct dsi_panel_reset_config *r_config = &panel->reset_config;

		if (!r_config) {
			DSI_ERR("Invalid panel reset configuration\n");
			return -EINVAL;
		}

		if (!gpio_is_valid(r_config->reset_gpio)) {
			DSI_ERR("failed to pull down gpio\n");
			return -EINVAL;
		}
		gpio_set_value(r_config->reset_gpio, 0);
	}

	SDE_EVT32(SDE_EVTLOG_FUNC_CASE1);
	DSI_INFO("GPIO pulled low to simulate ESD\n");

	return 0;
}

static int dsi_panel_reset(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_panel_reset_config *r_config = &panel->reset_config;
	int i;
#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_dual_supported() && panel->is_secondary)
		return rc;

	iris_reset();
#endif

	if (gpio_is_valid(panel->reset_config.disp_en_gpio)) {
		rc = gpio_direction_output(panel->reset_config.disp_en_gpio, 1);
		if (rc) {
			DSI_ERR("unable to set dir for disp gpio rc=%d\n", rc);
			goto exit;
		}
	}

	if (r_config->count) {
		rc = gpio_direction_output(r_config->reset_gpio,
			r_config->sequence[0].level);
		if (rc) {
			DSI_ERR("unable to set dir for rst gpio rc=%d\n", rc);
			goto exit;
		}
	}

	for (i = 0; i < r_config->count; i++) {
		gpio_set_value(r_config->reset_gpio,
			       r_config->sequence[i].level);


		if (r_config->sequence[i].sleep_ms)
			usleep_range(r_config->sequence[i].sleep_ms * 1000,
				(r_config->sequence[i].sleep_ms * 1000) + 100);
	}

	if (gpio_is_valid(panel->bl_config.en_gpio)) {
		rc = gpio_direction_output(panel->bl_config.en_gpio, 1);
		if (rc)
			DSI_ERR("unable to set dir for bklt gpio rc=%d\n", rc);
	}

	if (gpio_is_valid(panel->reset_config.lcd_mode_sel_gpio)) {
		bool out = true;

		if ((panel->reset_config.mode_sel_state == MODE_SEL_DUAL_PORT)
				|| (panel->reset_config.mode_sel_state
					== MODE_GPIO_LOW))
			out = false;
		else if ((panel->reset_config.mode_sel_state
				== MODE_SEL_SINGLE_PORT) ||
				(panel->reset_config.mode_sel_state
				 == MODE_GPIO_HIGH))
			out = true;

		rc = gpio_direction_output(
			panel->reset_config.lcd_mode_sel_gpio, out);
		if (rc)
			DSI_ERR("unable to set dir for mode gpio rc=%d\n", rc);
	}

	if (gpio_is_valid(panel->panel_test_gpio)) {
		rc = gpio_direction_input(panel->panel_test_gpio);
		if (rc)
			DSI_WARN("unable to set dir for panel test gpio rc=%d\n",
					rc);
	}

exit:
	return rc;
}

static int dsi_panel_set_pinctrl_state(struct dsi_panel *panel, bool enable)
{
	int rc = 0;
	struct pinctrl_state *state;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	if (!panel->pinctrl.pinctrl)
		return 0;

	if (enable)
		state = panel->pinctrl.active;
	else
		state = panel->pinctrl.suspend;

	rc = pinctrl_select_state(panel->pinctrl.pinctrl, state);
	if (rc)
		DSI_ERR("[%s] failed to set pin state, rc=%d\n",
				panel->name, rc);

	return rc;
}


static int dsi_panel_power_on(struct dsi_panel *panel)
{
	int rc = 0;

	rc = dsi_pwr_enable_regulator(&panel->power_info, true);
	DSI_ERR("enable pwr regulator\n");
	if (rc) {
		DSI_ERR("[%s] failed to enable vregs, rc=%d\n",
				panel->name, rc);
		goto exit;
	}

	if (gpio_is_valid(panel->poc)) {
		rc = gpio_direction_output(panel->poc, 1);
		DSI_ERR("enable poc gpio\n");
		if (rc) {
			DSI_ERR("unable to set dir for poc gpio rc=%d\n", rc);
			goto error_disable_vregs;
		}
	}

	if (gpio_is_valid(panel->vddr_gpio)) {
		rc = gpio_direction_output(panel->vddr_gpio, 1);
		DSI_ERR("enable vddr gpio\n");
		if (rc) {
			DSI_ERR("unable to set dir for vddr gpio rc=%d\n", rc);
			goto error_disable_poc;
		}
	}

	if (gpio_is_valid(panel->avdd_gpio)) {
		rc = gpio_direction_output(panel->avdd_gpio, 1);
		DSI_ERR("enable avdd gpio\n");
		if (rc) {
			DSI_ERR("unable to set dir for avdd gpio rc=%d\n", rc);
			goto error_disable_avdd;
		}
	}

	rc = dsi_panel_set_pinctrl_state(panel, true);
	DSI_ERR("set dsi panel pinctrl state true\n");
	if (rc) {
		DSI_ERR("[%s] failed to set pinctrl, rc=%d\n", panel->name, rc);
		goto error_disable_vddr;
	}

	rc = dsi_panel_reset(panel);
	DSI_ERR("reset panel\n");
	if (rc) {
		DSI_ERR("[%s] failed to reset panel, rc=%d\n", panel->name, rc);
		goto error_disable_gpio;
	}

	goto exit;

error_disable_gpio:
	if (gpio_is_valid(panel->reset_config.disp_en_gpio))
		gpio_set_value(panel->reset_config.disp_en_gpio, 0);

	if (gpio_is_valid(panel->bl_config.en_gpio))
		gpio_set_value(panel->bl_config.en_gpio, 0);

	(void)dsi_panel_set_pinctrl_state(panel, false);

error_disable_vddr:
	if (gpio_is_valid(panel->vddr_gpio))
		gpio_set_value(panel->vddr_gpio, 0);

error_disable_poc:
	if (gpio_is_valid(panel->poc))
		gpio_set_value(panel->poc, 0);

error_disable_avdd:
	if (gpio_is_valid(panel->avdd_gpio))
		gpio_set_value(panel->avdd_gpio, 0);

error_disable_vregs:
	(void)dsi_pwr_enable_regulator(&panel->power_info, false);

exit:
	return rc;
}
extern u8 alpha_debug_mode;

static int dsi_panel_power_off(struct dsi_panel *panel)
{
	int rc = 0;

	if (gpio_is_valid(panel->reset_config.disp_en_gpio))
		gpio_set_value(panel->reset_config.disp_en_gpio, 0);

	if (gpio_is_valid(panel->reset_config.reset_gpio) &&
					!panel->reset_gpio_always_on) {
		gpio_set_value(panel->reset_config.reset_gpio, 0);
		DSI_ERR("disable reset gpio\n");
	}
#if defined(CONFIG_PXLW_IRIS)
	iris_power_off(panel);
#endif
	if (gpio_is_valid(panel->reset_config.lcd_mode_sel_gpio))
		gpio_set_value(panel->reset_config.lcd_mode_sel_gpio, 0);

	rc = dsi_panel_set_pinctrl_state(panel, false);
	DSI_ERR("set dsi panel pinctrl state false\n");
	if (rc) {
		DSI_ERR("[%s] failed set pinctrl state, rc=%d\n", panel->name,
		       rc);
	}

	if (strcmp(panel->name, "samsung sofef03f_m fhd cmd mode dsc dsi panel") == 0) {
		msleep(10);
	} else {
		msleep(1);
	}

	if (gpio_is_valid(panel->vddr_gpio)) {
		gpio_set_value(panel->vddr_gpio, 0);
		DSI_ERR("disable vddr gpio\n");
		msleep(1);
	}

	if (gpio_is_valid(panel->avdd_gpio)) {
		gpio_set_value(panel->avdd_gpio, 0);
		DSI_ERR("disable avdd gpio\n");
	}

	if (gpio_is_valid(panel->poc)) {
		gpio_set_value(panel->poc, 0);
		DSI_ERR("disable poc gpio\n");
	}

	if (gpio_is_valid(panel->panel_test_gpio)) {
		rc = gpio_direction_input(panel->panel_test_gpio);
		if (rc)
			DSI_WARN("set dir for panel test gpio failed rc=%d\n",
				 rc);
	}

	rc = dsi_pwr_enable_regulator(&panel->power_info, false);
	DSI_ERR("disable dsi pwr regulator\n");
	if (rc)
		DSI_ERR("[%s] failed to enable vregs, rc=%d\n",
				panel->name, rc);
	if(alpha_debug_mode)
		alpha_debug_mode = false;

	return rc;
}

int dsi_panel_tx_cmd_set(struct dsi_panel *panel,
				enum dsi_cmd_set_type type)
{
	int rc = 0, i = 0;
	ssize_t len;
	struct dsi_cmd_desc *cmds;
	u32 count;
	enum dsi_cmd_set_state state;
	struct dsi_display_mode *mode;
	const struct mipi_dsi_host_ops *ops = panel->host->ops;

	if (!panel || !panel->cur_mode)
		return -EINVAL;

	mode = panel->cur_mode;

	cmds = mode->priv_info->cmd_sets[type].cmds;
	count = mode->priv_info->cmd_sets[type].count;
	state = mode->priv_info->cmd_sets[type].state;
	SDE_EVT32(type, state, count);

	if (count == 0) {
		DSI_DEBUG("[%s] No commands to be sent for state(%d)\n",
			 panel->name, type);
		goto error;
	}

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported() && iris_is_pt_mode(panel)) {
		rc = iris_pt_send_panel_cmd(panel, &(mode->priv_info->cmd_sets[type]));
		if (rc)
			DSI_ERR("iris_pt_send_panel_cmd failed\n");
		return rc;
	}
#endif
	for (i = 0; i < count; i++) {
		if (state == DSI_CMD_SET_STATE_LP)
			cmds->msg.flags |= MIPI_DSI_MSG_USE_LPM;

		if (cmds->last_command)
			cmds->msg.flags |= MIPI_DSI_MSG_LASTCOMMAND;

		if (type == DSI_CMD_SET_VID_TO_CMD_SWITCH)
			cmds->msg.flags |= MIPI_DSI_MSG_ASYNC_OVERRIDE;

		len = ops->transfer(panel->host, &cmds->msg);
		if (len < 0) {
			rc = len;
			DSI_ERR("failed to set cmds(%d), rc=%d\n", type, rc);
			goto error;
		}
		if (cmds->post_wait_ms)
			usleep_range(cmds->post_wait_ms*1000,
					((cmds->post_wait_ms*1000)+10));
		cmds++;
	}
error:
	return rc;
}

static int dsi_panel_pinctrl_deinit(struct dsi_panel *panel)
{
	int rc = 0;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	devm_pinctrl_put(panel->pinctrl.pinctrl);

	return rc;
}

static int dsi_panel_pinctrl_init(struct dsi_panel *panel)
{
	int rc = 0;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	/* TODO:  pinctrl is defined in dsi dt node */
	panel->pinctrl.pinctrl = devm_pinctrl_get(panel->parent);
	if (IS_ERR_OR_NULL(panel->pinctrl.pinctrl)) {
		rc = PTR_ERR(panel->pinctrl.pinctrl);
		DSI_ERR("failed to get pinctrl, rc=%d\n", rc);
		goto error;
	}

	panel->pinctrl.active = pinctrl_lookup_state(panel->pinctrl.pinctrl,
						       "panel_active");
	if (IS_ERR_OR_NULL(panel->pinctrl.active)) {
		rc = PTR_ERR(panel->pinctrl.active);
		DSI_ERR("failed to get pinctrl active state, rc=%d\n", rc);
		goto error;
	}

	panel->pinctrl.suspend =
		pinctrl_lookup_state(panel->pinctrl.pinctrl, "panel_suspend");

	if (IS_ERR_OR_NULL(panel->pinctrl.suspend)) {
		rc = PTR_ERR(panel->pinctrl.suspend);
		DSI_ERR("failed to get pinctrl suspend state, rc=%d\n", rc);
		goto error;
	}

	panel->pinctrl.pwm_pin =
		pinctrl_lookup_state(panel->pinctrl.pinctrl, "pwm_pin");

	if (IS_ERR_OR_NULL(panel->pinctrl.pwm_pin)) {
		panel->pinctrl.pwm_pin = NULL;
		DSI_DEBUG("failed to get pinctrl pwm_pin");
	}

error:
	return rc;
}

static int dsi_panel_wled_register(struct dsi_panel *panel,
		struct dsi_backlight_config *bl)
{
	struct backlight_device *bd;

	bd = backlight_device_get_by_type(BACKLIGHT_RAW);
	if (!bd) {
		DSI_ERR("[%s] fail raw backlight register rc=%d\n",
				panel->name, -EPROBE_DEFER);
		return -EPROBE_DEFER;
	}

	bl->raw_bd = bd;
	return 0;
}
bool HBM_flag =false;

int dsi_panel_gamma_read_address_setting(struct dsi_panel *panel, u16 read_number)
{
	int rc = 0;
	struct mipi_dsi_device *dsi;

	if (!panel || (read_number > 0xffff)) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	dsi = &panel->mipi_device;

	rc = mipi_dsi_dcs_write_c1(dsi, read_number);

	return rc;
}

extern int op_dimlayer_bl_alpha;
extern int op_dimlayer_bl_enabled;
extern int op_dimlayer_bl_enable_real;
extern int cur_brightness;

static int saved_backlight = -1;
int dsi_panel_backlight_get(void)
{
		return saved_backlight;
}

static int dsi_panel_update_backlight(struct dsi_panel *panel,
	u32 bl_lvl)
{
	int rc = 0;
	u32 count;
	struct mipi_dsi_device *dsi;
	struct dsi_display_mode *mode;
	if (!panel || (bl_lvl > 0xffff) || !panel->cur_mode) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	if(alpha_debug_mode)
		bl_lvl = cur_brightness;

	dsi = &panel->mipi_device;

	mode = panel->cur_mode;

	saved_backlight = bl_lvl;
	if (panel->is_hbm_enabled) {
		hbm_finger_print = true;
		DSI_ERR("HBM is enabled\n");
		return 0;
	}

	if (op_dimlayer_bl_enabled != op_dimlayer_bl_enable_real) {
		op_dimlayer_bl_enable_real = op_dimlayer_bl_enabled;
		if (op_dimlayer_bl_enable_real && (bl_lvl != 0) && (bl_lvl < op_dimlayer_bl_alpha))
			bl_lvl = op_dimlayer_bl_alpha;
		DSI_ERR("dc light %d %d\n", op_dimlayer_bl_enable_real, bl_lvl);
	}
	if (op_dimlayer_bl_enable_real && (bl_lvl != 0) && (bl_lvl < op_dimlayer_bl_alpha))
		bl_lvl = op_dimlayer_bl_alpha;

	if (panel->bl_config.bl_high2bit) {
		if (HBM_flag == true)
			return 0;

		if (((cur_backlight == bl_lvl) || (hbm_brightness_flag == 1))
			&& (mode_fps != cur_fps || cur_h != panel->cur_mode->timing.h_active) && !hbm_finger_print) {
			cur_fps = mode_fps;
			cur_h = panel->cur_mode->timing.h_active;
			return 0;
		}

		if (hbm_brightness_flag == 1) {
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_BRIGHTNESS_OFF].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM brightness off mode.\n");
				goto error;
			}
			else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_BRIGHTNESS_OFF);
				DSI_ERR("Send DSI_CMD_SET_HBM_BRIGHTNESS_OFF cmds.\n");
				hbm_brightness_flag = 0;
			}
		}

		DSI_ERR("backlight = %d\n", bl_lvl);

		if (strcmp(panel->name, "samsung amb670yf01 dsc cmd mode panel") == 0) {
			if(bl_lvl >= PANEL_MAX_NOMAL_BRIGHTNESS)
				bl_lvl = PANEL_MAX_NOMAL_BRIGHTNESS-1;
			bl_lvl = backlight_buf[bl_lvl];
		}

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported() && iris_is_pt_mode(panel))
		rc = iris_update_backlight(1, bl_lvl);
	else
#endif
		rc = mipi_dsi_dcs_set_display_brightness_samsung(get_main_display(), bl_lvl);
		cur_backlight = bl_lvl;
		cur_fps = mode_fps;
		cur_h = panel->cur_mode->timing.h_active;
		hbm_finger_print = false;
	} else {

	if (panel->bl_config.bl_inverted_dbv)
		bl_lvl = (((bl_lvl & 0xff) << 8) | (bl_lvl >> 8));

#if defined(CONFIG_PXLW_IRIS)
		if (iris_is_chip_supported() && iris_is_pt_mode(panel))
			rc = iris_update_backlight(1, bl_lvl);
		else
#endif
			rc = mipi_dsi_dcs_set_display_brightness(dsi, bl_lvl);
	}
	if (rc < 0)
		DSI_ERR("failed to update dcs backlight:%d\n", bl_lvl);

error:
	return rc;
}

int dsi_panel_op_set_hbm_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

    mode = panel->cur_mode;
    switch (level) {
    case 0:
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_OFF].count;
        if (!count) {
            DSI_ERR("This panel does not support HBM mode off.\n");
            goto error;
        } else {
            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_OFF);
			printk(KERN_ERR"When HBM OFF -->hbm_backight = %d panel->bl_config.bl_level =%d\n",panel->hbm_backlight,panel->bl_config.bl_level);
			rc= dsi_panel_update_backlight(panel,panel->hbm_backlight);
        }
    break;

    case 1:
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_ON_5].count;
        if (!count) {
            DSI_ERR("This panel does not support HBM mode.\n");
            goto error;
        } else {
            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_ON_5);
        }
    break;
    default:
    break;

    }
    DSI_ERR("Set HBM Mode = %d\n", level);
	if(level==5)
	{
		DSI_ERR("HBM == 5 for fingerprint\n");
	}

error:
	mutex_unlock(&panel->panel_lock);

	return rc;
}

static int dsi_panel_update_pwm_backlight(struct dsi_panel *panel,
	u32 bl_lvl)
{
	int rc = 0;
	u32 duty = 0;
	u32 period_ns = 0;
	struct dsi_backlight_config *bl;

	if (!panel) {
		DSI_ERR("Invalid Params\n");
		return -EINVAL;
	}

	bl = &panel->bl_config;
	if (!bl->pwm_bl) {
		DSI_ERR("pwm device not found\n");
		return -EINVAL;
	}

	period_ns = bl->pwm_period_usecs * NSEC_PER_USEC;
	duty = bl_lvl * period_ns;
	duty /= bl->bl_max_level;

	rc = pwm_config(bl->pwm_bl, duty, period_ns);
	if (rc) {
		DSI_ERR("[%s] failed to change pwm config, rc=\n", panel->name,
			rc);
		goto error;
	}

	if (bl_lvl == 0 && bl->pwm_enabled) {
		pwm_disable(bl->pwm_bl);
		bl->pwm_enabled = false;
		return 0;
	}

	if (bl_lvl != 0 && !bl->pwm_enabled) {
		rc = pwm_enable(bl->pwm_bl);
		if (rc) {
			DSI_ERR("[%s] failed to enable pwm, rc=\n", panel->name,
				rc);
			goto error;
		}

		bl->pwm_enabled = true;
	}

error:
	return rc;
}

int dsi_panel_set_backlight(struct dsi_panel *panel, u32 bl_lvl)
{
	int rc = 0;
	struct dsi_backlight_config *bl = &panel->bl_config;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	DSI_DEBUG("backlight type:%d lvl:%d\n", bl->type, bl_lvl);
	switch (bl->type) {
	case DSI_BACKLIGHT_WLED:
		rc = backlight_device_set_brightness(bl->raw_bd, bl_lvl);
		break;
	case DSI_BACKLIGHT_DCS:
		panel->hbm_backlight = bl_lvl;
		rc = dsi_panel_update_backlight(panel, bl_lvl);
		break;
	case DSI_BACKLIGHT_EXTERNAL:
		break;
	case DSI_BACKLIGHT_PWM:
		rc = dsi_panel_update_pwm_backlight(panel, bl_lvl);
		break;
	default:
		DSI_ERR("Backlight type(%d) not supported\n", bl->type);
		rc = -ENOTSUPP;
	}

	return rc;
}

static u32 dsi_panel_get_brightness(struct dsi_backlight_config *bl)
{
	u32 cur_bl_level;
	struct backlight_device *bd = bl->raw_bd;

	/* default the brightness level to 50% */
	cur_bl_level = bl->bl_max_level >> 1;

	switch (bl->type) {
	case DSI_BACKLIGHT_WLED:
		/* Try to query the backlight level from the backlight device */
		if (bd->ops && bd->ops->get_brightness)
			cur_bl_level = bd->ops->get_brightness(bd);
		break;
	case DSI_BACKLIGHT_DCS:
	case DSI_BACKLIGHT_EXTERNAL:
	case DSI_BACKLIGHT_PWM:
	default:
		/*
		 * Ideally, we should read the backlight level from the
		 * panel. For now, just set it default value.
		 */
		break;
	}

	DSI_DEBUG("cur_bl_level=%d\n", cur_bl_level);
	return cur_bl_level;
}

void dsi_panel_bl_handoff(struct dsi_panel *panel)
{
	struct dsi_backlight_config *bl = &panel->bl_config;

	bl->bl_level = dsi_panel_get_brightness(bl);
}

static int dsi_panel_pwm_register(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_backlight_config *bl = &panel->bl_config;

	bl->pwm_bl = devm_of_pwm_get(panel->parent, panel->panel_of_node, NULL);
	if (IS_ERR_OR_NULL(bl->pwm_bl)) {
		rc = PTR_ERR(bl->pwm_bl);
		DSI_ERR("[%s] failed to request pwm, rc=%d\n", panel->name,
			rc);
		return rc;
	}

	if (panel->pinctrl.pwm_pin) {
		rc = pinctrl_select_state(panel->pinctrl.pinctrl,
			panel->pinctrl.pwm_pin);
		if (rc)
			DSI_ERR("[%s] failed to set pwm pinctrl, rc=%d\n",
				panel->name, rc);
	}

	return 0;
}

static int dsi_panel_bl_register(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_backlight_config *bl = &panel->bl_config;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	switch (bl->type) {
	case DSI_BACKLIGHT_WLED:
		rc = dsi_panel_wled_register(panel, bl);
		break;
	case DSI_BACKLIGHT_DCS:
		break;
	case DSI_BACKLIGHT_EXTERNAL:
		break;
	case DSI_BACKLIGHT_PWM:
		rc = dsi_panel_pwm_register(panel);
		break;
	default:
		DSI_ERR("Backlight type(%d) not supported\n", bl->type);
		rc = -ENOTSUPP;
		goto error;
	}

error:
	return rc;
}

static void dsi_panel_pwm_unregister(struct dsi_panel *panel)
{
	struct dsi_backlight_config *bl = &panel->bl_config;

	devm_pwm_put(panel->parent, bl->pwm_bl);
}

static int dsi_panel_bl_unregister(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_backlight_config *bl = &panel->bl_config;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	switch (bl->type) {
	case DSI_BACKLIGHT_WLED:
		break;
	case DSI_BACKLIGHT_DCS:
		break;
	case DSI_BACKLIGHT_EXTERNAL:
		break;
	case DSI_BACKLIGHT_PWM:
		dsi_panel_pwm_unregister(panel);
		break;
	default:
		DSI_ERR("Backlight type(%d) not supported\n", bl->type);
		rc = -ENOTSUPP;
		goto error;
	}

error:
	return rc;
}

static int dsi_panel_parse_timing(struct dsi_mode_info *mode,
				  struct dsi_parser_utils *utils)
{
	int rc = 0;
	u64 tmp64 = 0;
	struct dsi_display_mode *display_mode;
	struct dsi_display_mode_priv_info *priv_info;

	display_mode = container_of(mode, struct dsi_display_mode, timing);

	priv_info = display_mode->priv_info;

	rc = utils->read_u64(utils->data,
			"qcom,mdss-dsi-panel-clockrate", &tmp64);
	if (rc == -EOVERFLOW) {
		tmp64 = 0;
		rc = utils->read_u32(utils->data,
			"qcom,mdss-dsi-panel-clockrate", (u32 *)&tmp64);
	}

	mode->clk_rate_hz = !rc ? tmp64 : 0;
	display_mode->priv_info->clk_rate_hz = mode->clk_rate_hz;

	mode->pclk_scale.numer = 1;
	mode->pclk_scale.denom = 1;
	display_mode->priv_info->pclk_scale = mode->pclk_scale;

	rc = utils->read_u32(utils->data, "qcom,mdss-mdp-transfer-time-us",
				&mode->mdp_transfer_time_us);
	if (!rc)
		display_mode->priv_info->mdp_transfer_time_us =
			mode->mdp_transfer_time_us;
	else
		display_mode->priv_info->mdp_transfer_time_us = 0;

	rc = utils->read_u32(utils->data,
				"qcom,mdss-dsi-panel-framerate",
				&mode->refresh_rate);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-panel-framerate, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-panel-width",
				  &mode->h_active);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-panel-width, rc=%d\n",
				rc);
		goto error;
	}

	rc = utils->read_u32(utils->data,
				"qcom,mdss-dsi-h-front-porch",
				  &mode->h_front_porch);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-h-front-porch, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data,
				"qcom,mdss-dsi-h-back-porch",
				  &mode->h_back_porch);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-h-back-porch, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data,
				"qcom,mdss-dsi-h-pulse-width",
				  &mode->h_sync_width);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-h-pulse-width, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-h-sync-skew",
				  &mode->h_skew);
	if (rc)
		DSI_DEBUG("qcom,mdss-dsi-h-sync-skew is not defined, rc=%d\n",
				rc);

	DSI_DEBUG("panel horz active:%d front_portch:%d back_porch:%d sync_skew:%d\n",
		mode->h_active, mode->h_front_porch, mode->h_back_porch,
		mode->h_sync_width);

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-panel-height",
				  &mode->v_active);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-panel-height, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-v-back-porch",
				  &mode->v_back_porch);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-v-back-porch, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-v-front-porch",
				  &mode->v_front_porch);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-v-back-porch, rc=%d\n",
		       rc);
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-v-pulse-width",
				  &mode->v_sync_width);
	if (rc) {
		DSI_ERR("failed to read qcom,mdss-dsi-v-pulse-width, rc=%d\n",
		       rc);
		goto error;
	}
	DSI_DEBUG("panel vert active:%d front_portch:%d back_porch:%d pulse_width:%d\n",
		mode->v_active, mode->v_front_porch, mode->v_back_porch,
		mode->v_sync_width);

error:
	return rc;
}

static int dsi_panel_parse_pixel_format(struct dsi_host_common_cfg *host,
					struct dsi_parser_utils *utils,
					const char *name)
{
	int rc = 0;
	u32 bpp = 0;
	enum dsi_pixel_format fmt;
	const char *packing;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-bpp", &bpp);
	if (rc) {
		DSI_ERR("[%s] failed to read qcom,mdss-dsi-bpp, rc=%d\n",
		       name, rc);
		return rc;
	}

	host->bpp = bpp;

	switch (bpp) {
	case 3:
		fmt = DSI_PIXEL_FORMAT_RGB111;
		break;
	case 8:
		fmt = DSI_PIXEL_FORMAT_RGB332;
		break;
	case 12:
		fmt = DSI_PIXEL_FORMAT_RGB444;
		break;
	case 16:
		fmt = DSI_PIXEL_FORMAT_RGB565;
		break;
	case 18:
		fmt = DSI_PIXEL_FORMAT_RGB666;
		break;
	case 24:
	default:
		fmt = DSI_PIXEL_FORMAT_RGB888;
		break;
	}

	if (fmt == DSI_PIXEL_FORMAT_RGB666) {
		packing = utils->get_property(utils->data,
					  "qcom,mdss-dsi-pixel-packing",
					  NULL);
		if (packing && !strcmp(packing, "loose"))
			fmt = DSI_PIXEL_FORMAT_RGB666_LOOSE;
	}

	host->dst_format = fmt;
	return rc;
}

static int dsi_panel_parse_lane_states(struct dsi_host_common_cfg *host,
				       struct dsi_parser_utils *utils,
				       const char *name)
{
	int rc = 0;
	bool lane_enabled;
	u32 num_of_lanes = 0;

	lane_enabled = utils->read_bool(utils->data,
					    "qcom,mdss-dsi-lane-0-state");
	host->data_lanes |= (lane_enabled ? DSI_DATA_LANE_0 : 0);

	lane_enabled = utils->read_bool(utils->data,
					     "qcom,mdss-dsi-lane-1-state");
	host->data_lanes |= (lane_enabled ? DSI_DATA_LANE_1 : 0);

	lane_enabled = utils->read_bool(utils->data,
					    "qcom,mdss-dsi-lane-2-state");
	host->data_lanes |= (lane_enabled ? DSI_DATA_LANE_2 : 0);

	lane_enabled = utils->read_bool(utils->data,
					     "qcom,mdss-dsi-lane-3-state");
	host->data_lanes |= (lane_enabled ? DSI_DATA_LANE_3 : 0);

	if (host->data_lanes & DSI_DATA_LANE_0)
		num_of_lanes++;
	if (host->data_lanes & DSI_DATA_LANE_1)
		num_of_lanes++;
	if (host->data_lanes & DSI_DATA_LANE_2)
		num_of_lanes++;
	if (host->data_lanes & DSI_DATA_LANE_3)
		num_of_lanes++;

	host->num_data_lanes = num_of_lanes;

	if (host->data_lanes == 0) {
		DSI_ERR("[%s] No data lanes are enabled, rc=%d\n", name, rc);
		rc = -EINVAL;
	}

	return rc;
}

static int dsi_panel_parse_color_swap(struct dsi_host_common_cfg *host,
				      struct dsi_parser_utils *utils,
				      const char *name)
{
	int rc = 0;
	const char *swap_mode;

	swap_mode = utils->get_property(utils->data,
			"qcom,mdss-dsi-color-order", NULL);
	if (swap_mode) {
		if (!strcmp(swap_mode, "rgb_swap_rgb")) {
			host->swap_mode = DSI_COLOR_SWAP_RGB;
		} else if (!strcmp(swap_mode, "rgb_swap_rbg")) {
			host->swap_mode = DSI_COLOR_SWAP_RBG;
		} else if (!strcmp(swap_mode, "rgb_swap_brg")) {
			host->swap_mode = DSI_COLOR_SWAP_BRG;
		} else if (!strcmp(swap_mode, "rgb_swap_grb")) {
			host->swap_mode = DSI_COLOR_SWAP_GRB;
		} else if (!strcmp(swap_mode, "rgb_swap_gbr")) {
			host->swap_mode = DSI_COLOR_SWAP_GBR;
		} else {
			DSI_ERR("[%s] Unrecognized color order-%s\n",
			       name, swap_mode);
			rc = -EINVAL;
		}
	} else {
		DSI_DEBUG("[%s] Falling back to default color order\n", name);
		host->swap_mode = DSI_COLOR_SWAP_RGB;
	}

	/* bit swap on color channel is not defined in dt */
	host->bit_swap_red = false;
	host->bit_swap_green = false;
	host->bit_swap_blue = false;
	return rc;
}

static int dsi_panel_parse_triggers(struct dsi_host_common_cfg *host,
				    struct dsi_parser_utils *utils,
				    const char *name)
{
	const char *trig;
	int rc = 0;

	trig = utils->get_property(utils->data,
			"qcom,mdss-dsi-mdp-trigger", NULL);
	if (trig) {
		if (!strcmp(trig, "none")) {
			host->mdp_cmd_trigger = DSI_TRIGGER_NONE;
		} else if (!strcmp(trig, "trigger_te")) {
			host->mdp_cmd_trigger = DSI_TRIGGER_TE;
		} else if (!strcmp(trig, "trigger_sw")) {
			host->mdp_cmd_trigger = DSI_TRIGGER_SW;
		} else if (!strcmp(trig, "trigger_sw_te")) {
			host->mdp_cmd_trigger = DSI_TRIGGER_SW_TE;
		} else {
			DSI_ERR("[%s] Unrecognized mdp trigger type (%s)\n",
			       name, trig);
			rc = -EINVAL;
		}

	} else {
		DSI_DEBUG("[%s] Falling back to default MDP trigger\n",
			 name);
		host->mdp_cmd_trigger = DSI_TRIGGER_SW;
	}

	trig = utils->get_property(utils->data,
			"qcom,mdss-dsi-dma-trigger", NULL);
	if (trig) {
		if (!strcmp(trig, "none")) {
			host->dma_cmd_trigger = DSI_TRIGGER_NONE;
		} else if (!strcmp(trig, "trigger_te")) {
			host->dma_cmd_trigger = DSI_TRIGGER_TE;
		} else if (!strcmp(trig, "trigger_sw")) {
			host->dma_cmd_trigger = DSI_TRIGGER_SW;
		} else if (!strcmp(trig, "trigger_sw_seof")) {
			host->dma_cmd_trigger = DSI_TRIGGER_SW_SEOF;
		} else if (!strcmp(trig, "trigger_sw_te")) {
			host->dma_cmd_trigger = DSI_TRIGGER_SW_TE;
		} else {
			DSI_ERR("[%s] Unrecognized mdp trigger type (%s)\n",
			       name, trig);
			rc = -EINVAL;
		}

	} else {
		DSI_DEBUG("[%s] Falling back to default MDP trigger\n", name);
		host->dma_cmd_trigger = DSI_TRIGGER_SW;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-te-pin-select",
			&host->te_mode);
	if (rc) {
		DSI_WARN("[%s] fallback to default te-pin-select\n", name);
		host->te_mode = 1;
		rc = 0;
	}

	return rc;
}

static int dsi_panel_parse_misc_host_config(struct dsi_host_common_cfg *host,
					    struct dsi_parser_utils *utils,
					    const char *name)
{
	u32 val = 0, line_no = 0, window = 0;
	int rc = 0;
	bool panel_cphy_mode = false;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-t-clk-post", &val);
	if (!rc) {
		host->t_clk_post = val;
		DSI_DEBUG("[%s] t_clk_post = %d\n", name, val);
	}

	val = 0;
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-t-clk-pre", &val);
	if (!rc) {
		host->t_clk_pre = val;
		DSI_DEBUG("[%s] t_clk_pre = %d\n", name, val);
	}

	host->ignore_rx_eot = utils->read_bool(utils->data,
						"qcom,mdss-dsi-rx-eot-ignore");

	host->append_tx_eot = utils->read_bool(utils->data,
						"qcom,mdss-dsi-tx-eot-append");

	host->ext_bridge_mode = utils->read_bool(utils->data,
					"qcom,mdss-dsi-ext-bridge-mode");

	host->force_hs_clk_lane = utils->read_bool(utils->data,
					"qcom,mdss-dsi-force-clock-lane-hs");
	panel_cphy_mode = utils->read_bool(utils->data,
					"qcom,panel-cphy-mode");
	host->phy_type = panel_cphy_mode ? DSI_PHY_TYPE_CPHY
						: DSI_PHY_TYPE_DPHY;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-dma-schedule-line",
				  &line_no);
	if (rc)
		host->dma_sched_line = 0;
	else
		host->dma_sched_line = line_no;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-dma-schedule-window",
				  &window);
	if (rc)
		host->dma_sched_window = 0;
	else
		host->dma_sched_window = window;

	DSI_DEBUG("[%s] DMA scheduling parameters Line: %d Window: %d\n", name,
			host->dma_sched_line, host->dma_sched_window);
	return 0;
}

static void dsi_panel_parse_split_link_config(struct dsi_host_common_cfg *host,
					struct dsi_parser_utils *utils,
					const char *name)
{
	int rc = 0;
	u32 val = 0;
	bool supported = false;
	struct dsi_split_link_config *split_link = &host->split_link;

	supported = utils->read_bool(utils->data, "qcom,split-link-enabled");

	if (!supported) {
		DSI_DEBUG("[%s] Split link is not supported\n", name);
		split_link->split_link_enabled = false;
		return;
	}

	rc = utils->read_u32(utils->data, "qcom,sublinks-count", &val);
	if (rc || val < 1) {
		DSI_DEBUG("[%s] Using default sublinks count\n", name);
		split_link->num_sublinks = 2;
	} else {
		split_link->num_sublinks = val;
	}

	rc = utils->read_u32(utils->data, "qcom,lanes-per-sublink", &val);
	if (rc || val < 1) {
		DSI_DEBUG("[%s] Using default lanes per sublink\n", name);
		split_link->lanes_per_sublink = 2;
	} else {
		split_link->lanes_per_sublink = val;
	}

	DSI_DEBUG("[%s] Split link is supported %d-%d\n", name,
		split_link->num_sublinks, split_link->lanes_per_sublink);
	split_link->split_link_enabled = true;
}

static int dsi_panel_parse_host_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_parser_utils *utils = &panel->utils;

	rc = dsi_panel_parse_pixel_format(&panel->host_config, utils,
					  panel->name);
	if (rc) {
		DSI_ERR("[%s] failed to get pixel format, rc=%d\n",
		panel->name, rc);
		goto error;
	}

	rc = dsi_panel_parse_lane_states(&panel->host_config, utils,
					 panel->name);
	if (rc) {
		DSI_ERR("[%s] failed to parse lane states, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

	rc = dsi_panel_parse_color_swap(&panel->host_config, utils,
					panel->name);
	if (rc) {
		DSI_ERR("[%s] failed to parse color swap config, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

	rc = dsi_panel_parse_triggers(&panel->host_config, utils,
				      panel->name);
	if (rc) {
		DSI_ERR("[%s] failed to parse triggers, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

	rc = dsi_panel_parse_misc_host_config(&panel->host_config, utils,
					      panel->name);
	if (rc) {
		DSI_ERR("[%s] failed to parse misc host config, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

	dsi_panel_parse_split_link_config(&panel->host_config, utils,
						panel->name);

error:
	return rc;
}

static int dsi_panel_parse_qsync_caps(struct dsi_panel *panel,
				     struct device_node *of_node)
{
	int rc = 0;
	u32 val = 0, i;
	struct dsi_qsync_capabilities *qsync_caps = &panel->qsync_caps;
	struct dsi_parser_utils *utils = &panel->utils;
	const char *name = panel->name;

	/**
	 * "mdss-dsi-qsync-min-refresh-rate" is defined in cmd mode and
	 *  video mode when there is only one qsync min fps present.
	 */
	rc = of_property_read_u32(of_node,
				  "qcom,mdss-dsi-qsync-min-refresh-rate",
				  &val);
	if (rc)
		DSI_DEBUG("[%s] qsync min fps not defined rc:%d\n",
			panel->name, rc);

	qsync_caps->qsync_min_fps = val;

	/**
	 * "dsi-supported-qsync-min-fps-list" may be defined in video
	 *  mode, only in dfps case when "qcom,dsi-supported-dfps-list"
	 *  is defined.
	 */
	qsync_caps->qsync_min_fps_list_len = utils->count_u32_elems(utils->data,
				  "qcom,dsi-supported-qsync-min-fps-list");
	if (qsync_caps->qsync_min_fps_list_len < 1)
		goto qsync_support;

	/**
	 * qcom,dsi-supported-qsync-min-fps-list cannot be defined
	 *  along with qcom,mdss-dsi-qsync-min-refresh-rate.
	 */
	if (qsync_caps->qsync_min_fps_list_len >= 1 &&
		qsync_caps->qsync_min_fps) {
		DSI_ERR("[%s] Both qsync nodes are defined\n",
				name);
		rc = -EINVAL;
		goto error;
	}

	if (panel->dfps_caps.dfps_list_len !=
			qsync_caps->qsync_min_fps_list_len) {
		DSI_ERR("[%s] Qsync min fps list mismatch with dfps\n", name);
		rc = -EINVAL;
		goto error;
	}

	qsync_caps->qsync_min_fps_list =
		kcalloc(qsync_caps->qsync_min_fps_list_len, sizeof(u32),
			GFP_KERNEL);
	if (!qsync_caps->qsync_min_fps_list) {
		rc = -ENOMEM;
		goto error;
	}

	rc = utils->read_u32_array(utils->data,
			"qcom,dsi-supported-qsync-min-fps-list",
			qsync_caps->qsync_min_fps_list,
			qsync_caps->qsync_min_fps_list_len);
	if (rc) {
		DSI_ERR("[%s] Qsync min fps list parse failed\n", name);
		rc = -EINVAL;
		goto error;
	}

	qsync_caps->qsync_min_fps = qsync_caps->qsync_min_fps_list[0];

	for (i = 1; i < qsync_caps->qsync_min_fps_list_len; i++) {
		if (qsync_caps->qsync_min_fps_list[i] <
				qsync_caps->qsync_min_fps)
			qsync_caps->qsync_min_fps =
				qsync_caps->qsync_min_fps_list[i];
	}

qsync_support:
	/* allow qsync support only if DFPS is with VFP approach */
	if ((panel->dfps_caps.dfps_support) &&
	    !(panel->dfps_caps.type == DSI_DFPS_IMMEDIATE_VFP))
		panel->qsync_caps.qsync_min_fps = 0;

error:
	if (rc < 0) {
		qsync_caps->qsync_min_fps = 0;
		qsync_caps->qsync_min_fps_list_len = 0;
	}
	return rc;
}

static int dsi_panel_parse_dyn_clk_caps(struct dsi_panel *panel)
{
	int rc = 0;
	bool supported = false;
	struct dsi_dyn_clk_caps *dyn_clk_caps = &panel->dyn_clk_caps;
	struct dsi_parser_utils *utils = &panel->utils;
	const char *name = panel->name;
	const char *type;

	supported = utils->read_bool(utils->data, "qcom,dsi-dyn-clk-enable");

	if (!supported) {
		dyn_clk_caps->dyn_clk_support = false;
		return rc;
	}

	dyn_clk_caps->bit_clk_list_len = utils->count_u32_elems(utils->data,
			"qcom,dsi-dyn-clk-list");

	if (dyn_clk_caps->bit_clk_list_len < 1) {
		DSI_ERR("[%s] failed to get supported bit clk list\n", name);
		return -EINVAL;
	}

	dyn_clk_caps->bit_clk_list = kcalloc(dyn_clk_caps->bit_clk_list_len,
			sizeof(u32), GFP_KERNEL);
	if (!dyn_clk_caps->bit_clk_list)
		return -ENOMEM;

	rc = utils->read_u32_array(utils->data, "qcom,dsi-dyn-clk-list",
			dyn_clk_caps->bit_clk_list,
			dyn_clk_caps->bit_clk_list_len);

	if (rc) {
		DSI_ERR("[%s] failed to parse supported bit clk list\n", name);
		return -EINVAL;
	}

	dyn_clk_caps->dyn_clk_support = true;

	type = utils->get_property(utils->data,
		"qcom,dsi-dyn-clk-type", NULL);
	if (!type) {
		dyn_clk_caps->type = DSI_DYN_CLK_TYPE_LEGACY;
		dyn_clk_caps->maintain_const_fps = false;
		return 0;
	}
	if (!strcmp(type, "constant-fps-adjust-hfp")) {
		dyn_clk_caps->type = DSI_DYN_CLK_TYPE_CONST_FPS_ADJUST_HFP;
		dyn_clk_caps->maintain_const_fps = true;
	} else if (!strcmp(type, "constant-fps-adjust-vfp")) {
		dyn_clk_caps->type = DSI_DYN_CLK_TYPE_CONST_FPS_ADJUST_VFP;
		dyn_clk_caps->maintain_const_fps = true;
	} else {
		dyn_clk_caps->type = DSI_DYN_CLK_TYPE_LEGACY;
		dyn_clk_caps->maintain_const_fps = false;
	}
	DSI_DEBUG("Dynamic clock type is [%s]\n", type);
	return 0;
}

static int dsi_panel_parse_dfps_caps(struct dsi_panel *panel)
{
	int rc = 0;
	bool supported = false;
	struct dsi_dfps_capabilities *dfps_caps = &panel->dfps_caps;
	struct dsi_parser_utils *utils = &panel->utils;
	const char *name = panel->name;
	const char *type;
	u32 i;

	supported = utils->read_bool(utils->data,
			"qcom,mdss-dsi-pan-enable-dynamic-fps");

	if (!supported) {
		DSI_DEBUG("[%s] DFPS is not supported\n", name);
		dfps_caps->dfps_support = false;
		return rc;
	}

	type = utils->get_property(utils->data,
			"qcom,mdss-dsi-pan-fps-update", NULL);
	if (!type) {
		DSI_ERR("[%s] dfps type not defined\n", name);
		rc = -EINVAL;
		goto error;
	} else if (!strcmp(type, "dfps_suspend_resume_mode")) {
		dfps_caps->type = DSI_DFPS_SUSPEND_RESUME;
	} else if (!strcmp(type, "dfps_immediate_clk_mode")) {
		dfps_caps->type = DSI_DFPS_IMMEDIATE_CLK;
	} else if (!strcmp(type, "dfps_immediate_porch_mode_hfp")) {
		dfps_caps->type = DSI_DFPS_IMMEDIATE_HFP;
	} else if (!strcmp(type, "dfps_immediate_porch_mode_vfp")) {
		dfps_caps->type = DSI_DFPS_IMMEDIATE_VFP;
	} else {
		DSI_ERR("[%s] dfps type is not recognized\n", name);
		rc = -EINVAL;
		goto error;
	}

	dfps_caps->dfps_list_len = utils->count_u32_elems(utils->data,
				  "qcom,dsi-supported-dfps-list");
	if (dfps_caps->dfps_list_len < 1) {
		DSI_ERR("[%s] dfps refresh list not present\n", name);
		rc = -EINVAL;
		goto error;
	}

	dfps_caps->dfps_list = kcalloc(dfps_caps->dfps_list_len, sizeof(u32),
			GFP_KERNEL);
	if (!dfps_caps->dfps_list) {
		rc = -ENOMEM;
		goto error;
	}

	rc = utils->read_u32_array(utils->data,
			"qcom,dsi-supported-dfps-list",
			dfps_caps->dfps_list,
			dfps_caps->dfps_list_len);
	if (rc) {
		DSI_ERR("[%s] dfps refresh rate list parse failed\n", name);
		rc = -EINVAL;
		goto error;
	}
	dfps_caps->dfps_support = true;

	/* calculate max and min fps */
	dfps_caps->max_refresh_rate = dfps_caps->dfps_list[0];
	dfps_caps->min_refresh_rate = dfps_caps->dfps_list[0];

	for (i = 1; i < dfps_caps->dfps_list_len; i++) {
		if (dfps_caps->dfps_list[i] < dfps_caps->min_refresh_rate)
			dfps_caps->min_refresh_rate = dfps_caps->dfps_list[i];
		else if (dfps_caps->dfps_list[i] > dfps_caps->max_refresh_rate)
			dfps_caps->max_refresh_rate = dfps_caps->dfps_list[i];
	}

error:
	return rc;
}

static int dsi_panel_parse_video_host_config(struct dsi_video_engine_cfg *cfg,
					     struct dsi_parser_utils *utils,
					     const char *name)
{
	int rc = 0;
	const char *traffic_mode;
	u32 vc_id = 0;
	u32 val = 0;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-h-sync-pulse", &val);
	if (rc) {
		DSI_DEBUG("[%s] fallback to default h-sync-pulse\n", name);
		cfg->pulse_mode_hsa_he = false;
	} else if (val == 1) {
		cfg->pulse_mode_hsa_he = true;
	} else if (val == 0) {
		cfg->pulse_mode_hsa_he = false;
	} else {
		DSI_ERR("[%s] Unrecognized value for mdss-dsi-h-sync-pulse\n",
		       name);
		rc = -EINVAL;
		goto error;
	}

	cfg->hfp_lp11_en = utils->read_bool(utils->data,
						"qcom,mdss-dsi-hfp-power-mode");

	cfg->hbp_lp11_en = utils->read_bool(utils->data,
						"qcom,mdss-dsi-hbp-power-mode");

	cfg->hsa_lp11_en = utils->read_bool(utils->data,
						"qcom,mdss-dsi-hsa-power-mode");

	cfg->last_line_interleave_en = utils->read_bool(utils->data,
					"qcom,mdss-dsi-last-line-interleave");

	cfg->eof_bllp_lp11_en = utils->read_bool(utils->data,
					"qcom,mdss-dsi-bllp-eof-power-mode");

	cfg->bllp_lp11_en = utils->read_bool(utils->data,
					"qcom,mdss-dsi-bllp-power-mode");

	traffic_mode = utils->get_property(utils->data,
				       "qcom,mdss-dsi-traffic-mode",
				       NULL);
	if (!traffic_mode) {
		DSI_DEBUG("[%s] Falling back to default traffic mode\n", name);
		cfg->traffic_mode = DSI_VIDEO_TRAFFIC_SYNC_PULSES;
	} else if (!strcmp(traffic_mode, "non_burst_sync_pulse")) {
		cfg->traffic_mode = DSI_VIDEO_TRAFFIC_SYNC_PULSES;
	} else if (!strcmp(traffic_mode, "non_burst_sync_event")) {
		cfg->traffic_mode = DSI_VIDEO_TRAFFIC_SYNC_START_EVENTS;
	} else if (!strcmp(traffic_mode, "burst_mode")) {
		cfg->traffic_mode = DSI_VIDEO_TRAFFIC_BURST_MODE;
	} else {
		DSI_ERR("[%s] Unrecognized traffic mode-%s\n", name,
		       traffic_mode);
		rc = -EINVAL;
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-virtual-channel-id",
				  &vc_id);
	if (rc) {
		DSI_DEBUG("[%s] Fallback to default vc id\n", name);
		cfg->vc_id = 0;
	} else {
		cfg->vc_id = vc_id;
	}

error:
	return rc;
}

static int dsi_panel_parse_cmd_host_config(struct dsi_cmd_engine_cfg *cfg,
					   struct dsi_parser_utils *utils,
					   const char *name)
{
	u32 val = 0;
	int rc = 0;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-wr-mem-start", &val);
	if (rc) {
		DSI_DEBUG("[%s] Fallback to default wr-mem-start\n", name);
		cfg->wr_mem_start = 0x2C;
	} else {
		cfg->wr_mem_start = val;
	}

	val = 0;
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-wr-mem-continue",
				  &val);
	if (rc) {
		DSI_DEBUG("[%s] Fallback to default wr-mem-continue\n", name);
		cfg->wr_mem_continue = 0x3C;
	} else {
		cfg->wr_mem_continue = val;
	}

	/* TODO:  fix following */
	cfg->max_cmd_packets_interleave = 0;

	val = 0;
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-te-dcs-command",
				  &val);
	if (rc) {
		DSI_DEBUG("[%s] fallback to default te-dcs-cmd\n", name);
		cfg->insert_dcs_command = true;
	} else if (val == 1) {
		cfg->insert_dcs_command = true;
	} else if (val == 0) {
		cfg->insert_dcs_command = false;
	} else {
		DSI_ERR("[%s] Unrecognized value for mdss-dsi-te-dcs-command\n",
		       name);
		rc = -EINVAL;
		goto error;
	}

error:
	return rc;
}

static int dsi_panel_parse_panel_mode(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_parser_utils *utils = &panel->utils;
	bool panel_mode_switch_enabled;
	enum dsi_op_mode panel_mode;
	const char *mode;

	mode = utils->get_property(utils->data,
			"qcom,mdss-dsi-panel-type", NULL);
	if (!mode) {
		DSI_DEBUG("[%s] Fallback to default panel mode\n", panel->name);
		panel_mode = DSI_OP_VIDEO_MODE;
	} else if (!strcmp(mode, "dsi_video_mode")) {
		panel_mode = DSI_OP_VIDEO_MODE;
	} else if (!strcmp(mode, "dsi_cmd_mode")) {
		panel_mode = DSI_OP_CMD_MODE;
	} else {
		DSI_ERR("[%s] Unrecognized panel type-%s\n", panel->name, mode);
		rc = -EINVAL;
		goto error;
	}

	panel_mode_switch_enabled = utils->read_bool(utils->data,
					"qcom,mdss-dsi-panel-mode-switch");

	DSI_DEBUG("%s: panel operating mode switch feature %s\n", __func__,
		(panel_mode_switch_enabled ? "enabled" : "disabled"));

	if (panel_mode == DSI_OP_VIDEO_MODE || panel_mode_switch_enabled) {
		rc = dsi_panel_parse_video_host_config(&panel->video_config,
						       utils,
						       panel->name);
		if (rc) {
			DSI_ERR("[%s] Failed to parse video host cfg, rc=%d\n",
			       panel->name, rc);
			goto error;
		}
	}

	if (panel_mode == DSI_OP_CMD_MODE || panel_mode_switch_enabled) {
		rc = dsi_panel_parse_cmd_host_config(&panel->cmd_config,
						     utils,
						     panel->name);
		if (rc) {
			DSI_ERR("[%s] Failed to parse cmd host config, rc=%d\n",
			       panel->name, rc);
			goto error;
		}
	}

	panel->poms_align_vsync = utils->read_bool(utils->data,
					"qcom,poms-align-panel-vsync");
	panel->panel_mode = panel_mode;
	panel->panel_mode_switch_enabled = panel_mode_switch_enabled;
error:
	return rc;
}

static int dsi_panel_parse_phy_props(struct dsi_panel *panel)
{
	int rc = 0;
	u32 val = 0;
	const char *str;
	struct dsi_panel_phy_props *props = &panel->phy_props;
	struct dsi_parser_utils *utils = &panel->utils;
	const char *name = panel->name;

	rc = utils->read_u32(utils->data,
		  "qcom,mdss-pan-physical-width-dimension", &val);
	if (rc) {
		DSI_DEBUG("[%s] Physical panel width is not defined\n", name);
		props->panel_width_mm = 0;
		rc = 0;
	} else {
		props->panel_width_mm = val;
	}

	rc = utils->read_u32(utils->data,
				  "qcom,mdss-pan-physical-height-dimension",
				  &val);
	if (rc) {
		DSI_DEBUG("[%s] Physical panel height is not defined\n", name);
		props->panel_height_mm = 0;
		rc = 0;
	} else {
		props->panel_height_mm = val;
	}

	str = utils->get_property(utils->data,
			"qcom,mdss-dsi-panel-orientation", NULL);
	if (!str) {
		props->rotation = DSI_PANEL_ROTATE_NONE;
	} else if (!strcmp(str, "180")) {
		props->rotation = DSI_PANEL_ROTATE_HV_FLIP;
	} else if (!strcmp(str, "hflip")) {
		props->rotation = DSI_PANEL_ROTATE_H_FLIP;
	} else if (!strcmp(str, "vflip")) {
		props->rotation = DSI_PANEL_ROTATE_V_FLIP;
	} else {
		DSI_ERR("[%s] Unrecognized panel rotation-%s\n", name, str);
		rc = -EINVAL;
		goto error;
	}
error:
	return rc;
}
const char *cmd_set_prop_map[DSI_CMD_SET_MAX] = {
	"qcom,mdss-dsi-pre-on-command",
	"qcom,mdss-dsi-on-command",
	"qcom,mdss-dsi-post-panel-on-command",
	"qcom,mdss-dsi-pre-off-command",
	"qcom,mdss-dsi-off-command",
	"qcom,mdss-dsi-post-off-command",
	"qcom,mdss-dsi-pre-res-switch",
	"qcom,mdss-dsi-res-switch",
	"qcom,mdss-dsi-post-res-switch",
	"qcom,cmd-to-video-mode-switch-commands",
	"qcom,cmd-to-video-mode-post-switch-commands",
	"qcom,video-to-cmd-mode-switch-commands",
	"qcom,video-to-cmd-mode-post-switch-commands",
	"qcom,mdss-dsi-panel-status-command",
	"qcom,mdss-dsi-lp1-command",
	"qcom,mdss-dsi-lp2-command",
	"qcom,mdss-dsi-nolp-command",
	"PPS not parsed from DTSI, generated dynamically",
	"ROI not parsed from DTSI, generated dynamically",
	"qcom,mdss-dsi-timing-switch-command",
	"qcom,mdss-dsi-post-mode-switch-on-command",
	"qcom,mdss-dsi-qsync-on-commands",
	"qcom,mdss-dsi-qsync-off-commands",
	"qcom,mdss-dsi-panel-hbm-brightness-on-command",
	"qcom,mdss-dsi-panel-hbm-brightness-off-command",
	"qcom,mdss-dsi-panel-seed-lp-on-command-0",
	"qcom,mdss-dsi-panel-seed-lp-on-command-1",
	"qcom,mdss-dsi-panel-seed-lp-on-command-2",
	"qcom,mdss-dsi-panel-seed-lp-off-command",
	"qcom,mdss-dsi-panel-hbm-on-command-1",
	"qcom,mdss-dsi-panel-hbm-on-command-2",
	"qcom,mdss-dsi-panel-hbm-on-command-3",
	"qcom,mdss-dsi-panel-hbm-on-command-4",
	"qcom,mdss-dsi-panel-hbm-on-command-5",
	"qcom,mdss-dsi-panel-hbm-off-command",
	"qcom,mdss-dsi-panel-serial-num-command",
	"qcom,mdss-dsi-panel-msd-aod-on",
	"qcom,mdss-dsi-panel-msd-aod-off",
	"qcom,mdss-dsi-panel-aod-on-command-1",
	"qcom,mdss-dsi-panel-aod-on-command-1-o",
	"qcom,mdss-dsi-panel-aod-on-command-2",
	"qcom,mdss-dsi-panel-aod-on-command-3",
	"qcom,mdss-dsi-panel-aod-on-command-3-o",
	"qcom,mdss-dsi-panel-aod-on-command-4",
	"qcom,mdss-dsi-panel-aod-on-command-5",
	"qcom,mdss-dsi-panel-aod-on-command-5-o",
	"qcom,mdss-dsi-panel-aod-off-command",
	"qcom,mdss-dsi-panel-aod-off-command-o1",
	"qcom,mdss-dsi-panel-aod-off-command-o2",
	"qcom,mdss-dsi-panel-aod-off-hbm-on-command",
	"qcom,mdss-dsi-panel-real-aod-off-hbm-on-command",
	"qcom,mdss-dsi-panel-real-aod-off-hbm-on-command-o1",
	"qcom,mdss-dsi-panel-real-aod-off-hbm-on-command-o2",
	"qcom,mdss-dsi-panel-hbm-off-aod-on-command",
	"qcom,mdss-dsi-panel-dci-p3-on-command",
	"qcom,mdss-dsi-panel-dci-p3-off-command",
	"qcom,mdss-dsi-panel-night-mode-on-command",
	"qcom,mdss-dsi-panel-night-mode-off-command",
	"qcom,mdss-dsi-panel-id-command",
	"qcom,mdss-dsi-panel-read-register-open-command",
	"qcom,mdss-dsi-panel-read-register-ed-command",
	"qcom,mdss-dsi-panel-id1-command",
	"qcom,mdss-dsi-panel-id2-command",
	"qcom,mdss-dsi-panel-id3-command",
	"qcom,mdss-dsi-panel-id4-command",
	"qcom,mdss-dsi-panel-id5-command",
	"qcom,mdss-dsi-panel-id6-command",
	"qcom,mdss-dsi-panel-id7-command",
	"qcom,mdss-dsi-panel-check-info-command",
	"qcom,mdss-dsi-panel-read-register-close-command",
	"qcom,mdss-dsi-acl-command",
	"qcom,mdss-dsi-panel-read-ic_v-register-open-command",
	"qcom,mdss-dsi-panel-ic_v-command",
	"qcom,mdss-dsi-panel-read-ic_v-register-close-command",
	"qcom,mdss-dsi-panel-serial-num-pre-command",
	"qcom,mdss-dsi-panel-serial-num-post-command",
	"qcom,mdss-dsi-panel-code-info-command",
	"qcom,mdss-dsi-panel-stage-info-command",
	"qcom,mdss-dsi-panel-production-info-command",
	"qcom,mdss-dsi-panel-read-esd-registed-longread-command",
	"qcom,mdss-dsi-panel-gamma-flash-pre-read-1-command",
	"qcom,mdss-dsi-panel-gamma-flash-pre-read-2-command",
	"qcom,mdss-dsi-panel-gamma-flash-read-fb-command",
	"qcom,mdss-dsi-panel-level2-key-enable-command",
	"qcom,mdss-dsi-panel-gamma-otp-read-c8-smrps-command",
	"qcom,mdss-dsi-panel-gamma-otp-read-c8-command",
	"qcom,mdss-dsi-panel-gamma-otp-read-c9-smrps-command",
	"qcom,mdss-dsi-panel-gamma-otp-read-c9-command",
	"qcom,mdss-dsi-panel-gamma-otp-read-b3-smrps-command",
	"qcom,mdss-dsi-panel-gamma-otp-read-b3-command",
	"qcom,mdss-dsi-panel-level2-key-disable-command",
	"qcom,mdss-dsi-panel-display-p3-mode-on-command",
	"qcom,mdss-dsi-panel-display-p3-mode-off-command",
	"qcom,mdss-dsi-panel-display-wide-color-mode-on-command",
	"qcom,mdss-dsi-panel-display-wide-color-mode-off-command",
	"qcom,mdss-dsi-panel-display-srgb-color-mode-on-command",
	"qcom,mdss-dsi-panel-display-srgb-color-mode-off-command",
	"qcom,mdss-113mhz-osc-dsi-on-command",
	"qcom,mdss-dsi-post-on-backlight",
	"qcom,mdss-dsi-customer-srgb-enable-command",
	"qcom,mdss-dsi-customer-srgb-disable-command",
	"qcom,mdss-dsi-customer-p3-enable-command",
	"qcom,mdss-dsi-customer-p3-disable-command",
	"qcom,mdss-dsi-panel-command",
	"qcom,mdss-dsi-seed-command",
	"qcom,mdss-dsi-opec-command",
	"qcom,mdss-dsi-opec-command-o",
	"qcom,mdss-dsi-loading-effect-1-command",
	"qcom,mdss-dsi-loading-effect-1-command-o",
	"qcom,mdss-dsi-loading-effect-2-command",
	"qcom,mdss-dsi-loading-effect-2-command-o",
	"qcom,mdss-dsi-loading-effect-off-command",
	"qcom,mdss-dsi-loading-effect-off-command-o",
	"qcom,mdss-dsi-panel-gamma-change-write-command",
	"qcom,mdss-dsi-panel-register-read-command",
#if defined(CONFIG_PXLW_IRIS)
	"iris,abyp-panel-command",
#endif
#ifdef CONFIG_PXLW_IRIS
	"qcom,mdss-dsi-qsync-min-fps-0-command",
	"qcom,mdss-dsi-qsync-min-fps-1-command",
	"qcom,mdss-dsi-qsync-min-fps-2-command",
	"qcom,mdss-dsi-qsync-min-fps-3-command",
	"qcom,mdss-dsi-qsync-min-fps-4-command",
	"qcom,mdss-dsi-qsync-min-fps-5-command",
	"qcom,mdss-dsi-qsync-min-fps-6-command",
	"qcom,mdss-dsi-qsync-min-fps-7-command",
	"qcom,mdss-dsi-qsync-min-fps-8-command",
	"qcom,mdss-dsi-qsync-min-fps-9-command",
	"qcom,mdss-dsi-fakeframe-command",
	"qcom,mdss-dsi-adfr-pre-switch-command",
#endif
};

const char *cmd_set_state_map[DSI_CMD_SET_MAX] = {
	"qcom,mdss-dsi-pre-on-command-state",
	"qcom,mdss-dsi-on-command-state",
	"qcom,mdss-dsi-post-on-command-state",
	"qcom,mdss-dsi-pre-off-command-state",
	"qcom,mdss-dsi-off-command-state",
	"qcom,mdss-dsi-post-off-command-state",
	"qcom,mdss-dsi-pre-res-switch-state",
	"qcom,mdss-dsi-res-switch-state",
	"qcom,mdss-dsi-post-res-switch-state",
	"qcom,cmd-to-video-mode-switch-commands-state",
	"qcom,cmd-to-video-mode-post-switch-commands-state",
	"qcom,video-to-cmd-mode-switch-commands-state",
	"qcom,video-to-cmd-mode-post-switch-commands-state",
	"qcom,mdss-dsi-panel-status-command-state",
	"qcom,mdss-dsi-lp1-command-state",
	"qcom,mdss-dsi-lp2-command-state",
	"qcom,mdss-dsi-nolp-command-state",
	"PPS not parsed from DTSI, generated dynamically",
	"ROI not parsed from DTSI, generated dynamically",
	"qcom,mdss-dsi-timing-switch-command-state",
	"qcom,mdss-dsi-post-mode-switch-on-command-state",
	"qcom,mdss-dsi-qsync-on-commands-state",
	"qcom,mdss-dsi-qsync-off-commands-state",
	"qcom,mdss-dsi-panel-hbm-brightness-on-command-state",
	"qcom,mdss-dsi-panel-hbm-brightness-off-command-state",
	"qcom,mdss-dsi-panel-seed-lp-on-command-0-state",
	"qcom,mdss-dsi-panel-seed-lp-on-command-1-state",
	"qcom,mdss-dsi-panel-seed-lp-on-command-2-state",
	"qcom,mdss-dsi-panel-seed-lp-off-command-state",
	"qcom,mdss-dsi-panel-hbm-on-command-1-state",
	"qcom,mdss-dsi-panel-hbm-on-command-2-state",
	"qcom,mdss-dsi-panel-hbm-on-command-3-state",
	"qcom,mdss-dsi-panel-hbm-on-command-4-state",
	"qcom,mdss-dsi-panel-hbm-on-command-5-state",
	"qcom,mdss-dsi-panel-hbm-off-command-state",
	"qcom,mdss-dsi-panel-serial-num-command-state",
	"qcom,mdss-dsi-panel-msd-aod-on-command-state",
	"qcom,mdss-dsi-panel-msd-aod-off-command-state",
	"qcom,mdss-dsi-panel-aod-on-command-1-state",
	"qcom,mdss-dsi-panel-aod-on-command-1-o-state",
	"qcom,mdss-dsi-panel-aod-on-command-2-state",
	"qcom,mdss-dsi-panel-aod-on-command-3-state",
	"qcom,mdss-dsi-panel-aod-on-command-3-o-state",
	"qcom,mdss-dsi-panel-aod-on-command-4-state",
	"qcom,mdss-dsi-panel-aod-on-command-5-state",
	"qcom,mdss-dsi-panel-aod-on-command-5-o-state",
	"qcom,mdss-dsi-panel-aod-off-command-state",
	"qcom,mdss-dsi-panel-aod-off-command-o1-state",
	"qcom,mdss-dsi-panel-aod-off-command-o2-state",
	"qcom,mdss-dsi-panel-aod-off-hbm-on-command-state",
	"qcom,mdss-dsi-panel-real-aod-off-hbm-on-command-state",
	"qcom,mdss-dsi-panel-real-aod-off-hbm-on-command-o1-state",
	"qcom,mdss-dsi-panel-real-aod-off-hbm-on-command-o2-state",
	"qcom,mdss-dsi-panel-hbm-off-aod-on-command-state",
	"qcom,mdss-dsi-panel-dci-p3-on-command-state",
	"qcom,mdss-dsi-panel-dci-p3-off-command-state",
	"qcom,mdss-dsi-panel-night-mode-on-command-state",
	"qcom,mdss-dsi-panel-night-mode-off-command-state",
	"qcom,mdss-dsi-panel-id-command-state",
	"qcom,mdss-dsi-panel-read-register-open-command-state",
	"qcom,mdss-dsi-panel-read-register-ed-command-state",
	"qcom,mdss-dsi-panel-id1-command-state",
	"qcom,mdss-dsi-panel-id2-command-state",
	"qcom,mdss-dsi-panel-id3-command-state",
	"qcom,mdss-dsi-panel-id4-command-state",
	"qcom,mdss-dsi-panel-id5-command-state",
	"qcom,mdss-dsi-panel-id6-command-state",
	"qcom,mdss-dsi-panel-id7-command-state",
	"qcom,mdss-dsi-panel-check-info-command-state",
	"qcom,mdss-dsi-panel-read-register-close-command-state",
	"qcom,mdss-dsi-acl-command-state",
	"qcom,mdss-dsi-panel-read-ic_v-register-open-command-state",
	"qcom,mdss-dsi-panel-ic_v-command-state",
	"qcom,mdss-dsi-panel-read-ic_v-register-close-command-state",
	"qcom,mdss-dsi-panel-serial-num-pre-command-state",
	"qcom,mdss-dsi-panel-serial-num-post-command-state",
	"qcom,mdss-dsi-panel-code-info-command-state",
	"qcom,mdss-dsi-panel-stage-info-command-state",
	"qcom,mdss-dsi-panel-production-info-command-state",
	"qcom,mdss-dsi-panel-read-esd-registed-longread-command-state",
	"qcom,mdss-dsi-panel-gamma-flash-pre-read-1-command-state",
	"qcom,mdss-dsi-panel-gamma-flash-pre-read-2-command-state",
	"qcom,mdss-dsi-panel-gamma-flash-read-fb-command-state",
	"qcom,mdss-dsi-panel-level2-key-enable-command-state",
	"qcom,mdss-dsi-panel-gamma-otp-read-c8-smrps-command-state",
	"qcom,mdss-dsi-panel-gamma-otp-read-c8-command-state",
	"qcom,mdss-dsi-panel-gamma-otp-read-c9-smrps-command-state",
	"qcom,mdss-dsi-panel-gamma-otp-read-c9-command-state",
	"qcom,mdss-dsi-panel-gamma-otp-read-b3-smrps-command-state",
	"qcom,mdss-dsi-panel-gamma-otp-read-b3-command-state",
	"qcom,mdss-dsi-panel-level2-key-disable-command-state",
	"qcom,mdss-dsi-panel-display-p3-mode-on-command-state",
	"qcom,mdss-dsi-panel-display-p3-mode-off-command-state",
	"qcom,mdss-dsi-panel-display-wide-color-mode-on-command-state",
	"qcom,mdss-dsi-panel-display-wide-color-mode-off-command-state",
	"qcom,mdss-dsi-panel-display-srgb-color-mode-on-command-state",
	"qcom,mdss-dsi-panel-display-srgb-color-mode-off-command-state",
	"qcom,mdss-113mhz-osc-dsi-on-command-state",
	"qcom,mdss-dsi-post-on-backlight-state",
	"qcom,mdss-dsi-customer-srgb-enable-command-state",
	"qcom,mdss-dsi-customer-srgb-disable-command-state",
	"qcom,mdss-dsi-customer-p3-enable-command-state",
	"qcom,mdss-dsi-customer-p3-disable-command-state",
	"qcom,mdss-dsi-panel-command-state",
	"qcom,mdss-dsi-seed-command-state",
	"qcom,mdss-dsi-opec-command-state",
	"qcom,mdss-dsi-opec-command-o-state",
	"qcom,mdss-dsi-loading-effect-1-command-state",
	"qcom,mdss-dsi-loading-effect-1-command-o-state",
	"qcom,mdss-dsi-loading-effect-2-command-state",
	"qcom,mdss-dsi-loading-effect-2-command-o-state",
	"qcom,mdss-dsi-loading-effect-off-command-state",
	"qcom,mdss-dsi-loading-effect-off-command-o-state",
	"qcom,mdss-dsi-panel-gamma-change-write-command-state",
	"qcom,mdss-dsi-panel-register-read-command-state",
#if defined(CONFIG_PXLW_IRIS)
	"iris,abyp-panel-command-state",
#endif
#ifdef CONFIG_PXLW_IRIS
	"qcom,mdss-dsi-qsync-min-fps-0-command-state",
	"qcom,mdss-dsi-qsync-min-fps-1-command-state",
	"qcom,mdss-dsi-qsync-min-fps-2-command-state",
	"qcom,mdss-dsi-qsync-min-fps-3-command-state",
	"qcom,mdss-dsi-qsync-min-fps-4-command-state",
	"qcom,mdss-dsi-qsync-min-fps-5-command-state",
	"qcom,mdss-dsi-qsync-min-fps-6-command-state",
	"qcom,mdss-dsi-qsync-min-fps-7-command-state",
	"qcom,mdss-dsi-qsync-min-fps-8-command-state",
	"qcom,mdss-dsi-qsync-min-fps-9-command-state",
	"qcom,mdss-dsi-fakeframe-command-state",
	"qcom,mdss-dsi-adfr-pre-switch-command-state",
#endif
};

int dsi_panel_get_cmd_pkt_count(const char *data, u32 length, u32 *cnt)
{
	const u32 cmd_set_min_size = 7;
	u32 count = 0;
	u32 packet_length;
	u32 tmp;

	while (length >= cmd_set_min_size) {
		packet_length = cmd_set_min_size;
		tmp = ((data[5] << 8) | (data[6]));
		packet_length += tmp;
		if (packet_length > length) {
			DSI_ERR("format error\n");
			return -EINVAL;
		}
		length -= packet_length;
		data += packet_length;
		count++;
	}

	*cnt = count;
	return 0;
}

int dsi_panel_create_cmd_packets(const char *data,
					u32 length,
					u32 count,
					struct dsi_cmd_desc *cmd)
{
	int rc = 0;
	int i, j;
	u8 *payload;

	for (i = 0; i < count; i++) {
		u32 size;

		cmd[i].msg.type = data[0];
		cmd[i].last_command = (data[1] == 1);
		cmd[i].msg.channel = data[2];
		cmd[i].msg.flags |= data[3];
		cmd[i].msg.ctrl = 0;
		cmd[i].post_wait_ms = cmd[i].msg.wait_ms = data[4];
		cmd[i].msg.tx_len = ((data[5] << 8) | (data[6]));

		size = cmd[i].msg.tx_len * sizeof(u8);

		payload = kzalloc(size, GFP_KERNEL);
		if (!payload) {
			rc = -ENOMEM;
			goto error_free_payloads;
		}

		for (j = 0; j < cmd[i].msg.tx_len; j++)
			payload[j] = data[7 + j];

		cmd[i].msg.tx_buf = payload;
		data += (7 + cmd[i].msg.tx_len);
	}

	return rc;
error_free_payloads:
	for (i = i - 1; i >= 0; i--) {
		cmd--;
		kfree(cmd->msg.tx_buf);
	}

	return rc;
}

void dsi_panel_destroy_cmd_packets(struct dsi_panel_cmd_set *set)
{
	u32 i = 0;
	struct dsi_cmd_desc *cmd;

	for (i = 0; i < set->count; i++) {
		cmd = &set->cmds[i];
		kfree(cmd->msg.tx_buf);
	}
}

void dsi_panel_dealloc_cmd_packets(struct dsi_panel_cmd_set *set)
{
	kfree(set->cmds);
}

int dsi_panel_alloc_cmd_packets(struct dsi_panel_cmd_set *cmd,
					u32 packet_count)
{
	u32 size;

	size = packet_count * sizeof(*cmd->cmds);
	cmd->cmds = kzalloc(size, GFP_KERNEL);
	if (!cmd->cmds)
		return -ENOMEM;

	cmd->count = packet_count;
	return 0;
}

static int dsi_panel_parse_cmd_sets_sub(struct dsi_panel_cmd_set *cmd,
					enum dsi_cmd_set_type type,
					struct dsi_parser_utils *utils)
{
	int rc = 0;
	u32 length = 0;
	const char *data;
	const char *state;
	u32 packet_count = 0;

	data = utils->get_property(utils->data, cmd_set_prop_map[type],
			&length);
	if (!data) {
		DSI_DEBUG("%s commands not defined\n", cmd_set_prop_map[type]);
		rc = -ENOTSUPP;
		goto error;
	}

	DSI_DEBUG("type=%d, name=%s, length=%d\n", type,
		cmd_set_prop_map[type], length);

	print_hex_dump_debug("", DUMP_PREFIX_NONE,
		       8, 1, data, length, false);

	rc = dsi_panel_get_cmd_pkt_count(data, length, &packet_count);
	if (rc) {
		DSI_ERR("commands failed, rc=%d\n", rc);
		goto error;
	}
	DSI_DEBUG("[%s] packet-count=%d, %d\n", cmd_set_prop_map[type],
		packet_count, length);

	rc = dsi_panel_alloc_cmd_packets(cmd, packet_count);
	if (rc) {
		DSI_ERR("failed to allocate cmd packets, rc=%d\n", rc);
		goto error;
	}

	rc = dsi_panel_create_cmd_packets(data, length, packet_count,
					  cmd->cmds);
	if (rc) {
		DSI_ERR("failed to create cmd packets, rc=%d\n", rc);
		goto error_free_mem;
	}

	state = utils->get_property(utils->data, cmd_set_state_map[type], NULL);
	if (!state || !strcmp(state, "dsi_lp_mode")) {
		cmd->state = DSI_CMD_SET_STATE_LP;
	} else if (!strcmp(state, "dsi_hs_mode")) {
		cmd->state = DSI_CMD_SET_STATE_HS;
	} else {
		DSI_ERR("[%s] command state unrecognized-%s\n",
		       cmd_set_state_map[type], state);
		goto error_free_mem;
	}

	return rc;
error_free_mem:
	kfree(cmd->cmds);
	cmd->cmds = NULL;
error:
	return rc;

}

static int dsi_panel_parse_cmd_sets(
		struct dsi_display_mode_priv_info *priv_info,
		struct dsi_parser_utils *utils)
{
	int rc = 0;
	struct dsi_panel_cmd_set *set;
	u32 i;

	if (!priv_info) {
		DSI_ERR("invalid mode priv info\n");
		return -EINVAL;
	}

	for (i = DSI_CMD_SET_PRE_ON; i < DSI_CMD_SET_MAX; i++) {
		set = &priv_info->cmd_sets[i];
		set->type = i;
		set->count = 0;

		if (i == DSI_CMD_SET_PPS) {
			rc = dsi_panel_alloc_cmd_packets(set, 1);
			if (rc)
				DSI_ERR("failed to allocate cmd set %d, rc = %d\n",
					i, rc);
			set->state = DSI_CMD_SET_STATE_LP;
#if defined(CONFIG_PXLW_IRIS)
			if (iris_is_chip_supported())
				set->state = DSI_CMD_SET_STATE_HS;
#endif
		} else {
			rc = dsi_panel_parse_cmd_sets_sub(set, i, utils);
			if (rc)
				DSI_DEBUG("failed to parse set %d\n", i);
		}
	}

	rc = 0;
	return rc;
}

static int dsi_panel_parse_reset_sequence(struct dsi_panel *panel)
{
	int rc = 0;
	int i;
	u32 length = 0;
	u32 count = 0;
	u32 size = 0;
	u32 *arr_32 = NULL;
	const u32 *arr;
	struct dsi_parser_utils *utils = &panel->utils;
	struct dsi_reset_seq *seq;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	arr = utils->get_property(utils->data,
			"qcom,mdss-dsi-reset-sequence", &length);
	if (!arr) {
		DSI_ERR("[%s] dsi-reset-sequence not found\n", panel->name);
		rc = -EINVAL;
		goto error;
	}
	if (length & 0x1) {
		DSI_ERR("[%s] syntax error for dsi-reset-sequence\n",
		       panel->name);
		rc = -EINVAL;
		goto error;
	}

	DSI_DEBUG("RESET SEQ LENGTH = %d\n", length);
	length = length / sizeof(u32);

	size = length * sizeof(u32);

	arr_32 = kzalloc(size, GFP_KERNEL);
	if (!arr_32) {
		rc = -ENOMEM;
		goto error;
	}

	rc = utils->read_u32_array(utils->data, "qcom,mdss-dsi-reset-sequence",
					arr_32, length);
	if (rc) {
		DSI_ERR("[%s] cannot read dso-reset-seqience\n", panel->name);
		goto error_free_arr_32;
	}

	count = length / 2;
	size = count * sizeof(*seq);
	seq = kzalloc(size, GFP_KERNEL);
	if (!seq) {
		rc = -ENOMEM;
		goto error_free_arr_32;
	}

	panel->reset_config.sequence = seq;
	panel->reset_config.count = count;

	for (i = 0; i < length; i += 2) {
		seq->level = arr_32[i];
		seq->sleep_ms = arr_32[i + 1];
		seq++;
	}


error_free_arr_32:
	kfree(arr_32);
error:
	return rc;
}

static int dsi_panel_parse_misc_features(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = &panel->utils;
	const char *string;
	int i, rc = 0;

	panel->ulps_feature_enabled =
		utils->read_bool(utils->data, "qcom,ulps-enabled");

	DSI_DEBUG("%s: ulps feature %s\n", __func__,
		(panel->ulps_feature_enabled ? "enabled" : "disabled"));

	panel->ulps_suspend_enabled =
		utils->read_bool(utils->data, "qcom,suspend-ulps-enabled");

	DSI_DEBUG("%s: ulps during suspend feature %s\n", __func__,
		(panel->ulps_suspend_enabled ? "enabled" : "disabled"));

	panel->te_using_watchdog_timer = utils->read_bool(utils->data,
					"qcom,mdss-dsi-te-using-wd");

	panel->sync_broadcast_en = utils->read_bool(utils->data,
			"qcom,cmd-sync-wait-broadcast");

	panel->lp11_init = utils->read_bool(utils->data,
			"qcom,mdss-dsi-lp11-init");

	panel->reset_gpio_always_on = utils->read_bool(utils->data,
			"qcom,platform-reset-gpio-always-on");

	panel->spr_info.enable = false;
	panel->spr_info.pack_type = MSM_DISPLAY_SPR_TYPE_MAX;

	rc = utils->read_string(utils->data, "qcom,spr-pack-type", &string);
	if (!rc) {
		// find match for pack-type string
		for (i = 0; i < MSM_DISPLAY_SPR_TYPE_MAX; i++) {
			if (msm_spr_pack_type_str[i] &&
				(!strcmp(string, msm_spr_pack_type_str[i]))) {
				panel->spr_info.enable = true;
				panel->spr_info.pack_type = i;
				break;
			}
		}
	}

	pr_debug("%s source side spr packing, pack-type %s\n",
		panel->spr_info.enable ? "enable" : "disable",
		panel->spr_info.enable ?
		msm_spr_pack_type_str[panel->spr_info.pack_type] : "none");

	return 0;
}

static int dsi_panel_parse_jitter_config(
				struct dsi_display_mode *mode,
				struct dsi_parser_utils *utils)
{
	int rc;
	struct dsi_display_mode_priv_info *priv_info;
	u32 jitter[DEFAULT_PANEL_JITTER_ARRAY_SIZE] = {0, 0};
	u64 jitter_val = 0;

	priv_info = mode->priv_info;

	rc = utils->read_u32_array(utils->data, "qcom,mdss-dsi-panel-jitter",
				jitter, DEFAULT_PANEL_JITTER_ARRAY_SIZE);
	if (rc) {
		DSI_DEBUG("panel jitter not defined rc=%d\n", rc);
	} else {
		jitter_val = jitter[0];
		jitter_val = div_u64(jitter_val, jitter[1]);
	}

	if (rc || !jitter_val || (jitter_val > MAX_PANEL_JITTER)) {
		priv_info->panel_jitter_numer = DEFAULT_PANEL_JITTER_NUMERATOR;
		priv_info->panel_jitter_denom =
					DEFAULT_PANEL_JITTER_DENOMINATOR;
	} else {
		priv_info->panel_jitter_numer = jitter[0];
		priv_info->panel_jitter_denom = jitter[1];
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-panel-prefill-lines",
				  &priv_info->panel_prefill_lines);
	if (rc) {
		DSI_DEBUG("panel prefill lines are not defined rc=%d\n", rc);
		priv_info->panel_prefill_lines = mode->timing.v_back_porch +
			mode->timing.v_sync_width + mode->timing.v_front_porch;
	} else if (priv_info->panel_prefill_lines >=
					DSI_V_TOTAL(&mode->timing)) {
		DSI_DEBUG("invalid prefill lines config=%d setting to:%d\n",
		priv_info->panel_prefill_lines, DEFAULT_PANEL_PREFILL_LINES);

		priv_info->panel_prefill_lines = DEFAULT_PANEL_PREFILL_LINES;
	}

	return 0;
}

static int dsi_panel_parse_power_cfg(struct dsi_panel *panel)
{
	int rc = 0;
	char *supply_name;

	if (panel->host_config.ext_bridge_mode)
		return 0;

	if (!strcmp(panel->type, "primary"))
		supply_name = "qcom,panel-supply-entries";
	else
		supply_name = "qcom,panel-sec-supply-entries";

	rc = dsi_pwr_of_get_vreg_data(&panel->utils,
			&panel->power_info, supply_name);
	if (rc) {
		DSI_ERR("[%s] failed to parse vregs\n", panel->name);
		goto error;
	}

error:
	return rc;
}

int dsi_panel_get_io_resources(struct dsi_panel *panel,
		struct msm_io_res *io_res)
{
	struct list_head temp_head;
	struct msm_io_mem_entry *io_mem, *pos, *tmp;
	struct list_head *mem_list = &io_res->mem;
	int i, rc = 0;

	INIT_LIST_HEAD(&temp_head);

	for (i = 0; i < panel->tlmm_gpio_count; i++) {
		io_mem = kzalloc(sizeof(*io_mem), GFP_KERNEL);
		if (!io_mem) {
			rc = -ENOMEM;
			goto parse_fail;
		}

		io_mem->base = panel->tlmm_gpio[i].addr;
		io_mem->size = panel->tlmm_gpio[i].size;

		list_add(&io_mem->list, &temp_head);
	}

	list_splice(&temp_head, mem_list);
	goto end;

parse_fail:
	list_for_each_entry_safe(pos, tmp, &temp_head, list) {
		list_del(&pos->list);
		kzfree(pos);
	}
end:
	return rc;
}

static int dsi_panel_parse_gpios(struct dsi_panel *panel)
{
	int rc = 0;
	const char *data;
	struct dsi_parser_utils *utils = &panel->utils;
	char *reset_gpio_name, *mode_set_gpio_name;
#if defined(CONFIG_PXLW_IRIS)
	bool is_primary = false;
#endif

	if (!strcmp(panel->type, "primary")) {
		reset_gpio_name = "qcom,platform-reset-gpio";
		mode_set_gpio_name = "qcom,panel-mode-gpio";
#if defined(CONFIG_PXLW_IRIS)
		if (iris_is_chip_supported())
			is_primary = true;
#endif
	} else {
		reset_gpio_name = "qcom,platform-sec-reset-gpio";
		mode_set_gpio_name = "qcom,panel-sec-mode-gpio";
	}

	panel->reset_config.reset_gpio = utils->get_named_gpio(utils->data,
					      reset_gpio_name, 0);
	if (!gpio_is_valid(panel->reset_config.reset_gpio) &&
		!panel->host_config.ext_bridge_mode) {
#if defined(CONFIG_PXLW_IRIS)
		if (iris_is_chip_supported()) {
			if (is_primary) {
				rc = panel->reset_config.reset_gpio;
				DSI_ERR("[%s] failed get primary reset gpio, rc=%d\n", panel->name, rc);
				goto error;
			}
		} else {
#endif
		rc = panel->reset_config.reset_gpio;
		DSI_ERR("[%s] failed get reset gpio, rc=%d\n", panel->name, rc);
		goto error;
#if defined(CONFIG_PXLW_IRIS)
		}
#endif
	}

	panel->reset_config.disp_en_gpio = utils->get_named_gpio(utils->data,
						"qcom,5v-boost-gpio",
						0);
	if (!gpio_is_valid(panel->reset_config.disp_en_gpio)) {
		DSI_DEBUG("[%s] 5v-boot-gpio is not set, rc=%d\n",
			 panel->name, rc);
		panel->reset_config.disp_en_gpio =
				utils->get_named_gpio(utils->data,
					"qcom,platform-en-gpio", 0);
		if (!gpio_is_valid(panel->reset_config.disp_en_gpio)) {
			DSI_DEBUG("[%s] platform-en-gpio is not set, rc=%d\n",
				 panel->name, rc);
		}
	}

	panel->poc = utils->get_named_gpio(utils->data, "qcom,platform-poc-gpio", 0);
	if (!gpio_is_valid(panel->poc)) {
		DSI_DEBUG("[%s] platform-poc-gpio is not set, rc=%d\n",
			 panel->name, rc);
	}

	panel->vddr_gpio = utils->get_named_gpio(utils->data, "qcom,vddr-gpio", 0);
	if (!gpio_is_valid(panel->vddr_gpio)) {
		DSI_DEBUG("[%s] vddr-gpio is not set, rc=%d\n",
			 panel->name, rc);
	}

	panel->avdd_gpio = utils->get_named_gpio(utils->data, "qcom,avdd-gpio", 0);
	if (!gpio_is_valid(panel->avdd_gpio)) {
		DSI_DEBUG("[%s] vdd-gpio is not set, rc=%d\n",
			 panel->name, rc);
	}

	panel->tp1v8_gpio = utils->get_named_gpio(utils->data, "qcom,tp1v8-gpio", 0);
	if (!gpio_is_valid(panel->tp1v8_gpio)) {
		DSI_DEBUG("[%s] tp1v8_gpio is not set, rc=%d\n",
			 panel->name, rc);
	}

	panel->err_flag_gpio = utils->get_named_gpio(utils->data, "qcom,err-flag-gpio", 0);
	if (!gpio_is_valid(panel->err_flag_gpio)) {
		DSI_DEBUG("[%s] err_flag_gpio is not set, rc=%d\n",
			 panel->name, rc);
	}

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		panel->vsync_switch_gpio = utils->get_named_gpio(utils->data, "qcom,vsync-switch-gpio", 0);
		if (!gpio_is_valid(panel->vsync_switch_gpio)) {
			DSI_DEBUG("[%s] vsync_switch_gpio is not set, rc=%d\n", panel->name, rc);
		}
	}
#endif /*(CONFIG_PXLW_IRIS)*/

	panel->reset_config.lcd_mode_sel_gpio = utils->get_named_gpio(
		utils->data, mode_set_gpio_name, 0);
	if (!gpio_is_valid(panel->reset_config.lcd_mode_sel_gpio))
		DSI_DEBUG("mode gpio not specified\n");

	DSI_DEBUG("mode gpio=%d\n", panel->reset_config.lcd_mode_sel_gpio);

	data = utils->get_property(utils->data,
		"qcom,mdss-dsi-mode-sel-gpio-state", NULL);
	if (data) {
		if (!strcmp(data, "single_port"))
			panel->reset_config.mode_sel_state =
				MODE_SEL_SINGLE_PORT;
		else if (!strcmp(data, "dual_port"))
			panel->reset_config.mode_sel_state =
				MODE_SEL_DUAL_PORT;
		else if (!strcmp(data, "high"))
			panel->reset_config.mode_sel_state =
				MODE_GPIO_HIGH;
		else if (!strcmp(data, "low"))
			panel->reset_config.mode_sel_state =
				MODE_GPIO_LOW;
	} else {
		/* Set default mode as SPLIT mode */
		panel->reset_config.mode_sel_state = MODE_SEL_DUAL_PORT;
	}

	/* TODO:  release memory */
	rc = dsi_panel_parse_reset_sequence(panel);
	if (rc) {
		DSI_ERR("[%s] failed to parse reset sequence, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

	panel->panel_test_gpio = utils->get_named_gpio(utils->data,
					"qcom,mdss-dsi-panel-test-pin",
					0);
	if (!gpio_is_valid(panel->panel_test_gpio))
		DSI_DEBUG("%s:%d panel test gpio not specified\n", __func__,
			 __LINE__);

error:
	return rc;
}

static int dsi_panel_parse_tlmm_gpio(struct dsi_panel *panel)
{
	struct dsi_parser_utils *utils = &panel->utils;
	u32 base, size, pin;
	int pin_count, address_count, name_count, i;

	address_count = of_property_count_u32_elems(utils->data,
				"qcom,dsi-panel-gpio-address");
	if (address_count != 2) {
		DSI_DEBUG("panel gpio address not defined\n");
		return 0;
	}

	of_property_read_u32_index(utils->data,
			"qcom,dsi-panel-gpio-address", 0, &base);
	of_property_read_u32_index(utils->data,
			"qcom,dsi-panel-gpio-address", 1, &size);

	pin_count = of_property_count_u32_elems(utils->data,
				"qcom,dsi-panel-gpio-pins");
	name_count = of_property_count_strings(utils->data,
				"qcom,dsi-panel-gpio-names");
	if ((pin_count < 0) || (name_count < 0) || (pin_count != name_count)) {
		DSI_ERR("invalid gpio pins/names\n");
		return -EINVAL;
	}

	panel->tlmm_gpio = kcalloc(pin_count,
				sizeof(struct dsi_tlmm_gpio), GFP_KERNEL);
	if (!panel->tlmm_gpio)
		return -ENOMEM;

	panel->tlmm_gpio_count = pin_count;
	for (i = 0; i < pin_count; i++) {
		of_property_read_u32_index(utils->data,
				"qcom,dsi-panel-gpio-pins", i, &pin);
		panel->tlmm_gpio[i].num = pin;
		panel->tlmm_gpio[i].addr = base + (pin * size);
		panel->tlmm_gpio[i].size = size;

		of_property_read_string_index(utils->data,
				"qcom,dsi-panel-gpio-names", i,
				&(panel->tlmm_gpio[i].name));
	}

	return 0;
}

static int dsi_panel_parse_bl_pwm_config(struct dsi_panel *panel)
{
	int rc = 0;
	u32 val;
	struct dsi_backlight_config *config = &panel->bl_config;
	struct dsi_parser_utils *utils = &panel->utils;

	rc = utils->read_u32(utils->data, "qcom,bl-pmic-pwm-period-usecs",
				  &val);
	if (rc) {
		DSI_ERR("bl-pmic-pwm-period-usecs is not defined, rc=%d\n", rc);
		goto error;
	}
	config->pwm_period_usecs = val;

error:
	return rc;
}

static int dsi_panel_parse_bl_config(struct dsi_panel *panel)
{
	int rc = 0;
	u32 val = 0;
	const char *bl_type = NULL;
	const char *data = NULL;
	const char *state = NULL;
	struct dsi_parser_utils *utils = &panel->utils;
	char *bl_name = NULL;

	if (!strcmp(panel->type, "primary"))
		bl_name = "qcom,mdss-dsi-bl-pmic-control-type";
	else
		bl_name = "qcom,mdss-dsi-sec-bl-pmic-control-type";

	bl_type = utils->get_property(utils->data, bl_name, NULL);
	if (!bl_type) {
		panel->bl_config.type = DSI_BACKLIGHT_UNKNOWN;
	} else if (!strcmp(bl_type, "bl_ctrl_pwm")) {
		panel->bl_config.type = DSI_BACKLIGHT_PWM;
	} else if (!strcmp(bl_type, "bl_ctrl_wled")) {
		panel->bl_config.type = DSI_BACKLIGHT_WLED;
	} else if (!strcmp(bl_type, "bl_ctrl_dcs")) {
		panel->bl_config.type = DSI_BACKLIGHT_DCS;
	} else if (!strcmp(bl_type, "bl_ctrl_external")) {
		panel->bl_config.type = DSI_BACKLIGHT_EXTERNAL;
	} else {
		DSI_DEBUG("[%s] bl-pmic-control-type unknown-%s\n",
			 panel->name, bl_type);
		panel->bl_config.type = DSI_BACKLIGHT_UNKNOWN;
	}

	data = utils->get_property(utils->data, "qcom,bl-update-flag", NULL);
	if (!data) {
		panel->bl_config.bl_update = BL_UPDATE_NONE;
	} else if (!strcmp(data, "delay_until_first_frame")) {
		panel->bl_config.bl_update = BL_UPDATE_DELAY_UNTIL_FIRST_FRAME;
	} else {
		DSI_DEBUG("[%s] No valid bl-update-flag: %s\n",
						panel->name, data);
		panel->bl_config.bl_update = BL_UPDATE_NONE;
	}

	panel->bl_config.bl_scale = MAX_BL_SCALE_LEVEL;
	panel->bl_config.bl_scale_sv = MAX_SV_BL_SCALE_LEVEL;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-bl-min-level", &val);
	if (rc) {
		DSI_DEBUG("[%s] bl-min-level unspecified, defaulting to zero\n",
			 panel->name);
		panel->bl_config.bl_min_level = 0;
	} else {
		panel->bl_config.bl_min_level = val;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-bl-max-level", &val);
	if (rc) {
		DSI_DEBUG("[%s] bl-max-level unspecified, defaulting to max level\n",
			 panel->name);
		panel->bl_config.bl_max_level = MAX_BL_LEVEL;
	} else {
		panel->bl_config.bl_max_level = val;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-brightness-default-val", &val);
	if (rc) {
		DSI_DEBUG("[%s] brightness-default-val unspecified, defaulting to val\n",
			 panel->name);
		panel->bl_config.bl_def_val = 200;
	} else {
		panel->bl_config.bl_def_val = val;
	}
	DSI_ERR("default backlight bl_def_val= %d\n",panel->bl_config.bl_def_val);
	rc = utils->read_u32(utils->data, "qcom,mdss-brightness-max-level",
		&val);
	if (rc) {
		DSI_DEBUG("[%s] brigheness-max-level unspecified, defaulting to 255\n",
			 panel->name);
		panel->bl_config.brightness_max_level = 255;
		rc = 0;
	} else {
		panel->bl_config.brightness_max_level = val;
	}

	panel->bl_config.bl_inverted_dbv = utils->read_bool(utils->data,
		"qcom,mdss-dsi-bl-inverted-dbv");

	state = utils->get_property(utils->data, "qcom,bl-dsc-cmd-state", NULL);
	if (!state || !strcmp(state, "dsi_hs_mode"))
		panel->bl_config.lp_mode = false;
	else if (!strcmp(state, "dsi_lp_mode"))
		panel->bl_config.lp_mode = true;
	else
		DSI_ERR("bl-dsc-cmd-state command state unrecognized-%s\n",
			state);

	if (panel->bl_config.type == DSI_BACKLIGHT_PWM) {
		rc = dsi_panel_parse_bl_pwm_config(panel);
		if (rc) {
			DSI_ERR("[%s] failed to parse pwm config, rc=%d\n",
			       panel->name, rc);
			goto error;
		}
	}

	panel->bl_config.en_gpio = utils->get_named_gpio(utils->data,
					      "qcom,platform-bklight-en-gpio",
					      0);
	if (!gpio_is_valid(panel->bl_config.en_gpio)) {
		if (panel->bl_config.en_gpio == -EPROBE_DEFER) {
			DSI_DEBUG("[%s] failed to get bklt gpio, rc=%d\n",
					panel->name, rc);
			rc = -EPROBE_DEFER;
			goto error;
		} else {
			DSI_DEBUG("[%s] failed to get bklt gpio, rc=%d\n",
					 panel->name, rc);
			rc = 0;
			goto error;
		}
	}

error:
	return rc;
}

static int dsi_panel_parse_phy_timing(struct dsi_display_mode *mode,
		struct dsi_parser_utils *utils)
{
	const char *data;
	u32 len, i;
	int rc = 0;
	struct dsi_display_mode_priv_info *priv_info;
	u64 pixel_clk_khz;

	if (!mode || !mode->priv_info)
		return -EINVAL;

	priv_info = mode->priv_info;

	data = utils->get_property(utils->data,
			"qcom,mdss-dsi-panel-phy-timings", &len);
	if (!data) {
		DSI_DEBUG("Unable to read Phy timing settings\n");
	} else {
		priv_info->phy_timing_val =
			kzalloc((sizeof(u32) * len), GFP_KERNEL);
		if (!priv_info->phy_timing_val)
			return -EINVAL;

		for (i = 0; i < len; i++)
			priv_info->phy_timing_val[i] = data[i];

		priv_info->phy_timing_len = len;
	}

	if (mode->panel_mode == DSI_OP_VIDEO_MODE) {
		/*
		 *  For command mode we update the pclk as part of
		 *  function dsi_panel_calc_dsi_transfer_time( )
		 *  as we set it based on dsi clock or mdp transfer time.
		 */
		pixel_clk_khz = (dsi_h_total_dce(&mode->timing) *
				DSI_V_TOTAL(&mode->timing) *
				mode->timing.refresh_rate);
		do_div(pixel_clk_khz, 1000);
		mode->pixel_clk_khz = pixel_clk_khz;
	}

	return rc;
}

static int dsi_panel_parse_dsc_params(struct dsi_display_mode *mode,
				struct dsi_parser_utils *utils)
{
	u32 data;
	int rc = -EINVAL;
	int intf_width;
	const char *compression;
	struct dsi_display_mode_priv_info *priv_info;

	if (!mode || !mode->priv_info)
		return -EINVAL;

	priv_info = mode->priv_info;

	priv_info->dsc_enabled = false;
	compression = utils->get_property(utils->data,
			"qcom,compression-mode", NULL);
	if (compression && !strcmp(compression, "dsc"))
		priv_info->dsc_enabled = true;

	if (!priv_info->dsc_enabled) {
		DSI_DEBUG("dsc compression is not enabled for the mode\n");
		return 0;
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-version", &data);
	if (rc) {
		priv_info->dsc.config.dsc_version_major = 0x1;
		priv_info->dsc.config.dsc_version_minor = 0x1;
		rc = 0;
	} else {
		/* BITS[0..3] provides minor version and BITS[4..7] provide
		 * major version information
		 */
		priv_info->dsc.config.dsc_version_major = (data >> 4) & 0x0F;
		priv_info->dsc.config.dsc_version_minor = data & 0x0F;
		if ((priv_info->dsc.config.dsc_version_major != 0x1) &&
				((priv_info->dsc.config.dsc_version_minor
				  != 0x1) ||
				 (priv_info->dsc.config.dsc_version_minor
				  != 0x2))) {
			DSI_ERR("%s:unsupported major:%d minor:%d version\n",
					__func__,
					priv_info->dsc.config.dsc_version_major,
					priv_info->dsc.config.dsc_version_minor
					);
			rc = -EINVAL;
			goto error;
		}
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-scr-version", &data);
	if (rc) {
		priv_info->dsc.scr_rev = 0x0;
		rc = 0;
	} else {
		priv_info->dsc.scr_rev = data & 0xff;
		/* only one scr rev supported */
		if (priv_info->dsc.scr_rev > 0x1) {
			DSI_ERR("%s: DSC scr version:%d not supported\n",
					__func__, priv_info->dsc.scr_rev);
			rc = -EINVAL;
			goto error;
		}
	}

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-slice-height", &data);
	if (rc) {
		DSI_ERR("failed to parse qcom,mdss-dsc-slice-height\n");
		goto error;
	}
	priv_info->dsc.config.slice_height = data;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-slice-width", &data);
	if (rc) {
		DSI_ERR("failed to parse qcom,mdss-dsc-slice-width\n");
		goto error;
	}
	priv_info->dsc.config.slice_width = data;

	intf_width = mode->timing.h_active;
	if (intf_width % priv_info->dsc.config.slice_width) {
		DSI_ERR("invalid slice width for the intf width:%d slice width:%d\n",
			intf_width, priv_info->dsc.config.slice_width);
		rc = -EINVAL;
		goto error;
	}

	priv_info->dsc.config.pic_width = mode->timing.h_active;
	priv_info->dsc.config.pic_height = mode->timing.v_active;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-slice-per-pkt", &data);
	if (rc) {
		DSI_ERR("failed to parse qcom,mdss-dsc-slice-per-pkt\n");
		goto error;
	} else if (!data || (data > 2)) {
		DSI_ERR("invalid dsc slice-per-pkt:%d\n", data);
		goto error;
	}
	priv_info->dsc.slice_per_pkt = data;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-bit-per-component",
		&data);
	if (rc) {
		DSI_ERR("failed to parse qcom,mdss-dsc-bit-per-component\n");
		goto error;
	}
	priv_info->dsc.config.bits_per_component = data;

	rc = utils->read_u32(utils->data, "qcom,mdss-pps-delay-ms", &data);
	if (rc) {
		DSI_DEBUG("pps-delay-ms not specified, defaulting to 0\n");
		data = 0;
	}
	priv_info->dsc.pps_delay_ms = data;

	rc = utils->read_u32(utils->data, "qcom,mdss-dsc-bit-per-pixel",
			&data);
	if (rc) {
		DSI_ERR("failed to parse qcom,mdss-dsc-bit-per-pixel\n");
		goto error;
	}
	priv_info->dsc.config.bits_per_pixel = data << 4;

	rc = utils->read_u32(utils->data, "qcom,src-chroma-format",
			&data);
	if (rc) {
		DSI_DEBUG("failed to parse qcom,src-chroma-format\n");
		rc = 0;
		data = MSM_CHROMA_444;
	}

	priv_info->dsc.chroma_format = data;

	rc = utils->read_u32(utils->data, "qcom,src-color-space",
			&data);
	if (rc) {
		DSI_DEBUG("failed to parse qcom,src-color-space\n");
		rc = 0;
		data = MSM_RGB;
	}

	priv_info->dsc.source_color_space = data;

	priv_info->dsc.config.block_pred_enable = utils->read_bool(utils->data,
		"qcom,mdss-dsc-block-prediction-enable");

	priv_info->dsc.config.slice_count = DIV_ROUND_UP(intf_width,
		priv_info->dsc.config.slice_width);

	rc = sde_dsc_populate_dsc_config(&priv_info->dsc.config,
			priv_info->dsc.scr_rev);
	if (rc) {
		DSI_DEBUG("failed populating dsc params\n");
		rc = -EINVAL;
		goto error;
	}

	rc = sde_dsc_populate_dsc_private_params(&priv_info->dsc, intf_width);
	if (rc) {
		DSI_DEBUG("failed populating other dsc params\n");
		rc = -EINVAL;
		goto error;
	}

	priv_info->pclk_scale.numer =
			priv_info->dsc.config.bits_per_pixel >> 4;
	priv_info->pclk_scale.denom = msm_get_src_bpc(
			priv_info->dsc.chroma_format,
			priv_info->dsc.config.bits_per_component);

	mode->timing.dsc_enabled = true;
	mode->timing.dsc = &priv_info->dsc;
	mode->timing.pclk_scale = priv_info->pclk_scale;

error:
	return rc;
}

static int dsi_panel_parse_vdc_params(struct dsi_display_mode *mode,
	struct dsi_parser_utils *utils, int traffic_mode, int panel_mode)
{
	u32 data;
	int rc = -EINVAL;
	const char *compression;
	struct dsi_display_mode_priv_info *priv_info;
	int intf_width;

	if (!mode || !mode->priv_info)
		return -EINVAL;

	priv_info = mode->priv_info;

	priv_info->vdc_enabled = false;
	compression = utils->get_property(utils->data,
			"qcom,compression-mode", NULL);
	if (compression && !strcmp(compression, "vdc"))
		priv_info->vdc_enabled = true;

	if (!priv_info->vdc_enabled) {
		DSI_DEBUG("vdc compression is not enabled for the mode\n");
		return 0;
	}

	priv_info->vdc.panel_mode = panel_mode;
	priv_info->vdc.traffic_mode = traffic_mode;

	rc = utils->read_u32(utils->data, "qcom,vdc-version", &data);
	if (rc) {
		priv_info->vdc.version_major = 0x1;
		priv_info->vdc.version_minor = 0x2;
		priv_info->vdc.version_release = 0x0;
		rc = 0;
	} else {
		/* BITS[0..3] provides minor version and BITS[4..7] provide
		 * major version information
		 */
		priv_info->vdc.version_major = (data >> 4) & 0x0F;
		priv_info->vdc.version_minor = data & 0x0F;
		if ((priv_info->vdc.version_major != 0x1) &&
				((priv_info->vdc.version_minor
				  != 0x2))) {
			DSI_ERR("%s:unsupported major:%d minor:%d version\n",
					__func__,
					priv_info->vdc.version_major,
					priv_info->vdc.version_minor
					);
			rc = -EINVAL;
			goto error;
		}
	}

	rc = utils->read_u32(utils->data, "qcom,vdc-version-release", &data);
	if (rc) {
		priv_info->vdc.version_release = 0x0;
		rc = 0;
	} else {
		priv_info->vdc.version_release = data & 0xff;
		/* only one release version is supported */
		if (priv_info->vdc.version_release != 0x0) {
			DSI_ERR("unsupported vdc release version %d\n",
					priv_info->vdc.version_release);
			rc = -EINVAL;
			goto error;
		}
	}

	DSI_INFO("vdc major: 0x%x minor : 0x%x release : 0x%x\n",
			priv_info->vdc.version_major,
			priv_info->vdc.version_minor,
			priv_info->vdc.version_release);

	rc = utils->read_u32(utils->data, "qcom,vdc-slice-height", &data);
	if (rc) {
		DSI_ERR("failed to parse qcom,vdc-slice-height\n");
		goto error;
	}
	priv_info->vdc.slice_height = data;

	/* slice height should be atleast 16 lines */
	if (priv_info->vdc.slice_height < 16) {
		DSI_ERR("invalid slice height %d\n",
			priv_info->vdc.slice_height);
		rc = -EINVAL;
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,vdc-slice-width", &data);
	if (rc) {
		DSI_ERR("failed to parse qcom,vdc-slice-width\n");
		goto error;
	}

	priv_info->vdc.slice_width = data;

	/*
	 * slide-width should be multiple of 8
	 * slice-width should be atlease 64 pixels
	 */
	if ((priv_info->vdc.slice_width & 7) ||
		(priv_info->vdc.slice_width < 64)) {
		DSI_ERR("invalid slice width:%d\n", priv_info->vdc.slice_width);
		rc = -EINVAL;
		goto error;
	}

	rc = utils->read_u32(utils->data, "qcom,vdc-slice-per-pkt", &data);
	if (rc) {
		DSI_ERR("failed to parse qcom,vdc-slice-per-pkt\n");
		goto error;
	} else if (!data || (data > 2)) {
		DSI_ERR("invalid vdc slice-per-pkt:%d\n", data);
		rc = -EINVAL;
		goto error;
	}

	intf_width = mode->timing.h_active;
	priv_info->vdc.slice_per_pkt = data;

	priv_info->vdc.frame_width = mode->timing.h_active;
	priv_info->vdc.frame_height = mode->timing.v_active;

	rc = utils->read_u32(utils->data, "qcom,vdc-bit-per-component",
		&data);
	if (rc) {
		DSI_ERR("failed to parse qcom,vdc-bit-per-component\n");
		goto error;
	}
	priv_info->vdc.bits_per_component = data;

	rc = utils->read_u32(utils->data, "qcom,mdss-pps-delay-ms", &data);
	if (rc) {
		DSI_DEBUG("pps-delay-ms not specified, defaulting to 0\n");
		data = 0;
	}
	priv_info->vdc.pps_delay_ms = data;

	rc = utils->read_u32(utils->data, "qcom,vdc-bit-per-pixel",
			&data);
	if (rc) {
		DSI_ERR("failed to parse qcom,vdc-bit-per-pixel\n");
		goto error;
	}
	priv_info->vdc.bits_per_pixel = data << 4;

	rc = utils->read_u32(utils->data, "qcom,src-chroma-format",
			&data);
	if (rc) {
		DSI_DEBUG("failed to parse qcom,src-chroma-format\n");
		rc = 0;
		data = MSM_CHROMA_444;
	}
	priv_info->vdc.chroma_format = data;

	rc = utils->read_u32(utils->data, "qcom,src-color-space",
			&data);
	if (rc) {
		DSI_DEBUG("failed to parse qcom,src-color-space\n");
		rc = 0;
		data = MSM_RGB;
	}
	priv_info->vdc.source_color_space = data;

	rc = sde_vdc_populate_config(&priv_info->vdc,
		intf_width, traffic_mode);
	if (rc) {
		DSI_DEBUG("failed populating vdc config\n");
		rc = -EINVAL;
		goto error;
	}

	priv_info->pclk_scale.numer =
			priv_info->vdc.bits_per_pixel >> 4;
	priv_info->pclk_scale.denom = msm_get_src_bpc(
			priv_info->vdc.chroma_format,
			priv_info->vdc.bits_per_component);

	mode->timing.vdc_enabled = true;
	mode->timing.vdc = &priv_info->vdc;
	mode->timing.pclk_scale = priv_info->pclk_scale;

error:
	return rc;
}

static int dsi_panel_parse_hdr_config(struct dsi_panel *panel)
{
	int rc = 0;
	struct drm_panel_hdr_properties *hdr_prop;
	struct dsi_parser_utils *utils = &panel->utils;

	hdr_prop = &panel->hdr_props;
	hdr_prop->hdr_enabled = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-hdr-enabled");

	if (hdr_prop->hdr_enabled) {
		rc = utils->read_u32_array(utils->data,
				"qcom,mdss-dsi-panel-hdr-color-primaries",
				hdr_prop->display_primaries,
				DISPLAY_PRIMARIES_MAX);
		if (rc) {
			DSI_ERR("%s:%d, Unable to read color primaries,rc:%u\n",
					__func__, __LINE__, rc);
			hdr_prop->hdr_enabled = false;
			return rc;
		}

		rc = utils->read_u32(utils->data,
			"qcom,mdss-dsi-panel-peak-brightness",
			&(hdr_prop->peak_brightness));
		if (rc) {
			DSI_ERR("%s:%d, Unable to read hdr brightness, rc:%u\n",
				__func__, __LINE__, rc);
			hdr_prop->hdr_enabled = false;
			return rc;
		}

		rc = utils->read_u32(utils->data,
			"qcom,mdss-dsi-panel-blackness-level",
			&(hdr_prop->blackness_level));
		if (rc) {
			DSI_ERR("%s:%d, Unable to read hdr brightness, rc:%u\n",
				__func__, __LINE__, rc);
			hdr_prop->hdr_enabled = false;
			return rc;
		}
	}
	return 0;
}

static int dsi_panel_parse_topology(
		struct dsi_display_mode_priv_info *priv_info,
		struct dsi_parser_utils *utils,
		int topology_override)
{
	struct msm_display_topology *topology;
	u32 top_count, top_sel, *array = NULL;
	int i, len = 0;
	int rc = -EINVAL;

	len = utils->count_u32_elems(utils->data, "qcom,display-topology");
	if (len <= 0 || len % TOPOLOGY_SET_LEN ||
			len > (TOPOLOGY_SET_LEN * MAX_TOPOLOGY)) {
		DSI_ERR("invalid topology list for the panel, rc = %d\n", rc);
		return rc;
	}

	top_count = len / TOPOLOGY_SET_LEN;

	array = kcalloc(len, sizeof(u32), GFP_KERNEL);
	if (!array)
		return -ENOMEM;

	rc = utils->read_u32_array(utils->data,
			"qcom,display-topology", array, len);
	if (rc) {
		DSI_ERR("unable to read the display topologies, rc = %d\n", rc);
		goto read_fail;
	}

	topology = kcalloc(top_count, sizeof(*topology), GFP_KERNEL);
	if (!topology) {
		rc = -ENOMEM;
		goto read_fail;
	}

	for (i = 0; i < top_count; i++) {
		struct msm_display_topology *top = &topology[i];

		top->num_lm = array[i * TOPOLOGY_SET_LEN];
		top->num_enc = array[i * TOPOLOGY_SET_LEN + 1];
		top->num_intf = array[i * TOPOLOGY_SET_LEN + 2];
	}

	if (topology_override >= 0 && topology_override < top_count) {
		DSI_INFO("override topology: cfg:%d lm:%d comp_enc:%d intf:%d\n",
			topology_override,
			topology[topology_override].num_lm,
			topology[topology_override].num_enc,
			topology[topology_override].num_intf);
		top_sel = topology_override;
		goto parse_done;
	}

	rc = utils->read_u32(utils->data,
			"qcom,default-topology-index", &top_sel);
	if (rc) {
		DSI_ERR("no default topology selected, rc = %d\n", rc);
		goto parse_fail;
	}

	if (top_sel >= top_count) {
		rc = -EINVAL;
		DSI_ERR("default topology is specified is not valid, rc = %d\n",
			rc);
		goto parse_fail;
	}

	DSI_INFO("default topology: lm: %d comp_enc:%d intf: %d\n",
		topology[top_sel].num_lm,
		topology[top_sel].num_enc,
		topology[top_sel].num_intf);

parse_done:
	memcpy(&priv_info->topology, &topology[top_sel],
		sizeof(struct msm_display_topology));
parse_fail:
	kfree(topology);
read_fail:
	kfree(array);

	return rc;
}

static int dsi_panel_parse_roi_alignment(struct dsi_parser_utils *utils,
					 struct msm_roi_alignment *align)
{
	int len = 0, rc = 0;
	u32 value[6];
	struct property *data;

	if (!align)
		return -EINVAL;

	memset(align, 0, sizeof(*align));

	data = utils->find_property(utils->data,
			"qcom,panel-roi-alignment", &len);
	len /= sizeof(u32);
	if (!data) {
		DSI_ERR("panel roi alignment not found\n");
		rc = -EINVAL;
	} else if (len != 6) {
		DSI_ERR("incorrect roi alignment len %d\n", len);
		rc = -EINVAL;
	} else {
		rc = utils->read_u32_array(utils->data,
				"qcom,panel-roi-alignment", value, len);
		if (rc)
			DSI_DEBUG("error reading panel roi alignment values\n");
		else {
			align->xstart_pix_align = value[0];
			align->ystart_pix_align = value[1];
			align->width_pix_align = value[2];
			align->height_pix_align = value[3];
			align->min_width = value[4];
			align->min_height = value[5];
		}

		DSI_INFO("roi alignment: [%d, %d, %d, %d, %d, %d]\n",
			align->xstart_pix_align,
			align->width_pix_align,
			align->ystart_pix_align,
			align->height_pix_align,
			align->min_width,
			align->min_height);
	}

	return rc;
}

static int dsi_panel_parse_partial_update_caps(struct dsi_display_mode *mode,
				struct dsi_parser_utils *utils)
{
	struct msm_roi_caps *roi_caps = NULL;
	const char *data;
	int rc = 0;

	if (!mode || !mode->priv_info) {
		DSI_ERR("invalid arguments\n");
		return -EINVAL;
	}

	roi_caps = &mode->priv_info->roi_caps;

	memset(roi_caps, 0, sizeof(*roi_caps));

	data = utils->get_property(utils->data,
		"qcom,partial-update-enabled", NULL);
	if (data) {
		if (!strcmp(data, "dual_roi"))
			roi_caps->num_roi = 2;
		else if (!strcmp(data, "single_roi"))
			roi_caps->num_roi = 1;
		else {
			DSI_INFO(
			"invalid value for qcom,partial-update-enabled: %s\n",
			data);
			return 0;
		}
	} else {
		DSI_DEBUG("partial update disabled as the property is not set\n");
		return 0;
	}

	roi_caps->merge_rois = utils->read_bool(utils->data,
			"qcom,partial-update-roi-merge");

	roi_caps->enabled = roi_caps->num_roi > 0;

	DSI_DEBUG("partial update num_rois=%d enabled=%d\n", roi_caps->num_roi,
			roi_caps->enabled);

	if (roi_caps->enabled)
		rc = dsi_panel_parse_roi_alignment(utils,
				&roi_caps->align);

	if (rc)
		memset(roi_caps, 0, sizeof(*roi_caps));

	return rc;
}

static int dsi_panel_parse_panel_mode_caps(struct dsi_display_mode *mode,
				struct dsi_parser_utils *utils)
{
	bool vid_mode_support, cmd_mode_support;

	if (!mode || !mode->priv_info) {
		DSI_ERR("invalid arguments\n");
		return -EINVAL;
	}

	vid_mode_support = utils->read_bool(utils->data,
				"qcom,mdss-dsi-video-mode");

	cmd_mode_support = utils->read_bool(utils->data,
				"qcom,mdss-dsi-cmd-mode");

	if (cmd_mode_support)
		mode->panel_mode = DSI_OP_CMD_MODE;
	else if (vid_mode_support)
		mode->panel_mode = DSI_OP_VIDEO_MODE;
	else
		return -EINVAL;

	return 0;
};


static int dsi_panel_parse_dms_info(struct dsi_panel *panel)
{
	int dms_enabled;
	const char *data;
	struct dsi_parser_utils *utils = &panel->utils;

	panel->dms_mode = DSI_DMS_MODE_DISABLED;
	dms_enabled = utils->read_bool(utils->data,
		"qcom,dynamic-mode-switch-enabled");
	if (!dms_enabled)
		return 0;

	data = utils->get_property(utils->data,
			"qcom,dynamic-mode-switch-type", NULL);
	if (data && !strcmp(data, "dynamic-resolution-switch-immediate")) {
		panel->dms_mode = DSI_DMS_MODE_RES_SWITCH_IMMEDIATE;
	} else {
		DSI_ERR("[%s] unsupported dynamic switch mode: %s\n",
							panel->name, data);
		return -EINVAL;
	}

	return 0;
};
static int dsi_panel_parse_oem_config(struct dsi_panel *panel,
				     struct device_node *of_node)
{
	u32 tmp = 0;
	int rc;
	static const char *panel_manufacture;
	static const char *panel_version;
	static const char *backlight_manufacture;
	static const char *backlight_version;

	DSI_DEBUG("%s start\n", __func__);

	panel->lp11_init =
		of_property_read_bool(of_node, "qcom,mdss-dsi-lp11-init");
	if (panel->lp11_init)
		DSI_DEBUG("lp11_init:%d\n", panel->lp11_init);

	panel->bl_config.bl_high2bit =
		of_property_read_bool(of_node, "qcom,mdss-bl-high2bit");
	if (panel->bl_config.bl_high2bit) {
		DSI_DEBUG("bl_high2bit:%d\n", panel->bl_config.bl_high2bit);
	}

	panel->naive_display_loading_effect_mode =
		of_property_read_bool(of_node, "qcom,mdss-loading-effect");
	if (panel->naive_display_loading_effect_mode)
		DSI_INFO("naive_display_loading_effect_mode:%d\n", panel->naive_display_loading_effect_mode);

	panel_manufacture = of_get_property(of_node,
		"qcom,mdss-dsi-panel-manufacture", NULL);
	if (!panel_manufacture)
		DSI_ERR("panel manufacture is not specified\n");
	panel_version = of_get_property(of_node,
		"qcom,mdss-dsi-panel-version", NULL);
	if (!panel_version)
		DSI_ERR("panel version is not specified\n");

	backlight_manufacture = of_get_property(of_node,
		"qcom,mdss-dsi-backlight-manufacture", NULL);
	if (!backlight_manufacture)
		DSI_ERR("backlight manufacture is not specified\n");
	backlight_version = of_get_property(of_node,
		"qcom,mdss-dsi-backlight-version", NULL);
	if (!backlight_version)
		DSI_ERR("backlight version is not specified\n");

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-acl-cmd-index", &tmp);
	panel->acl_cmd_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-acl-mode-index", &tmp);
	panel->acl_mode_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-year-index", &tmp);
	panel->panel_year_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-mon-index", &tmp);
	panel->panel_mon_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-day-index", &tmp);
	panel->panel_day_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-hour-index", &tmp);
	panel->panel_hour_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-min-index", &tmp);
	panel->panel_min_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-sec-index", &tmp);
	panel->panel_sec_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-msec-high-index", &tmp);
	panel->panel_msec_high_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-seria-num-msec-low-index", &tmp);
	panel->panel_msec_low_index = (!rc ? tmp : 0);

	rc = of_property_read_u32(of_node, "qcom,mdss-dsi-panel-status-value-2", &tmp);
	panel->status_value = (!rc ? tmp : 0);

	panel->panel_mismatch_check =
		of_property_read_bool(of_node, "qcom,mdss-panel-mismatch-check");

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		panel->vsync_switch_gpio_level = 1;
		panel->cur_h_active = 1080;
	}
#endif /* (CONFIG_PXLW_IRIS) */

	DSI_DEBUG("%s end\n", __func__);
	return 0;
}

/*
 * The length of all the valid values to be checked should not be greater
 * than the length of returned data from read command.
 */
static bool
dsi_panel_parse_esd_check_valid_params(struct dsi_panel *panel, u32 count)
{
	int i;
	struct drm_panel_esd_config *config = &panel->esd_config;

	for (i = 0; i < count; ++i) {
		if (config->status_valid_params[i] >
				config->status_cmds_rlen[i]) {
			DSI_DEBUG("ignore valid params\n");
			return false;
		}
	}

	return true;
}

static bool dsi_panel_parse_esd_status_len(struct dsi_parser_utils *utils,
	char *prop_key, u32 **target, u32 cmd_cnt)
{
	int tmp;

	if (!utils->find_property(utils->data, prop_key, &tmp))
		return false;

	tmp /= sizeof(u32);
	if (tmp != cmd_cnt) {
		DSI_ERR("request property(%d) do not match cmd count(%d)\n",
				tmp, cmd_cnt);
		return false;
	}

	*target = kcalloc(tmp, sizeof(u32), GFP_KERNEL);
	if (IS_ERR_OR_NULL(*target)) {
		DSI_ERR("Error allocating memory for property\n");
		return false;
	}

	if (utils->read_u32_array(utils->data, prop_key, *target, tmp)) {
		DSI_ERR("cannot get values from dts\n");
		kfree(*target);
		*target = NULL;
		return false;
	}

	return true;
}

static void dsi_panel_esd_config_deinit(struct drm_panel_esd_config *esd_config)
{
	kfree(esd_config->status_buf);
	kfree(esd_config->return_buf);
	kfree(esd_config->status_value);
	kfree(esd_config->status_valid_params);
	kfree(esd_config->status_cmds_rlen);
	kfree(esd_config->status_cmd.cmds);
}

int dsi_panel_parse_esd_reg_read_configs(struct dsi_panel *panel)
{
	struct drm_panel_esd_config *esd_config;
	int rc = 0;
	u32 tmp;
	u32 i, status_len, *lenp;
	struct property *data;
	struct dsi_parser_utils *utils = &panel->utils;

	if (!panel) {
		DSI_ERR("Invalid Params\n");
		return -EINVAL;
	}

	esd_config = &panel->esd_config;
	if (!esd_config)
		return -EINVAL;

	dsi_panel_parse_cmd_sets_sub(&esd_config->status_cmd,
				DSI_CMD_SET_PANEL_STATUS, utils);
	if (!esd_config->status_cmd.count) {
		DSI_ERR("panel status command parsing failed\n");
		rc = -EINVAL;
		goto error;
	}

	if (!dsi_panel_parse_esd_status_len(utils,
		"qcom,mdss-dsi-panel-status-read-length",
			&panel->esd_config.status_cmds_rlen,
				esd_config->status_cmd.count)) {
		DSI_ERR("Invalid status read length\n");
		rc = -EINVAL;
		goto error1;
	}

	if (dsi_panel_parse_esd_status_len(utils,
		"qcom,mdss-dsi-panel-status-valid-params",
			&panel->esd_config.status_valid_params,
				esd_config->status_cmd.count)) {
		if (!dsi_panel_parse_esd_check_valid_params(panel,
					esd_config->status_cmd.count)) {
			rc = -EINVAL;
			goto error2;
		}
	}

	status_len = 0;
	lenp = esd_config->status_valid_params ?: esd_config->status_cmds_rlen;
	for (i = 0; i < esd_config->status_cmd.count; ++i)
		status_len += lenp[i];

	if (!status_len) {
		rc = -EINVAL;
		goto error2;
	}

	/*
	 * Some panel may need multiple read commands to properly
	 * check panel status. Do a sanity check for proper status
	 * value which will be compared with the value read by dsi
	 * controller during ESD check. Also check if multiple read
	 * commands are there then, there should be corresponding
	 * status check values for each read command.
	 */
	data = utils->find_property(utils->data,
			"qcom,mdss-dsi-panel-status-value", &tmp);
	tmp /= sizeof(u32);
	if (!IS_ERR_OR_NULL(data) && tmp != 0 && (tmp % status_len) == 0) {
		esd_config->groups = tmp / status_len;
	} else {
		DSI_ERR("error parse panel-status-value\n");
		rc = -EINVAL;
		goto error2;
	}

	esd_config->status_value =
		kzalloc(sizeof(u32) * status_len * esd_config->groups,
			GFP_KERNEL);
	if (!esd_config->status_value) {
		rc = -ENOMEM;
		goto error2;
	}

	esd_config->return_buf = kcalloc(status_len * esd_config->groups,
			sizeof(unsigned char), GFP_KERNEL);
	if (!esd_config->return_buf) {
		rc = -ENOMEM;
		goto error3;
	}

	esd_config->status_buf = kzalloc(SZ_4K, GFP_KERNEL);
	if (!esd_config->status_buf) {
		rc = -ENOMEM;
		goto error4;
	}

	rc = utils->read_u32_array(utils->data,
		"qcom,mdss-dsi-panel-status-value",
		esd_config->status_value, esd_config->groups * status_len);
	if (rc) {
		DSI_DEBUG("error reading panel status values\n");
		memset(esd_config->status_value, 0,
				esd_config->groups * status_len);
	}

	return 0;

error4:
	kfree(esd_config->return_buf);
error3:
	kfree(esd_config->status_value);
error2:
	kfree(esd_config->status_valid_params);
	kfree(esd_config->status_cmds_rlen);
error1:
	kfree(esd_config->status_cmd.cmds);
error:
	return rc;
}

static int dsi_panel_parse_esd_config(struct dsi_panel *panel)
{
	int rc = 0;
	const char *string;
	struct drm_panel_esd_config *esd_config;
	struct dsi_parser_utils *utils = &panel->utils;
	u8 *esd_mode = NULL;

	esd_config = &panel->esd_config;
	esd_config->status_mode = ESD_MODE_MAX;
	esd_config->esd_enabled = utils->read_bool(utils->data,
		"qcom,esd-check-enabled");

	if (!esd_config->esd_enabled)
		return 0;

	rc = utils->read_string(utils->data,
			"qcom,mdss-dsi-panel-status-check-mode", &string);
	if (!rc) {
		if (!strcmp(string, "bta_check")) {
			esd_config->status_mode = ESD_MODE_SW_BTA;
		} else if (!strcmp(string, "reg_read")) {
			esd_config->status_mode = ESD_MODE_REG_READ;
		} else if (!strcmp(string, "te_signal_check")) {
			if (panel->panel_mode == DSI_OP_CMD_MODE) {
				esd_config->status_mode = ESD_MODE_PANEL_TE;
			} else {
				DSI_ERR("TE-ESD not valid for video mode\n");
				rc = -EINVAL;
				goto error;
			}
		} else {
			DSI_ERR("No valid panel-status-check-mode string\n");
			rc = -EINVAL;
			goto error;
		}
	} else {
		DSI_DEBUG("status check method not defined!\n");
		rc = -EINVAL;
		goto error;
	}

	if (panel->esd_config.status_mode == ESD_MODE_REG_READ) {
		rc = dsi_panel_parse_esd_reg_read_configs(panel);
		if (rc) {
			DSI_ERR("failed to parse esd reg read mode params, rc=%d\n",
						rc);
			goto error;
		}
		esd_mode = "register_read";
	} else if (panel->esd_config.status_mode == ESD_MODE_SW_BTA) {
		esd_mode = "bta_trigger";
	} else if (panel->esd_config.status_mode ==  ESD_MODE_PANEL_TE) {
		esd_mode = "te_check";
	}

	DSI_DEBUG("ESD enabled with mode: %s\n", esd_mode);

	return 0;

error:
	panel->esd_config.esd_enabled = false;
	return rc;
}

static void dsi_panel_update_util(struct dsi_panel *panel,
				  struct device_node *parser_node)
{
	struct dsi_parser_utils *utils = &panel->utils;

	if (parser_node) {
		*utils = *dsi_parser_get_parser_utils();
		utils->data = parser_node;

		DSI_DEBUG("switching to parser APIs\n");

		goto end;
	}

	*utils = *dsi_parser_get_of_utils();
	utils->data = panel->panel_of_node;
end:
	utils->node = panel->panel_of_node;
}

static int dsi_panel_vm_stub(struct dsi_panel *panel)
{
	return 0;
}

static void dsi_panel_setup_vm_ops(struct dsi_panel *panel, bool trusted_vm_env)
{
	if (trusted_vm_env) {
		panel->panel_ops.pinctrl_init = dsi_panel_vm_stub;
		panel->panel_ops.gpio_request = dsi_panel_vm_stub;
		panel->panel_ops.pinctrl_deinit = dsi_panel_vm_stub;
		panel->panel_ops.gpio_release = dsi_panel_vm_stub;
		panel->panel_ops.bl_register = dsi_panel_vm_stub;
		panel->panel_ops.bl_unregister = dsi_panel_vm_stub;
		panel->panel_ops.parse_gpios = dsi_panel_vm_stub;
	} else {
		panel->panel_ops.pinctrl_init = dsi_panel_pinctrl_init;
		panel->panel_ops.gpio_request = dsi_panel_gpio_request;
		panel->panel_ops.pinctrl_deinit = dsi_panel_pinctrl_deinit;
		panel->panel_ops.gpio_release = dsi_panel_gpio_release;
		panel->panel_ops.bl_register = dsi_panel_bl_register;
		panel->panel_ops.bl_unregister = dsi_panel_bl_unregister;
		panel->panel_ops.parse_gpios = dsi_panel_parse_gpios;
	}
}

struct dsi_panel *dsi_panel_get(struct device *parent,
				struct device_node *of_node,
				struct device_node *parser_node,
				const char *type,
				int topology_override,
				bool trusted_vm_env)
{
	struct dsi_panel *panel;
	struct dsi_parser_utils *utils;
	const char *panel_physical_type;
	int rc = 0;

	panel = kzalloc(sizeof(*panel), GFP_KERNEL);
	if (!panel)
		return ERR_PTR(-ENOMEM);

	dsi_panel_setup_vm_ops(panel, trusted_vm_env);

	panel->panel_of_node = of_node;
	panel->parent = parent;
	panel->type = type;

	dsi_panel_update_util(panel, parser_node);
	utils = &panel->utils;

	panel->name = utils->get_property(utils->data,
				"qcom,mdss-dsi-panel-name", NULL);
	if (strcmp(panel->name, "samsung dsc cmd mode oneplus dsi panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_S6E3HC2;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_S6E3HC2");
	}
	else if (strcmp(panel->name, "samsung sofef03f_m fhd cmd mode dsc dsi panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_SOFEF03F_M;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_SOFEF03F_M");
	}
	else if (strcmp(panel->name, "samsung s6e3fc2x01 cmd mode dsi panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_S6E3FC2X01;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_S6E3FC2X01");
	}
	else if (strcmp(panel->name, "samsung ana6705 fhd cmd mode dsc dsi panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_ANA6705;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_ANA6705");
	}
	else if (strcmp(panel->name, "samsung ana6706 dsc cmd mode panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_ANA6706;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_ANA6706");
	}
	else if (strcmp(panel->name, "samsung amb655x fhd cmd mode dsc dsi panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_AMB655XL;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_AMB655XL");
	}
	else if (strcmp(panel->name, "samsung amb655xl08 fhd cmd mode dsc dsi panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_AMB655XL08;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_AMB655XL08");
	}
	else if (strcmp(panel->name, "samsung amb670yf01 dsc cmd mode panel") == 0) {
		dsi_panel_name = DSI_PANEL_SAMSUNG_AMB670YF01;
		DSI_ERR("Dsi panel name is DSI_PANEL_SAMSUNG_AMB670YF01");
	}
	else if (!panel->name)
		panel->name = DSI_PANEL_DEFAULT_LABEL;

#if defined(CONFIG_PXLW_IRIS)
	iris_query_capability(panel);
#endif

	if (parse_count == 0) {
		rc = oplus_panel_parse_msd_aod_config(panel);
		parse_count = 1;
		if (rc) {
			DSI_ERR("failed to parse msd aod config, rc=%d\n", rc);
			if (rc == -EPROBE_DEFER)
				goto error;
		}
	}

	/*
	 * Set panel type to LCD as default.
	 */
	panel->panel_type = DSI_DISPLAY_PANEL_TYPE_LCD;
	panel_physical_type = utils->get_property(utils->data,
				"qcom,mdss-dsi-panel-physical-type", NULL);
	if (panel_physical_type && !strcmp(panel_physical_type, "oled"))
		panel->panel_type = DSI_DISPLAY_PANEL_TYPE_OLED;
	rc = dsi_panel_parse_host_config(panel);
	if (rc) {
		DSI_ERR("failed to parse host configuration, rc=%d\n",
				rc);
		goto error;
	}

	rc = dsi_panel_parse_panel_mode(panel);
	if (rc) {
		DSI_ERR("failed to parse panel mode configuration, rc=%d\n",
				rc);
		goto error;
	}

	rc = dsi_panel_parse_dfps_caps(panel);
	if (rc)
		DSI_ERR("failed to parse dfps configuration, rc=%d\n", rc);

	rc = dsi_panel_parse_qsync_caps(panel, of_node);
	if (rc)
		DSI_DEBUG("failed to parse qsync features, rc=%d\n", rc);

	rc = dsi_panel_parse_dyn_clk_caps(panel);
	if (rc)
		DSI_ERR("failed to parse dynamic clk config, rc=%d\n", rc);

	rc = dsi_panel_parse_phy_props(panel);
	if (rc) {
		DSI_ERR("failed to parse panel physical dimension, rc=%d\n",
				rc);
		goto error;
	}

	rc = panel->panel_ops.parse_gpios(panel);
	if (rc) {
		DSI_ERR("failed to parse panel gpios, rc=%d\n", rc);
		goto error;
	}

	rc = dsi_panel_parse_tlmm_gpio(panel);
	if (rc) {
		DSI_ERR("failed to parse panel tlmm gpios, rc=%d\n", rc);
		goto error;
	}

	rc = dsi_panel_parse_power_cfg(panel);
	if (rc)
		DSI_ERR("failed to parse power config, rc=%d\n", rc);

	rc = dsi_panel_parse_bl_config(panel);
	if (rc) {
		DSI_ERR("failed to parse backlight config, rc=%d\n", rc);
		if (rc == -EPROBE_DEFER)
			goto error;
	}

	rc = dsi_panel_parse_misc_features(panel);
	if (rc)
		DSI_ERR("failed to parse misc features, rc=%d\n", rc);

	rc = dsi_panel_parse_hdr_config(panel);
	if (rc)
		DSI_ERR("failed to parse hdr config, rc=%d\n", rc);

	rc = dsi_panel_parse_oem_config(panel, of_node);
	if (rc)
		DSI_DEBUG("failed to get oem config, rc=%d\n", rc);

	rc = dsi_panel_get_mode_count(panel);
	if (rc) {
		DSI_ERR("failed to get mode count, rc=%d\n", rc);
		goto error;
	}

	rc = dsi_panel_parse_dms_info(panel);
	if (rc)
		DSI_DEBUG("failed to get dms info, rc=%d\n", rc);

	rc = dsi_panel_parse_esd_config(panel);
	if (rc)
		DSI_DEBUG("failed to parse esd config, rc=%d\n", rc);

	rc = dsi_panel_vreg_get(panel);
	if (rc) {
		DSI_ERR("[%s] failed to get panel regulators, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

	panel->power_mode = SDE_MODE_DPMS_OFF;
	drm_panel_init(&panel->drm_panel);
	panel->drm_panel.dev = &panel->mipi_device.dev;
	panel->mipi_device.dev.of_node = of_node;
	printk("enter drm_panel_add, name is %s\n", of_node->name);
	rc = drm_panel_add(&panel->drm_panel);
	if (rc)
		goto error_vreg_put;
	dsi_panel_parse_oplus_config(panel);

	mutex_init(&panel->panel_lock);

	return panel;
error_vreg_put:
	(void)dsi_panel_vreg_put(panel);
error:
	kfree(panel);
	return ERR_PTR(rc);
}

void dsi_panel_put(struct dsi_panel *panel)
{
	drm_panel_remove(&panel->drm_panel);

	/* free resources allocated for ESD check */
	dsi_panel_esd_config_deinit(&panel->esd_config);

	kfree(panel);
}

int dsi_panel_drv_init(struct dsi_panel *panel,
		       struct mipi_dsi_host *host)
{
	int rc = 0;
	struct mipi_dsi_device *dev;

	if (!panel || !host) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	dev = &panel->mipi_device;

	dev->host = host;
	/*
	 * We dont have device structure since panel is not a device node.
	 * When using drm panel framework, the device is probed when the host is
	 * create.
	 */
	dev->channel = 0;
	dev->lanes = 4;

	panel->host = host;

	rc = panel->panel_ops.pinctrl_init(panel);
	if (rc) {
		DSI_ERR("[%s] failed to init pinctrl, rc=%d\n",
				panel->name, rc);
		goto exit;
	}

	rc = panel->panel_ops.gpio_request(panel);
	if (rc) {
		DSI_ERR("[%s] failed to request gpios, rc=%d\n", panel->name,
		       rc);
#if defined(CONFIG_PXLW_IRIS)
		if (iris_is_chip_supported()) {
			if (!strcmp(panel->type, "primary"))
				goto error_pinctrl_deinit;
			rc = 0;
		} else
#endif
		goto error_pinctrl_deinit;
	}

	rc = panel->panel_ops.bl_register(panel);
	if (rc) {
		if (rc != -EPROBE_DEFER)
			DSI_ERR("[%s] failed to register backlight, rc=%d\n",
			       panel->name, rc);
		goto error_gpio_release;
	}

	goto exit;

error_gpio_release:
	(void)dsi_panel_gpio_release(panel);
error_pinctrl_deinit:
	(void)dsi_panel_pinctrl_deinit(panel);
exit:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_drv_deinit(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = panel->panel_ops.bl_unregister(panel);
	if (rc)
		DSI_ERR("[%s] failed to unregister backlight, rc=%d\n",
		       panel->name, rc);

	rc = panel->panel_ops.gpio_release(panel);
	if (rc)
		DSI_ERR("[%s] failed to release gpios, rc=%d\n", panel->name,
		       rc);

	rc = panel->panel_ops.pinctrl_deinit(panel);
	if (rc)
		DSI_ERR("[%s] failed to deinit gpios, rc=%d\n", panel->name,
		       rc);

	rc = dsi_panel_vreg_put(panel);
	if (rc)
		DSI_ERR("[%s] failed to put regs, rc=%d\n", panel->name, rc);

	kfree(panel->tlmm_gpio);
	panel->host = NULL;
	memset(&panel->mipi_device, 0x0, sizeof(panel->mipi_device));

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_validate_mode(struct dsi_panel *panel,
			    struct dsi_display_mode *mode)
{
	return 0;
}

static int dsi_panel_get_max_res_count(struct dsi_parser_utils *utils,
	struct device_node *node, u32 *dsc_count, u32 *lm_count)
{
	const char *compression;
	u32 *array = NULL, top_count, len, i;
	int rc = -EINVAL;
	bool dsc_enable = false;

	*dsc_count = 0;
	*lm_count = 0;
	compression = utils->get_property(node, "qcom,compression-mode", NULL);
	if (compression && !strcmp(compression, "dsc"))
		dsc_enable = true;

	len = utils->count_u32_elems(node, "qcom,display-topology");
	if (len <= 0 || len % TOPOLOGY_SET_LEN ||
			len > (TOPOLOGY_SET_LEN * MAX_TOPOLOGY))
		return rc;

	top_count = len / TOPOLOGY_SET_LEN;

	array = kcalloc(len, sizeof(u32), GFP_KERNEL);
	if (!array)
		return -ENOMEM;

	rc = utils->read_u32_array(node, "qcom,display-topology", array, len);
	if (rc) {
		DSI_ERR("unable to read the display topologies, rc = %d\n", rc);
		goto read_fail;
	}

	for (i = 0; i < top_count; i++) {
		*lm_count = max(*lm_count, array[i * TOPOLOGY_SET_LEN]);
		if (dsc_enable)
			*dsc_count = max(*dsc_count,
					array[i * TOPOLOGY_SET_LEN + 1]);
	}

read_fail:
	kfree(array);
	return 0;
}

int dsi_panel_get_mode_count(struct dsi_panel *panel)
{
	const u32 SINGLE_MODE_SUPPORT = 1;
	struct dsi_parser_utils *utils;
	struct device_node *timings_np, *child_np;
	int num_dfps_rates, num_bit_clks;
	int num_video_modes = 0, num_cmd_modes = 0;
	int count, rc = 0;
	u32 dsc_count = 0, lm_count = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	utils = &panel->utils;

	panel->num_timing_nodes = 0;

	timings_np = utils->get_child_by_name(utils->data,
			"qcom,mdss-dsi-display-timings");
	if (!timings_np && !panel->host_config.ext_bridge_mode) {
		DSI_ERR("no display timing nodes defined\n");
		rc = -EINVAL;
		goto error;
	}

	count = utils->get_child_count(timings_np);
	if ((!count && !panel->host_config.ext_bridge_mode) ||
		count > DSI_MODE_MAX) {
		DSI_ERR("invalid count of timing nodes: %d\n", count);
		rc = -EINVAL;
		goto error;
	}

	/* No multiresolution support is available for video mode panels.
	 * Multi-mode is supported for video mode during POMS is enabled.
	 */
	if (panel->panel_mode != DSI_OP_CMD_MODE &&
		!panel->host_config.ext_bridge_mode &&
		!panel->panel_mode_switch_enabled)
		count = SINGLE_MODE_SUPPORT;

	panel->num_timing_nodes = count;
	dsi_for_each_child_node(timings_np, child_np) {
		if (utils->read_bool(child_np, "qcom,mdss-dsi-video-mode"))
			num_video_modes++;
		else if (utils->read_bool(child_np,
					"qcom,mdss-dsi-cmd-mode"))
			num_cmd_modes++;
		else if (panel->panel_mode == DSI_OP_VIDEO_MODE)
			num_video_modes++;
		else if (panel->panel_mode == DSI_OP_CMD_MODE)
			num_cmd_modes++;

		dsi_panel_get_max_res_count(utils, child_np,
				&dsc_count, &lm_count);
		panel->dsc_count = max(dsc_count, panel->dsc_count);
		panel->lm_count = max(lm_count, panel->lm_count);
	}

	num_dfps_rates = !panel->dfps_caps.dfps_support ? 1 :
					panel->dfps_caps.dfps_list_len;

	num_bit_clks = !panel->dyn_clk_caps.dyn_clk_support ? 1 :
					panel->dyn_clk_caps.bit_clk_list_len;

	/*
	 * Inflate num_of_modes by fps and bit clks in dfps.
	 * Single command mode for video mode panels supporting
	 * panel operating mode switch.
	 */
	num_video_modes = num_video_modes * num_bit_clks * num_dfps_rates;

	if ((panel->panel_mode == DSI_OP_VIDEO_MODE) &&
			(panel->panel_mode_switch_enabled))
		num_cmd_modes  = 1;
	else
		num_cmd_modes = num_cmd_modes * num_bit_clks;

	panel->num_display_modes = num_video_modes + num_cmd_modes;

error:
	return rc;
}

int dsi_panel_get_phy_props(struct dsi_panel *panel,
			    struct dsi_panel_phy_props *phy_props)
{
	int rc = 0;

	if (!panel || !phy_props) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	memcpy(phy_props, &panel->phy_props, sizeof(*phy_props));
	return rc;
}

int dsi_panel_get_dfps_caps(struct dsi_panel *panel,
			    struct dsi_dfps_capabilities *dfps_caps)
{
	int rc = 0;

	if (!panel || !dfps_caps) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	memcpy(dfps_caps, &panel->dfps_caps, sizeof(*dfps_caps));
	return rc;
}

void dsi_panel_put_mode(struct dsi_display_mode *mode)
{
	int i;

	if (!mode->priv_info)
		return;

	for (i = 0; i < DSI_CMD_SET_MAX; i++) {
		dsi_panel_destroy_cmd_packets(&mode->priv_info->cmd_sets[i]);
		dsi_panel_dealloc_cmd_packets(&mode->priv_info->cmd_sets[i]);
	}

	kfree(mode->priv_info);
}

void dsi_panel_calc_dsi_transfer_time(struct dsi_host_common_cfg *config,
		struct dsi_display_mode *mode, u32 frame_threshold_us)
{
	u32 frame_time_us, nslices;
	u64 min_bitclk_hz, total_active_pixels, bits_per_line, pclk_rate_hz,
		dsi_transfer_time_us, pixel_clk_khz;
	struct msm_display_dsc_info *dsc = mode->timing.dsc;
	struct dsi_mode_info *timing = &mode->timing;
	struct dsi_display_mode *display_mode;
	u32 jitter_numer, jitter_denom, prefill_lines;
	u32 min_threshold_us, prefill_time_us, max_transfer_us;
	u16 bpp;

	/* Packet overlead in bits,2 bytes header + 2 bytes checksum
	 * + 1 byte dcs data command.
	*/
	const u32 packet_overhead = 56;

	display_mode = container_of(timing, struct dsi_display_mode, timing);

	jitter_numer = display_mode->priv_info->panel_jitter_numer;
	jitter_denom = display_mode->priv_info->panel_jitter_denom;

	frame_time_us = mult_frac(1000, 1000, (timing->refresh_rate));

	if (timing->refresh_rate >= 120)
		frame_threshold_us = HIGH_REFRESH_RATE_THRESHOLD_TIME_US;

	if (timing->dsc_enabled) {
		nslices = (timing->h_active)/(dsc->config.slice_width);
		/* (slice width x bit-per-pixel + packet overhead) x
		 * number of slices x height x fps / lane
		 */
		bpp = DSC_BPP(dsc->config);
		bits_per_line = ((dsc->config.slice_width * bpp) +
				packet_overhead) * nslices;
		bits_per_line = bits_per_line / (config->num_data_lanes);

		min_bitclk_hz = (bits_per_line * timing->v_active *
					timing->refresh_rate);
	} else {
		total_active_pixels = ((dsi_h_active_dce(timing)
					* timing->v_active));
		/* calculate the actual bitclk needed to transfer the frame */
		min_bitclk_hz = (total_active_pixels * (timing->refresh_rate) *
				(config->bpp));
		do_div(min_bitclk_hz, config->num_data_lanes);
	}

	timing->min_dsi_clk_hz = min_bitclk_hz;

	min_threshold_us = mult_frac(frame_time_us,
			jitter_numer, (jitter_denom * 100));
	/*
	 * Increase the prefill_lines proportionately as recommended
	 * 35lines for 60fps, 52 for 90fps, 70lines for 120fps.
	 */
	prefill_lines = mult_frac(MIN_PREFILL_LINES,
			timing->refresh_rate, 60);

	prefill_time_us = mult_frac(frame_time_us, prefill_lines,
			(timing->v_active));

	/*
	 * Threshold is sum of panel jitter time, prefill line time
	 * plus 64usec buffer time.
	 */
	min_threshold_us = min_threshold_us + 64 + prefill_time_us;

	DSI_DEBUG("min threshold time=%d\n", min_threshold_us);

	if (timing->clk_rate_hz) {
		/* adjust the transfer time proportionately for bit clk*/
		dsi_transfer_time_us = frame_time_us * min_bitclk_hz;
		do_div(dsi_transfer_time_us, timing->clk_rate_hz);
		timing->dsi_transfer_time_us = dsi_transfer_time_us;

	} else if (mode->priv_info->mdp_transfer_time_us) {
		max_transfer_us = frame_time_us - min_threshold_us;
		mode->priv_info->mdp_transfer_time_us = min(
				mode->priv_info->mdp_transfer_time_us,
				max_transfer_us);
		timing->dsi_transfer_time_us =
			mode->priv_info->mdp_transfer_time_us;
	} else {
		if (min_threshold_us > frame_threshold_us)
			frame_threshold_us = min_threshold_us;

		timing->dsi_transfer_time_us = frame_time_us -
			frame_threshold_us;
	}

	timing->mdp_transfer_time_us = timing->dsi_transfer_time_us;

	/* Force update mdp xfer time to hal,if clk and mdp xfer time is set */
	if (mode->priv_info->mdp_transfer_time_us && timing->clk_rate_hz) {
		timing->mdp_transfer_time_us =
			mode->priv_info->mdp_transfer_time_us;
	}

	/* Calculate pclk_khz to update modeinfo */
	pclk_rate_hz =  min_bitclk_hz * frame_time_us;
	do_div(pclk_rate_hz, timing->dsi_transfer_time_us);

	pixel_clk_khz = pclk_rate_hz * config->num_data_lanes;
	do_div(pixel_clk_khz, config->bpp);
	display_mode->pixel_clk_khz = pixel_clk_khz;

	display_mode->pixel_clk_khz =  display_mode->pixel_clk_khz / 1000;
}


int dsi_panel_get_mode(struct dsi_panel *panel,
			u32 index, struct dsi_display_mode *mode,
			int topology_override)
{
	struct device_node *timings_np, *child_np;
	struct dsi_parser_utils *utils;
	struct dsi_display_mode_priv_info *prv_info;
	u32 child_idx = 0;
	int rc = 0, num_timings;
	int traffic_mode;
	int panel_mode;
	void *utils_data = NULL;

	if (!panel || !mode) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	utils = &panel->utils;

	mode->priv_info = kzalloc(sizeof(*mode->priv_info), GFP_KERNEL);
	if (!mode->priv_info) {
		rc = -ENOMEM;
		goto done;
	}

	prv_info = mode->priv_info;

	timings_np = utils->get_child_by_name(utils->data,
		"qcom,mdss-dsi-display-timings");
	if (!timings_np) {
		DSI_ERR("no display timing nodes defined\n");
		rc = -EINVAL;
		goto parse_fail;
	}

	num_timings = utils->get_child_count(timings_np);
	if (!num_timings || num_timings > DSI_MODE_MAX) {
		DSI_ERR("invalid count of timing nodes: %d\n", num_timings);
		rc = -EINVAL;
		goto parse_fail;
	}

	utils_data = utils->data;
	traffic_mode = panel->video_config.traffic_mode;
	panel_mode = panel->panel_mode;

	dsi_for_each_child_node(timings_np, child_np) {
		if (index != child_idx++)
			continue;

		utils->data = child_np;

		rc = dsi_panel_parse_timing(&mode->timing, utils);
		if (rc) {
			DSI_ERR("failed to parse panel timing, rc=%d\n", rc);
			goto parse_fail;
		}

		rc = dsi_panel_parse_dsc_params(mode, utils);
		if (rc) {
			DSI_ERR("failed to parse dsc params, rc=%d\n", rc);
			goto parse_fail;
		}

		rc = dsi_panel_parse_vdc_params(mode, utils, traffic_mode,
			panel_mode);
		if (rc) {
			DSI_ERR("failed to parse vdc params, rc=%d\n", rc);
			goto parse_fail;
		}

		rc = dsi_panel_parse_topology(prv_info, utils,
				topology_override);
		if (rc) {
			DSI_ERR("failed to parse panel topology, rc=%d\n", rc);
			goto parse_fail;
		}

		rc = dsi_panel_parse_cmd_sets(prv_info, utils);
		if (rc) {
			DSI_ERR("failed to parse command sets, rc=%d\n", rc);
			goto parse_fail;
		}

		rc = dsi_panel_parse_jitter_config(mode, utils);
		if (rc)
			DSI_ERR(
			"failed to parse panel jitter config, rc=%d\n", rc);

		rc = dsi_panel_parse_phy_timing(mode, utils);
		if (rc) {
			DSI_ERR(
			"failed to parse panel phy timings, rc=%d\n", rc);
			goto parse_fail;
		}

		rc = dsi_panel_parse_partial_update_caps(mode, utils);
		if (rc)
			DSI_ERR("failed to partial update caps, rc=%d\n", rc);

		if (panel->panel_mode_switch_enabled) {
			rc = dsi_panel_parse_panel_mode_caps(mode, utils);
			if (rc) {
				rc = 0;
				mode->panel_mode = panel->panel_mode;
				DSI_INFO(
				"POMS: panel mode isn't specified in timing[%d]\n",
				child_idx);
			}
		} else {
			mode->panel_mode = panel->panel_mode;
		}

		mode->splash_dms = of_property_read_bool(child_np,
				"qcom,mdss-dsi-splash-dms-switch-to-this-timing");

#ifdef CONFIG_PXLW_IRIS
		if (iris_is_chip_supported()) {
			dsi_panel_parse_adfr(mode, utils);
		}
#endif
	}
	goto done;

parse_fail:
	kfree(mode->priv_info);
	mode->priv_info = NULL;
done:
	utils->data = utils_data;
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_get_host_cfg_for_mode(struct dsi_panel *panel,
				    struct dsi_display_mode *mode,
				    struct dsi_host_config *config)
{
	int rc = 0;
	struct dsi_dyn_clk_caps *dyn_clk_caps = &panel->dyn_clk_caps;

	if (!panel || !mode || !config) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	config->panel_mode = panel->panel_mode;
	memcpy(&config->common_config, &panel->host_config,
	       sizeof(config->common_config));

	if (panel->panel_mode == DSI_OP_VIDEO_MODE) {
		memcpy(&config->u.video_engine, &panel->video_config,
		       sizeof(config->u.video_engine));
	} else {
		memcpy(&config->u.cmd_engine, &panel->cmd_config,
		       sizeof(config->u.cmd_engine));
	}

	memcpy(&config->video_timing, &mode->timing,
	       sizeof(config->video_timing));
	config->video_timing.mdp_transfer_time_us =
			mode->priv_info->mdp_transfer_time_us;
	config->video_timing.dsc_enabled = mode->priv_info->dsc_enabled;
	config->video_timing.dsc = &mode->priv_info->dsc;

	config->video_timing.vdc_enabled = mode->priv_info->vdc_enabled;
	config->video_timing.vdc = &mode->priv_info->vdc;

	if (dyn_clk_caps->dyn_clk_support)
		config->bit_clk_rate_hz_override = mode->timing.clk_rate_hz;
	else
		config->bit_clk_rate_hz_override = mode->priv_info->clk_rate_hz;

	config->esc_clk_rate_hz = 19200000;
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_pre_prepare(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	if (panel->err_flag_status == true) {
		DSI_ERR("no need to power on when err flag irq comes\n");
		panel->err_flag_status = false;
		return rc;
	}

	mutex_lock(&panel->panel_lock);

#if defined(CONFIG_PXLW_IRIS)
	iris_power_on(panel);
#endif
	/* If LP11_INIT is set, panel will be powered up during prepare() */
	if (panel->lp11_init)
		goto error;

	if (gpio_is_valid(panel->tp1v8_gpio)) {
		rc = gpio_direction_output(panel->tp1v8_gpio, 1);
		DSI_ERR("enable tp1v8 gpio\n");
		if (rc)
			DSI_ERR("unable to set dir for tp1v8 gpio rc=%d\n", rc);
	}

	rc = dsi_panel_power_on(panel);
	if (rc) {
		DSI_ERR("[%s] panel power on failed, rc=%d\n", panel->name, rc);
		if (gpio_is_valid(panel->tp1v8_gpio))
			gpio_set_value(panel->tp1v8_gpio, 0);
#if defined(CONFIG_PXLW_IRIS)
		if (iris_is_chip_supported()) {
			if (iris_vdd_valid())
				iris_disable_vdd();
			else
				iris_control_pwr_regulator(false);
		}
#endif
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_update_pps(struct dsi_panel *panel)
{
	int rc = 0;
	struct dsi_panel_cmd_set *set = NULL;
	struct dsi_display_mode_priv_info *priv_info = NULL;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_dual_supported() && panel->is_secondary)
		return rc;
#endif
	mutex_lock(&panel->panel_lock);

	priv_info = panel->cur_mode->priv_info;

	set = &priv_info->cmd_sets[DSI_CMD_SET_PPS];

	if (priv_info->dsc_enabled)
		dsi_dsc_create_pps_buf_cmd(&priv_info->dsc,
				panel->dce_pps_cmd, 0,
				DSI_CMD_PPS_SIZE - DSI_CMD_PPS_HDR_SIZE);
	else if (priv_info->vdc_enabled)
		dsi_vdc_create_pps_buf_cmd(&priv_info->vdc,
				panel->dce_pps_cmd, 0,
				DSI_CMD_PPS_SIZE - DSI_CMD_PPS_HDR_SIZE);

	if (priv_info->dsc_enabled || priv_info->vdc_enabled) {
		rc = dsi_panel_create_cmd_packets(panel->dce_pps_cmd,
				DSI_CMD_PPS_SIZE, 1, set->cmds);
		if (rc) {
			DSI_ERR("failed to create cmd packets, rc=%d\n", rc);
			goto error;
		}
	}
#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported() && iris_is_pt_mode(panel))
		rc = iris_pt_send_panel_cmd(panel, &(panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_PPS]));
	else
#endif
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_PPS);
	if (rc) {
		DSI_ERR("[%s] failed to send DSI_CMD_SET_PPS cmds, rc=%d\n",
			panel->name, rc);
	}

	dsi_panel_destroy_cmd_packets(set);
error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_set_lp1(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	if (!panel->panel_initialized)
		goto exit;

	/*
	 * Consider LP1->LP2->LP1.
	 * If the panel is already in LP mode, do not need to
	 * set the regulator.
	 * IBB and AB power mode would be set at the same time
	 * in PMIC driver, so we only call ibb setting that is enough.
	 */
	if (dsi_panel_is_type_oled(panel) &&
		panel->power_mode != SDE_MODE_DPMS_LP2)
		dsi_pwr_panel_regulator_mode_set(&panel->power_info,
			"ibb", REGULATOR_MODE_IDLE);
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_LP1);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_LP1 cmd, rc=%d\n",
		       panel->name, rc);
exit:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_set_lp2(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	if (!panel->panel_initialized)
		goto exit;

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_LP2);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_LP2 cmd, rc=%d\n",
		       panel->name, rc);
exit:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_set_nolp(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);
	if (!panel->panel_initialized)
		goto exit;

	/*
	 * Consider about LP1->LP2->NOLP.
	 */
	if (dsi_panel_is_type_oled(panel) &&
	    (panel->power_mode == SDE_MODE_DPMS_LP1 ||
	     panel->power_mode == SDE_MODE_DPMS_LP2))
		dsi_pwr_panel_regulator_mode_set(&panel->power_info,
			"ibb", REGULATOR_MODE_NORMAL);
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NOLP);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_NOLP cmd, rc=%d\n",
		       panel->name, rc);
exit:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_prepare(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	if (panel->lp11_init) {
		rc = dsi_panel_power_on(panel);
		if (rc) {
			DSI_ERR("[%s] panel power on failed, rc=%d\n",
			       panel->name, rc);
			goto error;
		}
	}

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_PRE_ON);
	if (rc) {
		DSI_ERR("[%s] failed to send DSI_CMD_SET_PRE_ON cmds, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

static int dsi_panel_roi_prepare_dcs_cmds(struct dsi_panel_cmd_set *set,
		struct dsi_rect *roi, int ctrl_idx, int unicast)
{
	static const int ROI_CMD_LEN = 5;

	int rc = 0;

	/* DTYPE_DCS_LWRITE */
	char *caset, *paset;

	set->cmds = NULL;

	caset = kzalloc(ROI_CMD_LEN, GFP_KERNEL);
	if (!caset) {
		rc = -ENOMEM;
		goto exit;
	}
	caset[0] = 0x2a;
	caset[1] = (roi->x & 0xFF00) >> 8;
	caset[2] = roi->x & 0xFF;
	caset[3] = ((roi->x - 1 + roi->w) & 0xFF00) >> 8;
	caset[4] = (roi->x - 1 + roi->w) & 0xFF;

	paset = kzalloc(ROI_CMD_LEN, GFP_KERNEL);
	if (!paset) {
		rc = -ENOMEM;
		goto error_free_mem;
	}
	paset[0] = 0x2b;
	paset[1] = (roi->y & 0xFF00) >> 8;
	paset[2] = roi->y & 0xFF;
	paset[3] = ((roi->y - 1 + roi->h) & 0xFF00) >> 8;
	paset[4] = (roi->y - 1 + roi->h) & 0xFF;

	set->type = DSI_CMD_SET_ROI;
	set->state = DSI_CMD_SET_STATE_LP;
	set->count = 2; /* send caset + paset together */
	set->cmds = kcalloc(set->count, sizeof(*set->cmds), GFP_KERNEL);
	if (!set->cmds) {
		rc = -ENOMEM;
		goto error_free_mem;
	}
	set->cmds[0].msg.channel = 0;
	set->cmds[0].msg.type = MIPI_DSI_DCS_LONG_WRITE;
	set->cmds[0].msg.flags = unicast ? MIPI_DSI_MSG_UNICAST : 0;
	set->cmds[0].msg.ctrl = unicast ? ctrl_idx : 0;
	set->cmds[0].msg.tx_len = ROI_CMD_LEN;
	set->cmds[0].msg.tx_buf = caset;
	set->cmds[0].msg.rx_len = 0;
	set->cmds[0].msg.rx_buf = 0;
	set->cmds[0].msg.wait_ms = 0;
	set->cmds[0].last_command = 0;
	set->cmds[0].post_wait_ms = 0;

	set->cmds[1].msg.channel = 0;
	set->cmds[1].msg.type = MIPI_DSI_DCS_LONG_WRITE;
	set->cmds[1].msg.flags = unicast ? MIPI_DSI_MSG_UNICAST : 0;
	set->cmds[1].msg.ctrl = unicast ? ctrl_idx : 0;
	set->cmds[1].msg.tx_len = ROI_CMD_LEN;
	set->cmds[1].msg.tx_buf = paset;
	set->cmds[1].msg.rx_len = 0;
	set->cmds[1].msg.rx_buf = 0;
	set->cmds[1].msg.wait_ms = 0;
	set->cmds[1].last_command = 1;
	set->cmds[1].post_wait_ms = 0;

	goto exit;

error_free_mem:
	kfree(caset);
	kfree(paset);
	kfree(set->cmds);

exit:
	return rc;
}

int dsi_panel_send_qsync_on_dcs(struct dsi_panel *panel,
		int ctrl_idx)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

#ifdef CONFIG_PXLW_IRIS
	DSI_INFO("ctrl:%d qsync on\n", ctrl_idx);
	SDE_ATRACE_INT("qsync_mode_cmd", 1);
#else
	DSI_DEBUG("ctrl:%d qsync on\n", ctrl_idx);
#endif
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_QSYNC_ON);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_QSYNC_ON cmds rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_send_qsync_off_dcs(struct dsi_panel *panel,
		int ctrl_idx)
{
	int rc = 0;

#ifdef CONFIG_PXLW_IRIS
	if (!panel || !panel->cur_mode) {
#else
	if (!panel) {
#endif
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

#ifdef CONFIG_PXLW_IRIS
	DSI_INFO("ctrl:%d qsync off\n", ctrl_idx);
	SDE_ATRACE_INT("qsync_mode_cmd", 0);
	SDE_ATRACE_INT("oplus_adfr_qsync_mode_minfps_cmd", panel->cur_mode->timing.refresh_rate);
#else
	DSI_DEBUG("ctrl:%d qsync off\n", ctrl_idx);
#endif
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_QSYNC_OFF);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_QSYNC_OFF cmds rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_send_roi_dcs(struct dsi_panel *panel, int ctrl_idx,
		struct dsi_rect *roi)
{
	int rc = 0;
	struct dsi_panel_cmd_set *set;
	struct dsi_display_mode_priv_info *priv_info;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	priv_info = panel->cur_mode->priv_info;
	set = &priv_info->cmd_sets[DSI_CMD_SET_ROI];

	rc = dsi_panel_roi_prepare_dcs_cmds(set, roi, ctrl_idx, true);
	if (rc) {
		DSI_ERR("[%s] failed to prepare DSI_CMD_SET_ROI cmds, rc=%d\n",
				panel->name, rc);
		return rc;
	}
	DSI_DEBUG("[%s] send roi x %d y %d w %d h %d\n", panel->name,
			roi->x, roi->y, roi->w, roi->h);
	SDE_EVT32(roi->x, roi->y, roi->w, roi->h);

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ROI);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_ROI cmds, rc=%d\n",
				panel->name, rc);

	mutex_unlock(&panel->panel_lock);

	dsi_panel_destroy_cmd_packets(set);
	dsi_panel_dealloc_cmd_packets(set);

	return rc;
}

int dsi_panel_pre_mode_switch_to_video(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_CMD_TO_VID_SWITCH);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_CMD_TO_VID_SWITCH cmds, rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_pre_mode_switch_to_cmd(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_VID_TO_CMD_SWITCH);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_CMD_TO_VID_SWITCH cmds, rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_mode_switch_to_cmd(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_POST_VID_TO_CMD_SWITCH);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_CMD_TO_VID_SWITCH cmds, rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_mode_switch_to_vid(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_POST_CMD_TO_VID_SWITCH);
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_CMD_TO_VID_SWITCH cmds, rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_switch(struct dsi_panel *panel)
{
	int rc = 0;
	static int cur_h_active = 0;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_dual_supported() && panel->is_secondary)
		return rc;
#endif
	if (cur_h_active != panel->cur_mode->timing.h_active) {
		udelay(2000);
		cur_h_active = panel->cur_mode->timing.h_active;
	}

	mutex_lock(&panel->panel_lock);

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported()) {
		rc = iris_switch(panel,
				&(panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_TIMING_SWITCH]),
				&panel->cur_mode->timing);
	} else
#endif
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_TIMING_SWITCH);
	DSI_ERR("Send DSI_CMD_SET_TIMING_SWITCH cmds\n");
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_TIMING_SWITCH cmds, rc=%d\n",
		       panel->name, rc);

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		dsi_panel_adfr_status_reset(panel);
		oplus_adfr_resolution_vsync_switch(panel);
	}
#endif

	if((strcmp(panel->name, "samsung dsc cmd mode oneplus dsi panel") == 0) && (gamma_read_flag == GAMMA_READ_SUCCESS)) {
		if (mode_fps == 90) {
			rc = dsi_panel_tx_gamma_cmd_set(panel, DSI_GAMMA_CMD_SET_SWITCH_90HZ);
			DSI_ERR("Send DSI_GAMMA_CMD_SET_SWITCH_90HZ cmds\n");
			if (rc)
				DSI_ERR("[%s] Failed to send DSI_GAMMA_CMD_SET_SWITCH_90HZ cmds, rc=%d\n",
					panel->name, rc);
		}
		else {
			rc = dsi_panel_tx_gamma_cmd_set(panel, DSI_GAMMA_CMD_SET_SWITCH_60HZ);
			DSI_ERR("Send DSI_GAMMA_CMD_SET_SWITCH_60HZ cmds\n");
			if (rc)
				DSI_ERR("[%s] Failed to send DSI_GAMMA_CMD_SET_SWITCH_60HZ cmds, rc=%d\n",
					panel->name, rc);
		}
	}

	mutex_unlock(&panel->panel_lock);
	panel->panel_switch_status = false;
	return rc;
}

int dsi_panel_post_switch(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_dual_supported() && panel->is_secondary)
		return rc;
#endif
	panel->panel_switch_status = true;
	mutex_lock(&panel->panel_lock);

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported()) {
		rc = iris_post_switch(panel,
				&(panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_POST_TIMING_SWITCH]),
				&panel->cur_mode->timing);
	} else
#endif
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_POST_TIMING_SWITCH);
	DSI_ERR("Send DSI_CMD_SET_POST_TIMING_SWITCH cmds\n");
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_POST_TIMING_SWITCH cmds, rc=%d\n",
		       panel->name, rc);

	mutex_unlock(&panel->panel_lock);
	return rc;
}

bool aod_fod_flag = false;
bool real_aod_mode = false;

extern bool oneplus_dimlayer_hbm_enable;
bool backup_dimlayer_hbm = false;
extern int oneplus_dim_status;
int backup_dim_status = 0;

int dsi_panel_enable(struct dsi_panel *panel)
{
	int rc = 0;
	int blank;
	struct drm_panel_notifier notifier_data;

	if (!panel) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
	DSI_ERR("start\n");

	mutex_lock(&panel->panel_lock);

	if (panel->aod_mode == 2) {
		DSI_ERR("Send dsi_panel_set_aod_mode 2 cmds\n");
		rc = dsi_panel_set_aod_mode(panel, 2);
		panel->aod_status = 1;
	}

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		oplus_adfr_vsync_switch_reset(panel);
		}
#endif

	if (strcmp(panel->name, "samsung dsc cmd mode oneplus dsi panel") == 0) {
		if ((panel->panel_stage_info == EVT2_113MHZ_OSC) || (panel->panel_stage_info == PVT_113MHZ_OSC)
			|| (panel->panel_stage_info == PVT_113MHZ_OSC_XTALK) || (panel->panel_code_info == 0xEE)) {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_113MHZ_OSC_ON);
			DSI_ERR("Send DSI_CMD_SET_113MHZ_OSC_ON cmds\n");
		} else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ON);
			DSI_ERR("Send DSI_CMD_SET_ON cmds\n");
		}
	} else {
#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported())
		rc = iris_enable(panel, &(panel->cur_mode->priv_info->cmd_sets[DSI_CMD_SET_ON]));
	else
#endif
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ON);
		DSI_ERR("Send DSI_CMD_SET_ON cmds\n");
	}
	if (rc)
		DSI_ERR("[%s] failed to send DSI_CMD_SET_ON cmds, rc=%d\n",
		       panel->name, rc);
	else
		panel->panel_initialized = true;

	oplus_dsi_panel_enable(panel);

#ifdef CONFIG_PXLW_IRIS
	if (iris_is_chip_supported()) {
		dsi_panel_adfr_status_reset(panel);
	}
#endif

	if (strcmp(panel->name, "samsung amb655x fhd cmd mode dsc dsi panel") == 0) {
		rc = dsi_panel_dimming_gamma_write(panel);
	}

	if ((strcmp(panel->name, "samsung dsc cmd mode oneplus dsi panel") == 0) && (gamma_read_flag == GAMMA_READ_SUCCESS)) {
		if (mode_fps == 60) {
			rc = dsi_panel_tx_gamma_cmd_set(panel, DSI_GAMMA_CMD_SET_SWITCH_60HZ);
			DSI_ERR("Send DSI_GAMMA_CMD_SET_SWITCH_60HZ cmds\n");
			if (rc)
				DSI_ERR("[%s] Failed to send DSI_GAMMA_CMD_SET_SWITCH_60HZ cmds, rc=%d\n",
					panel->name, rc);
		}
	}

	panel->need_power_on_backlight = true;

	oneplus_dimlayer_hbm_enable = backup_dimlayer_hbm;
	oneplus_dim_status = backup_dim_status;
	pr_err("Restore dim when panel goes on");

	blank = DRM_PANEL_BLANK_UNBLANK_CHARGE;
	notifier_data.data = &blank;
	if (&panel->drm_panel!= NULL)
		drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);

	if (panel->aod_mode == 0) {
		DSI_ERR("Send dsi_panel_set_aod_mode 0 cmds\n");
		panel->aod_status = 0;
	}

#if defined(CONFIG_PXLW_IRIS)
	if (panel->is_secondary) {
		mutex_unlock(&panel->panel_lock);
		return rc;
	}
#endif
	mutex_unlock(&panel->panel_lock);
	DSI_ERR("end\n");
	#if 0
	pm_print_active_wakeup_sources_queue(false);
	#endif
	return rc;
}

int dsi_panel_post_enable(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_POST_ON);
	if (rc) {
		DSI_ERR("[%s] failed to send DSI_CMD_SET_POST_ON cmds, rc=%d\n",
		       panel->name, rc);
		goto error;
	}
error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_pre_disable(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	if (gpio_is_valid(panel->bl_config.en_gpio))
		gpio_set_value(panel->bl_config.en_gpio, 0);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_PRE_OFF);
	if (rc) {
		DSI_ERR("[%s] failed to send DSI_CMD_SET_PRE_OFF cmds, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_disable(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	DSI_ERR("start\n");
#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_dual_supported() && panel->is_secondary)
		return rc;
#endif
	mutex_lock(&panel->panel_lock);

	/* Avoid sending panel off commands when ESD recovery is underway */
	if (!atomic_read(&panel->esd_recovery_pending)) {
		oneplus_dimlayer_hbm_enable = false;
		oneplus_dim_status = 0;
		pr_err("Kill dim when panel goes off");
		HBM_flag = false;

		/*
		 * Need to set IBB/AB regulator mode to STANDBY,
		 * if panel is going off from AOD mode.
		 */
		if (dsi_panel_is_type_oled(panel) &&
			(panel->power_mode == SDE_MODE_DPMS_LP1 ||
			panel->power_mode == SDE_MODE_DPMS_LP2))
			dsi_pwr_panel_regulator_mode_set(&panel->power_info,
				"ibb", REGULATOR_MODE_STANDBY);
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_OFF);
#if defined(CONFIG_PXLW_IRIS)
		if (iris_is_chip_supported())
			iris_disable(panel, NULL);
#endif
		if (rc) {
			/*
			 * Sending panel off commands may fail when  DSI
			 * controller is in a bad state. These failures can be
			 * ignored since controller will go for full reset on
			 * subsequent display enable anyway.
			 */
			pr_warn_ratelimited("[%s] failed to send DSI_CMD_SET_OFF cmds, rc=%d\n",
					panel->name, rc);
			rc = 0;
		}
		if (panel->aod_mode == 2)
			panel->aod_status = 1;

		if (panel->aod_mode == 0)
			panel->aod_status = 0;
	}
	panel->panel_initialized = false;
	panel->power_mode = SDE_MODE_DPMS_OFF;

	mutex_unlock(&panel->panel_lock);
	DSI_ERR("end\n");

	#if 0
	pm_print_active_wakeup_sources_queue(true);
#endif
	return rc;
}

int dsi_panel_unprepare(struct dsi_panel *panel)
{
	int rc = 0;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_POST_OFF);
	if (rc) {
		DSI_ERR("[%s] failed to send DSI_CMD_SET_POST_OFF cmds, rc=%d\n",
		       panel->name, rc);
		goto error;
	}

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_post_unprepare(struct dsi_panel *panel)
{
	int rc = 0;
	int blank;
	struct drm_panel_notifier notifier_data;

	if (!panel) {
		DSI_ERR("invalid params\n");
		return -EINVAL;
	}

	if (panel->err_flag_status == true) {
		DSI_ERR("no need to power off when err flag irq comes\n");
		return rc;
	}

	mutex_lock(&panel->panel_lock);

	rc = dsi_panel_power_off(panel);
	if (rc) {
		DSI_ERR("[%s] panel power_Off failed, rc=%d\n",
		       panel->name, rc);
	}


	blank = DRM_PANEL_BLANK_POWERDOWN_CHARGE;
	notifier_data.data = &blank;

	if (&panel->drm_panel!= NULL)
		drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);
	mutex_unlock(&panel->panel_lock);
	return rc;
}
int dsi_panel_set_seed_lp_mode(struct dsi_panel *panel, int seed_lp_level)
{
	int rc = 0;
	u32 count;
	struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
		}

	mutex_lock(&panel->panel_lock);
	mode = panel->cur_mode;

	switch (seed_lp_level) {
	case 0:
		count = mode->priv_info->cmd_sets[DSI_CMD_SET_SEED_LP_ON_0].count;
		if (!count) {
			DSI_ERR("This panel does not support seed lp mode0 on.\n");
			goto error;
		}
		else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_SEED_LP_ON_0);
			if(!rc){
				DSI_ERR("This panel does not support seed lp mode0.\n");
			goto error;
			}
			DSI_ERR("Send DSI_CMD_SET_SEED_LP_ON_0 cmds.\n");
		}
		break;

	case 1:
		count = mode->priv_info->cmd_sets[DSI_CMD_SET_SEED_LP_ON_1].count;
		if (!count) {
			DSI_ERR("This panel does not support seed lp mode1.\n");
			goto error;
		}
		else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_SEED_LP_ON_1);
			if(!rc){
				DSI_ERR("This panel does not support seed lp mode1.\n");
			goto error;
			}
			DSI_ERR("Send DSI_CMD_SET_SEED_LP_ON_1 cmds.\n");
		}
		break;

	case 2:
		count = mode->priv_info->cmd_sets[DSI_CMD_SET_SEED_LP_ON_2].count;
		if (!count) {
			DSI_ERR("This panel does not support seed lp mode2.\n");
			goto error;
		}
		else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_SEED_LP_ON_2);
			if(!rc){
				DSI_ERR("This panel does not support seed lp mode2.\n");
			goto error;
			}
			DSI_ERR("Send DSI_CMD_SET_SEED_LP_ON_2 cmds.\n");
		}
		break;

	case 4:   //default off
		count = mode->priv_info->cmd_sets[DSI_CMD_SET_SEED_LP_OFF].count;
		if (!count) {
			DSI_ERR("This panel does not support seed lp off.\n");
			goto error;
		}
		else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_SEED_LP_OFF);
			if(!rc){
				DSI_ERR("This panel does not support seed lp off.\n");
			goto error;
			}
			DSI_ERR("Send DSI_CMD_SET_SEED_LP_OFF cmds.\n");
		}
		break;

		default:
			break;
	}

	error:
		mutex_unlock(&panel->panel_lock);
		return rc;
}

int dsi_panel_set_hbm_mode(struct dsi_panel *panel, int level)
{
		int rc = 0;
		u32 count;
		struct dsi_display_mode *mode;

		if (!panel || !panel->cur_mode) {
			DSI_ERR("Invalid params\n");
			return -EINVAL;
		}

		mutex_lock(&panel->panel_lock);
		mode = panel->cur_mode;

		switch (level) {
		case 0:
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_OFF].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM mode off.\n");
				goto error;
			}
			else {
				HBM_flag = false;
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_OFF);
				DSI_ERR("Send DSI_CMD_SET_HBM_OFF cmds.\n");
				DSI_ERR("hbm_backight = %d, panel->bl_config.bl_level = %d\n",panel->hbm_backlight, panel->bl_config.bl_level);
				rc= dsi_panel_update_backlight(panel,panel->hbm_backlight);
			}
			break;

		case 1:
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_ON_1].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM mode 1.\n");
				goto error;
			}
			else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_ON_1);
				DSI_ERR("Send DSI_CMD_SET_HBM_ON_1 cmds.\n");
			}
			break;

		case 2:
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_ON_2].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM mode 2.\n");
				goto error;
			}
			else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_ON_2);
				DSI_ERR("Send DSI_CMD_SET_HBM_ON_2 cmds.\n");
			}
			break;

		case 3:
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_ON_3].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM mode 3.\n");
				goto error;
			}
			else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_ON_3);
				DSI_ERR("Send DSI_CMD_SET_HBM_ON_3 cmds.\n");
			}
			break;

		case 4:
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_ON_4].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM mode 4.\n");
				goto error;
			}
			else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_ON_4);
				DSI_ERR("Send DSI_CMD_SET_HBM_ON_4 cmds.\n");
			}
			break;

		case 5:
			count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_ON_5].count;
			if (!count) {
				DSI_ERR("This panel does not support HBM mode 5.\n");
				goto error;
			}
			else {
				HBM_flag = true;
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_ON_5);
				DSI_ERR("Send DSI_CMD_SET_HBM_ON_5 cmds.\n");
			}
			break;

		default:
			break;
		}

		DSI_ERR("Set HBM Mode = %d\n", level);
		if(level == 5){
			DSI_ERR("HBM == 5 for fingerprint\n");
		}

	error:
		mutex_unlock(&panel->panel_lock);
		return rc;
}

int dsi_panel_set_hbm_brightness(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
	struct mipi_dsi_device *dsi;
	struct dsi_display_mode *mode;

	if (!panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	dsi = &panel->mipi_device;
	mode = panel->cur_mode;

	if (panel->is_hbm_enabled) {
		hbm_finger_print = true;
		DSI_ERR("HBM is enabled\n");
		return 0;
	}

	mutex_lock(&panel->panel_lock);
	if (hbm_brightness_flag == 0) {
		count = mode->priv_info->cmd_sets[DSI_CMD_SET_HBM_BRIGHTNESS_ON].count;
		if (!count) {
			DSI_ERR("This panel does not support HBM brightness on mode.\n");
			goto error;
		}
		else {
			DSI_ERR("Send DSI_CMD_SET_HBM_BRIGHTNESS_ON cmds.\n");
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_HBM_BRIGHTNESS_ON);
			hbm_brightness_flag = 1;
		}
	}
	DSI_ERR("hbm backlight = %d\n", level);
	if (strcmp(panel->name, "samsung sofef03f_m fhd cmd mode dsc dsi panel") == 0) {
		level = level + 1023;
	} else if (strcmp(panel->name, "BOE dsc cmd mode oneplus dsi panel") == 0) {
		level = level + 3072;
	} else if (strcmp(panel->name, "samsung amb670yf01 dsc cmd mode panel") == 0) {
		if(level >= HBM_BASE_600NIT_800NIT)
			level = HBM_BASE_600NIT_800NIT-1;
		level = backlight_600_800nit_buf[level];
	}

#if defined(CONFIG_PXLW_IRIS)
	if (iris_is_chip_supported() && iris_is_pt_mode(panel))
		rc = iris_update_backlight(1, level);
	else
		rc = mipi_dsi_dcs_set_display_brightness_samsung(get_main_display(), level);
#else
	rc = mipi_dsi_dcs_set_display_brightness_samsung(get_main_display(), level);
#endif

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_set_acl_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_cmd_desc *cmds;
    struct dsi_display_mode *mode;
    u8 *tx = NULL;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
	mutex_lock(&panel->panel_lock);

    mode = panel->cur_mode;

    count = mode->priv_info->cmd_sets[DSI_CMD_SET_ACL_MODE].count;
    cmds = mode->priv_info->cmd_sets[DSI_CMD_SET_ACL_MODE].cmds;

	if (count == 0) {
		DSI_ERR("This panel does not support acl mode\n");
		goto error;
	}

    tx = (u8 *)cmds[panel->acl_cmd_index].msg.tx_buf;
    if (tx != NULL) {
	    tx[panel->acl_mode_index] = level;
	}
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_ACL_MODE);

    DSI_ERR("Set ACL Mode = %d\n", level);

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_set_dci_p3_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;
	if(aod_fod_flag==true)
	return rc;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);
    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_DCI_P3_ON].count;


            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DCI_P3_ON);
            DSI_ERR("DCI-P3 Mode On.\n");
    } else {

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_DCI_P3_OFF);
            DSI_ERR("DCI-P3 Mode Off.\n");
    }
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_set_night_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);
    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NIGHT_ON].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NIGHT_ON);
            DSI_ERR("night Mode On.\n");
    } else {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NIGHT_OFF].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NIGHT_OFF);
            DSI_ERR("night Mode Off.\n");
    }
	mutex_unlock(&panel->panel_lock);
	return rc;
}
int dsi_panel_set_native_display_p3_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);

    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NATIVE_DISPLAY_P3_ON].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NATIVE_DISPLAY_P3_ON);
            DSI_ERR("Native Display p3 Mode On.\n");
    } else {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NATIVE_DISPLAY_P3_OFF].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NATIVE_DISPLAY_P3_OFF);
            DSI_ERR("Native Display p3 Mode Off.\n");
    }
	mutex_unlock(&panel->panel_lock);
return rc;
}

int dsi_panel_set_native_display_wide_color_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);

    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NATIVE_DISPLAY_WIDE_COLOR_ON].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NATIVE_DISPLAY_WIDE_COLOR_ON);
            DSI_ERR("Native wide color Mode On.\n");
    } else {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NATIVE_DISPLAY_WIDE_COLOR_OFF].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NATIVE_DISPLAY_WIDE_COLOR_OFF);
            DSI_ERR("Native wide color Mode Off.\n");
    }
	mutex_unlock(&panel->panel_lock);
return rc;
}

int dsi_panel_set_native_display_srgb_color_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);

    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NATIVE_DISPLAY_SRGB_COLOR_ON].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NATIVE_DISPLAY_SRGB_COLOR_ON);
            DSI_ERR("Native srgb color Mode On.\n");
    } else {
        count = mode->priv_info->cmd_sets[DSI_CMD_SET_NATIVE_DISPLAY_SRGB_COLOR_OFF].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_NATIVE_DISPLAY_SRGB_COLOR_OFF);
            DSI_ERR("Native  srgb color Mode Off.\n");
    }
	mutex_unlock(&panel->panel_lock);
return rc;
}

int dsi_panel_set_customer_srgb_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);

    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_LOADING_CUSTOMER_RGB_ON].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_CUSTOMER_RGB_ON);
            DSI_ERR("turn on customer srgb\n");
    } else {
        count = mode->priv_info->cmd_sets[DSI_CMD_LOADING_CUSTOMER_RGB_OFF].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_CUSTOMER_RGB_OFF);
            DSI_ERR("turn off customer srgb\n");
    }
	mutex_unlock(&panel->panel_lock);
return rc;
}

int dsi_panel_set_customer_p3_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	u32 count;
    struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
    mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);

    if (level) {
        count = mode->priv_info->cmd_sets[DSI_CMD_LOADING_CUSTOMER_P3_ON].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_CUSTOMER_P3_ON);
            DSI_ERR("turn on customer P3\n");
    } else {
        count = mode->priv_info->cmd_sets[DSI_CMD_LOADING_CUSTOMER_P3_OFF].count;

            rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_LOADING_CUSTOMER_P3_OFF);
            DSI_ERR("turn off customer P3\n");
    }
	mutex_unlock(&panel->panel_lock);
return rc;
}

int dsi_panel_set_aod_mode(struct dsi_panel *panel, int level)
{
	int rc = 0;
	struct dsi_display_mode *mode;
	struct drm_panel_notifier notifier_data;
	int tp_aod_flag;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	if (panel->aod_disable)
		return 0;

	mode = panel->cur_mode;
	DSI_ERR("aod_status == %d\n", panel->aod_status);

	if (level == 1) {
		mutex_lock(&panel->panel_lock);
		panel->aod_status = 1;
		real_aod_mode = true;
		if (dsi_panel_name == DSI_PANEL_SAMSUNG_AMB670YF01) {
			if (panel->panel_stage_info >= 1 &&
				panel->panel_stage_info <= 4) {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_1_O);
				DSI_ERR("Send DSI_CMD_SET_AOD_ON_1_O cmds\n");
			} else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_1);
				DSI_ERR("Send DSI_CMD_SET_AOD_ON_1 cmds\n");
			}
		} else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_1);
			DSI_ERR("Send DSI_CMD_SET_AOD_ON_1 cmds\n");
		}

		tp_aod_flag = 100;
		notifier_data.data = &tp_aod_flag;
		DSI_ERR("set aod state TP flag: %d\n", tp_aod_flag);
		if (&panel->drm_panel!= NULL)
			drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);
		aod_fod_flag = false;
		mutex_unlock(&panel->panel_lock);
	} else if (level == 2) {
		if (panel->aod_status == 0) {
			panel->aod_status = 1;
			real_aod_mode = false;
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_2);
			DSI_ERR("Send  DSI_CMD_SET_AOD_ON_2 cmds\n");

			tp_aod_flag = 100;
			notifier_data.data = &tp_aod_flag;
			DSI_ERR("set aod state TP flag: %d\n", tp_aod_flag);

			if (&panel->drm_panel!= NULL)
				drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);
			aod_fod_flag = false;
		}
	} else if (level == 3) {
		mutex_lock(&panel->panel_lock);
		panel->aod_status = 1;
		real_aod_mode = true;
		if (dsi_panel_name == DSI_PANEL_SAMSUNG_AMB670YF01) {
			if (panel->panel_stage_info >= 1 &&
				panel->panel_stage_info <= 4) {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_3_O);
				DSI_ERR("Send DSI_CMD_SET_AOD_ON_3_O cmds\n");
			} else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_3);
				DSI_ERR("Send DSI_CMD_SET_AOD_ON_3 cmds\n");
			}
		} else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_3);
			DSI_ERR("Send DSI_CMD_SET_AOD_ON_3 cmds\n");
		}

		tp_aod_flag = 100;
		notifier_data.data = &tp_aod_flag;
		DSI_ERR("set aod state TP flag: %d\n", tp_aod_flag);

		if (&panel->drm_panel!= NULL)
			drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);
		aod_fod_flag = false;
		mutex_unlock(&panel->panel_lock);
	} else if (level == 4 || level == 5) {
		mutex_lock(&panel->panel_lock);
		panel->aod_status = 1;
		real_aod_mode = true;
		if (dsi_panel_name == DSI_PANEL_SAMSUNG_AMB670YF01) {
			if (panel->panel_stage_info >= 1 &&
				panel->panel_stage_info <= 4) {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_5_O);
				DSI_ERR("Send DSI_CMD_SET_AOD_ON_5_O cmds\n");
			} else {
				rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_5);
				DSI_ERR("Send DSI_CMD_SET_AOD_ON_5 cmds\n");
			}
		} else {
			rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_ON_5);
			DSI_ERR("Send DSI_CMD_SET_AOD_ON_5 cmds\n");
		}

		tp_aod_flag = 100;
		notifier_data.data = &tp_aod_flag;
		DSI_ERR("set aod state TP flag: %d\n", tp_aod_flag);

		if (&panel->drm_panel!= NULL)
			drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);
		aod_fod_flag = false;
		mutex_unlock(&panel->panel_lock);
	} else {
		if (panel->aod_status) {
			panel->aod_status = 0;
			if (real_aod_mode) {
				mutex_lock(&panel->panel_lock);
				if (dsi_panel_name == DSI_PANEL_SAMSUNG_AMB670YF01) {
					if (panel->panel_stage_info == 1) {
						rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_OFF_O1);
						DSI_ERR("Real aod mode send DSI_CMD_SET_AOD_OFF_O1 cmds\n");
					} else if (panel->panel_stage_info >= 2 &&
						panel->panel_stage_info <= 4) {
						rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_OFF_O2);
						DSI_ERR("Real aod mode send DSI_CMD_SET_AOD_OFF_O2 cmds\n");
					} else {
						rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_OFF);
						DSI_ERR("Real aod mode send DSI_CMD_SET_AOD_OFF cmds\n");
					}
				} else {
					rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_AOD_OFF);
					DSI_ERR("Real aod mode send DSI_CMD_SET_AOD_OFF cmds\n");
				}
				mutex_unlock(&panel->panel_lock);
#ifdef CONFIG_PXLW_IRIS
				if (iris_is_chip_supported()) {
					oplus_adfr_aod_fod_vsync_switch(panel, false);
				}
#endif
			} else {
				DSI_ERR("real_aod_mode is %d, aod_fod_flag is %d\n",
						real_aod_mode, aod_fod_flag);
			}

			if (level == 0) {
					tp_aod_flag = 200;
					notifier_data.data = &tp_aod_flag;
					DSI_ERR("set aod state TP flag: %d\n", tp_aod_flag);
				if (&panel->drm_panel!= NULL)
					drm_panel_notifier_call_chain(&panel->drm_panel, DRM_PANEL_EARLY_EVENT_BLANK, &notifier_data);
			}
		}
	}

	panel->aod_curr_mode = level;
	DSI_ERR("AOD mode = %d\n", level);

	return rc;
}

int dsi_panel_send_dsi_panel_command(struct dsi_panel *panel)
{
	int rc = 0;
	int count;
	struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
	mode = panel->cur_mode;
	mutex_lock(&panel->panel_lock);

	count = mode->priv_info->cmd_sets[DSI_CMD_SET_PANEL_COMMAND].count;
	if (!count) {
		DSI_ERR("This panel does not support DSI_CMD_SET_PANEL_COMMAND\n");
		goto error;
	}
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_PANEL_COMMAND);
	if (rc)
		DSI_ERR("Failed to send dsi panel command\n");
	DSI_ERR("Send DSI_CMD_SET_PANEL_COMMAND cmds.\n");

error:
	mutex_unlock(&panel->panel_lock);
	return rc;
}

int dsi_panel_send_dsi_seed_command(struct dsi_panel *panel)
{
	int rc = 0;
	int count;
	struct dsi_display_mode *mode;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}
	mode = panel->cur_mode;

	count = mode->priv_info->cmd_sets[DSI_CMD_SET_SEED_COMMAND].count;
	if (!count) {
		DSI_ERR("This panel does not support DSI_CMD_SET_SEED_COMMAND\n");
		goto error;
	}
	rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_SEED_COMMAND);
	if (rc)
		DSI_ERR("Failed to send dsi seed command\n");

error:
	return rc;
}

static int dsi_panel_parse_gamma_cmd_sets_sub(struct dsi_panel_cmd_set *cmd, const char *data, unsigned int length, enum dsi_cmd_set_state state, enum dsi_gamma_cmd_set_type type)
{
	int rc = 0;
	u32 packet_count = 0;

	if (!data) {
		DSI_DEBUG("[%s] data not found\n", gamma_cmd_set_map[type]);
		rc = -ENOTSUPP;
		goto error;
	}

	DSI_DEBUG("type=%d, name=%s, length=%d\n", type,
		gamma_cmd_set_map[type], length);

	print_hex_dump_debug("", DUMP_PREFIX_NONE,
		       8, 1, data, length, false);

	rc = dsi_panel_get_cmd_pkt_count(data, length, &packet_count);
	if (rc) {
		DSI_ERR("commands failed, rc=%d\n", rc);
		goto error;
	}
	DSI_DEBUG("[%s] packet-count=%d, %d\n", gamma_cmd_set_map[type],
		packet_count, length);

	rc = dsi_panel_alloc_cmd_packets(cmd, packet_count);
	if (rc) {
		DSI_ERR("failed to allocate cmd packets, rc=%d\n", rc);
		goto error;
	}

	rc = dsi_panel_create_cmd_packets(data, length, packet_count,
					  cmd->cmds);
	if (rc) {
		DSI_ERR("failed to create cmd packets, rc=%d\n", rc);
		goto error_free_mem;
	}

	cmd->state = state;

	return rc;

error_free_mem:
	kfree(cmd->cmds);
	cmd->cmds = NULL;

error:
	return rc;

}

int dsi_panel_parse_gamma_cmd_sets(void)
{
    int rc = 0;
    int i = 0;

    memset(gamma_cmd_set, 0, 2*sizeof(struct dsi_panel_cmd_set));

    for (i = 0; i < 2; i++) {
        rc = dsi_panel_parse_gamma_cmd_sets_sub(&gamma_cmd_set[i], (char *)&gamma_para[i], sizeof(gamma_para)/2, DSI_CMD_SET_STATE_HS, i);
        if (rc) {
            DSI_ERR("Failed to parse gamma cmd sets %d, rc=%d\n", i, rc);
        }
    }

    return rc;
}

int dsi_panel_tx_gamma_cmd_set(struct dsi_panel *panel,
				enum dsi_gamma_cmd_set_type type)
{
	int rc = 0, i = 0;
	ssize_t len;
	struct dsi_cmd_desc *cmds;
	u32 count;
	enum dsi_cmd_set_state state;
	const struct mipi_dsi_host_ops *ops = panel->host->ops;

	if (!panel)
		return -EINVAL;

	cmds = gamma_cmd_set[type].cmds;
	count = gamma_cmd_set[type].count;
	state = gamma_cmd_set[type].state;

	if (count == 0) {
		DSI_DEBUG("[%s] No commands to be sent for gamma state(%d)\n",
			 panel->name, type);
		goto error;
	}

	for (i = 0; i < count; i++) {
		if (state == DSI_CMD_SET_STATE_LP)
			cmds->msg.flags |= MIPI_DSI_MSG_USE_LPM;

		if (cmds->last_command)
			cmds->msg.flags |= MIPI_DSI_MSG_LASTCOMMAND;

		len = ops->transfer(panel->host, &cmds->msg);
		if (len < 0) {
			rc = len;
			DSI_ERR("failed to set cmds(%d), rc=%d\n", type, rc);
			goto error;
		}
		if (cmds->post_wait_ms)
			usleep_range(cmds->post_wait_ms*1000,
					((cmds->post_wait_ms*1000)+10));
		cmds++;
	}
error:
	return rc;
}

void dsi_panel_update_gamma_change_write(struct dsi_panel *panel)
{
	int r_4 = 0;
	int g_4 = 0;
	int b_4 = 0;
	int r_3 = 0;
	int g_3 = 0;
	int b_3 = 0;
	int r_2 = 0;
	int g_2 = 0;
	int b_2 = 0;
	int r_1 = 0;
	int g_1 = 0;
	int b_1 = 0;
	int i = 0;


	if (strcmp(panel->name, "samsung amb655x fhd cmd mode dsc dsi panel") == 0) {
	/* GAMMA change Setting (120Hz) 80-40-20-10*/
		for (i = 0; i < 229; i++) {
		DSI_ERR("120HZ B9_register_value[%d] = 0x%02x", i, b9_register_value_500step[i]);
		}
		for (i = 0; i < 186; i++) {
			b9_register_value[i]=b9_register_value_500step[i];
		}
		for (i = 0; i < 12; i++){
			dimming_gamma_120hz[i] = b9_register_value[174+i];
			}
		r_3 = ((dimming_gamma_120hz[8] & 0x3F) << 4) | ((dimming_gamma_120hz[9] & 0xF0) >> 4);
		if (r_3 < 0) {
			r_3 = 0;
			DSI_ERR("80nit 120hz: r_3 = 0");
		}
		g_3 = ((dimming_gamma_120hz[9] & 0x0F) << 6) | ((dimming_gamma_120hz[10] & 0xFC) >> 2);
		if (g_3 < 0) {
			g_3 = 0;
			DSI_ERR("80nit 120hz: g_3 = 0");
		}
		b_3 = ((dimming_gamma_120hz[10] & 0x03) << 8) | dimming_gamma_120hz[11];
		if (b_3 < 0) {
			b_3 = 0;
			DSI_ERR("80nit 120hz: b_3 = 0");
		}
		r_2 = r_3 - 40;
		if (r_2 < 0) {
			r_2 = 0;
			DSI_ERR("80nit 120hz: r_2 = 0");
		}
		g_2 = g_3 - 21;
		if (g_2 < 0) {
			g_2 = 0;
			DSI_ERR("80nit 120hz: g_2 = 0");
		}
		b_2 = b_3 - 40;
		if (b_2 < 0) {
			b_2 = 0;
			DSI_ERR("80nit 120hz: b_2 = 0");
		}
		r_1 = r_3 - 52;
		if (r_1 < 0) {
			r_1 = 0;
			DSI_ERR("80nit 120hz: r_1 = 0");
		}
		g_1 = g_3 - 24;
		if (g_1 < 0) {
			g_1 = 0;
			DSI_ERR("80nit 120hz: g_1 = 0");
		}
		b_1 = b_3 - 68;
		if (b_1 < 0) {
			b_1 = 0;
			DSI_ERR("80nit 120hz: b_1 = 0");
		}
		dimming_gamma_120hz[0] = (dimming_gamma_120hz[0] & 0xC0) | (r_1 >> 4);
		dimming_gamma_120hz[1] = ((r_1 & 0x00F) << 4) | (g_1 >> 6);
		dimming_gamma_120hz[2] = ((g_1 & 0x03F) << 2) | (b_1 >> 8);
		dimming_gamma_120hz[3] = b_1 & 0x0FF;
		dimming_gamma_120hz[4] = (dimming_gamma_120hz[4] & 0xC0) | (r_2 >> 4);
		dimming_gamma_120hz[5] = ((r_2 & 0x00F) << 4) | (g_2 >> 6);
		dimming_gamma_120hz[6] = ((g_2 & 0x03F) << 2) | (b_2 >> 8);
		dimming_gamma_120hz[7] = b_2 & 0x0FF;
		for (i = 0; i < 8; i++) {
		DSI_ERR("dimming_gamma_120hz_80nit[%d] = 0x%02x", i, dimming_gamma_120hz[i]);
		}
		for (i = 0; i < 12; i++)
			dimming_gamma_120hz[12+i] = b9_register_value[131+i];
		r_3 = ((dimming_gamma_120hz[20] & 0x3F) << 4) | ((dimming_gamma_120hz[21] & 0xF0) >> 4);
		if (r_3 < 0) {
			r_3 = 0;
			DSI_ERR("40nit 120hz: r_3 = 0");
		}
		g_3 = ((dimming_gamma_120hz[21] & 0x0F) << 6) | ((dimming_gamma_120hz[22] & 0xFC) >> 2);
		if (g_3 < 0) {
			g_3 = 0;
			DSI_ERR("40nit 120hz: g_3 = 0");
		}
		b_3 = ((dimming_gamma_120hz[22] & 0x03) << 8) | dimming_gamma_120hz[23];	
		if (b_3 < 0) {
			b_3 = 0;
			DSI_ERR("40nit 120hz: b_3 = 0");
		}
		r_2 = r_3 - 27;
		if (r_2 < 0) {
			r_2 = 0;
			DSI_ERR("40nit 120hz: r_2 = 0");
		}
		g_2 = g_3 - 13;
		if (g_2 < 0) {
			g_2 = 0;
			DSI_ERR("40nit 120hz: g_2 = 0");
		}
		b_2 = b_3 - 38;
		if (b_2 < 0) {
			b_2 = 0;
			DSI_ERR("40nit 120hz: b_2 = 0");
		}
		r_1 = r_3 - 38;
		if (r_1 < 0) {
			r_1 = 0;
			DSI_ERR("40nit 120hz: r_1 = 0");
		}
		g_1 = g_3 - 20;
		if (g_1 < 0) {
			g_1 = 0;
			DSI_ERR("40nit 120hz: g_1 = 0");
		}
		b_1 = b_3 - 56;
		if (b_1 < 0) {
			b_1 = 0;
			DSI_ERR("40nit 120hz: b_1 = 0");
		}
		dimming_gamma_120hz[12] = (dimming_gamma_120hz[12] & 0xC0) | (r_1 >> 4);
		dimming_gamma_120hz[13] = ((r_1 & 0x00F) << 4) | (g_1 >> 6);
		dimming_gamma_120hz[14] = ((g_1 & 0x03F) << 2) | (b_1 >> 8);
		dimming_gamma_120hz[15] = b_1 & 0x0FF;
		dimming_gamma_120hz[16] = (dimming_gamma_120hz[16] & 0xC0) | (r_2 >> 4);
		dimming_gamma_120hz[17] = ((r_2 & 0x00F) << 4) | (g_2 >> 6);
		dimming_gamma_120hz[18] = ((g_2 & 0x03F) << 2) | (b_2 >> 8);
		dimming_gamma_120hz[19] = b_2 & 0x0FF;
		for (i = 12; i < 20; i++) {
		DSI_ERR("dimming_gamma_120hz_40nit[%d] = 0x%02x", i, dimming_gamma_120hz[i]);
		}
		for (i = 0; i < 12; i++){
			dimming_gamma_120hz[24+i] = b9_register_value[88+i];
			}
		r_3 = ((dimming_gamma_120hz[32] & 0x3F) << 4) | ((dimming_gamma_120hz[33] & 0xF0) >> 4);
		if (r_3 < 0) {
			r_3 = 0;
			DSI_ERR("20nit 120hz: r_3 = 0");
		}
		g_3 = ((dimming_gamma_120hz[33] & 0x0F) << 6) | ((dimming_gamma_120hz[34] & 0xFC) >> 2);
		if (g_3 < 0) {
			g_3 = 0;
			DSI_ERR("20nit 120hz: g_3 = 0");
		}
		b_3 = ((dimming_gamma_120hz[34] & 0x03) << 8) | dimming_gamma_120hz[35];
		if (b_3 < 0) {
			b_3 = 0;
			DSI_ERR("20nit 120hz: b_3 = 0");
		}
		r_2 = r_3 - 16;
		if (r_2 < 0) {
			r_2 = 0;
			DSI_ERR("20nit 120hz: r_2 = 0");
		}
		g_2 = g_3 - 10;
		if (g_2 < 0) {
			g_2 = 0;
			DSI_ERR("20nit 120hz: g_2 = 0");
		}
		b_2 = b_3 - 26;
		if (b_2 < 0) {
			b_2 = 0;
			DSI_ERR("20nit 120hz: b_2 = 0");
		}
		r_1 = r_3 - 24;
		if (r_1 < 0) {
			r_1 = 0;
			DSI_ERR("20nit 120hz: r_1 = 0");
		}
		g_1 = g_3 - 16;
		if (g_1 < 0) {
			g_1 = 0;
			DSI_ERR("20nit 120hz: g_1 = 0");
		}
		b_1 = b_3 - 38;
		if (b_1 < 0) {
			b_1 = 0;
			DSI_ERR("20nit 120hz: b_1 = 0");
		}
		dimming_gamma_120hz[24] = (dimming_gamma_120hz[24] & 0xC0) | (r_1 >> 4);
		dimming_gamma_120hz[25] = ((r_1 & 0x00F) << 4) | (g_1 >> 6);
		dimming_gamma_120hz[26] = ((g_1 & 0x03F) << 2) | (b_1 >> 8);
		dimming_gamma_120hz[27] = b_1 & 0x0FF;
		dimming_gamma_120hz[28] = (dimming_gamma_120hz[28] & 0xC0) | (r_2 >> 4);
		dimming_gamma_120hz[29] = ((r_2 & 0x00F) << 4) | (g_2 >> 6);
		dimming_gamma_120hz[30] = ((g_2 & 0x03F) << 2) | (b_2 >> 8);
		dimming_gamma_120hz[31] = b_2 & 0x0FF;
		for (i = 24; i < 32; i++) {
		DSI_ERR("dimming_gamma_120hz_20nit[%d] = 0x%02x", i, dimming_gamma_120hz[i]);
		}
		for (i = 0; i < 12; i++)
			dimming_gamma_120hz[36+i] = b9_register_value[45+i];
		r_3 = ((dimming_gamma_120hz[44] & 0x3F) << 4) | ((dimming_gamma_120hz[45] & 0xF0) >> 4);
		if (r_3 < 0) {
			r_3 = 0;
			DSI_ERR("10nit 120hz: r_3 = 0");
		}
		g_3 = ((dimming_gamma_120hz[45] & 0x0F) << 6) | ((dimming_gamma_120hz[46] & 0xFC) >> 2);
		if (g_3 < 0) {
			g_3 = 0;
			DSI_ERR("10nit 120hz: g_3 = 0");
		}
		b_3 = ((dimming_gamma_120hz[46] & 0x03) << 8) | dimming_gamma_120hz[47];
		if (b_3 < 0) {
			b_3 = 0;
			DSI_ERR("10nit 120hz: b_3 = 0");
		}
		r_2 = r_3 - 12;
		if (r_2 < 0) {
			r_2 = 0;
			DSI_ERR("10nit 120hz: r_2 = 0");
		}
		g_2 = g_3 - 6;
		if (g_2 < 0) {
			g_2 = 0;
			DSI_ERR("10nit 120hz: g_2 = 0");
		}
		b_2 = b_3 - 18;
		if (b_2 < 0) {
			b_2 = 0;
			DSI_ERR("10nit 120hz: b_2 = 0");
		}
		r_1 = r_3 - 27;
		if (r_1 < 0) {
			r_1 = 0;
			DSI_ERR("10nit 120hz: r_1 = 0");
		}
		g_1 = g_3 - 16;
		if (g_1 < 0) {
			g_1 = 0;
			DSI_ERR("10nit 120hz: g_1 = 0");
		}
		b_1 = b_3 - 32;
		if (b_1 < 0) {
			b_1 = 0;
			DSI_ERR("10nit 120hz: b_1 = 0");
		}
		dimming_gamma_120hz[36] = (dimming_gamma_120hz[36] & 0xC0) | (r_1 >> 4);
		dimming_gamma_120hz[37] = ((r_1 & 0x00F) << 4) | (g_1 >> 6);
		dimming_gamma_120hz[38] = ((g_1 & 0x03F) << 2) | (b_1 >> 8);
		dimming_gamma_120hz[39] = b_1 & 0x0FF;
		dimming_gamma_120hz[40] = (dimming_gamma_120hz[40] & 0xC0) | (r_2 >> 4);
		dimming_gamma_120hz[41] = ((r_2 & 0x00F) << 4) | (g_2 >> 6);
		dimming_gamma_120hz[42] = ((g_2 & 0x03F) << 2) | (b_2 >> 8);
		dimming_gamma_120hz[43] = b_2 & 0x0FF;
		for (i = 36; i < 44; i++) {
		DSI_ERR("dimming_gamma_120hz_10nit[%d] = 0x%02x", i, dimming_gamma_120hz[i]);
		}
		/* GAMMA change Setting (120Hz) 80-500*/
		for (i = 0; i < 8; i++){
		dimming_gamma_120hz_500step[i] = b9_register_value_500step[221+i];
			}
		r_3 = ((dimming_gamma_120hz_500step[0] & 0x3F) << 4) | ((dimming_gamma_120hz_500step[1] & 0xF0) >> 4);
		if (r_3 < 0) {
		r_3 = 0;
		DSI_ERR("120hz: r_3 = 0");
		}
		g_3 = ((dimming_gamma_120hz_500step[1] & 0x0F) << 6) | ((dimming_gamma_120hz_500step[2] & 0xFC) >> 2);
		if (g_3 < 0) {
		g_3 = 0;
		DSI_ERR("120hz: g_3 = 0");
		}
		b_3 = ((dimming_gamma_120hz_500step[2] & 0x03) << 8) | dimming_gamma_120hz_500step[3];
		if (b_3 < 0) {
		b_3 = 0;
		DSI_ERR("120hz: b_3 = 0");
		}
		r_4 = ((dimming_gamma_120hz_500step[4] & 0x3F) << 4) | ((dimming_gamma_120hz_500step[5] & 0xF0) >> 4);
		if (r_4 < 0) {
		r_4 = 0;
		DSI_ERR("120hz: r_4 = 0");
		}
		g_4 = ((dimming_gamma_120hz_500step[5] & 0x0F) << 6) | ((dimming_gamma_120hz_500step[6] & 0xFC) >> 2);
		if (g_4 < 0) {
		g_4 = 0;
		DSI_ERR("120hz: g_4 = 0");
		}
		b_4 = ((dimming_gamma_120hz_500step[6] & 0x03) << 8) | dimming_gamma_120hz_500step[7];
		if (b_4 < 0) {
		b_4 = 0;
		DSI_ERR(" 120hz: b_4 = 0");
		}
		r_3 = r_3 - 3;
		if (r_3 < 0) {
		r_3 = 0;
		DSI_ERR("after modify 120hz: R_3 = 0");
		}
		g_3 = g_3 - 5;
		if (g_3 < 0) {
		g_3 = 0;
		DSI_ERR("after modify 120hz: g_3 = 0");
		}
		b_3 = b_3 - 6;
		if (b_3 < 0) {
		b_3 = 0;
		DSI_ERR("after modify  120hz: b_3 = 0");
		}
		dimming_gamma_120hz_500step[0] = (dimming_gamma_120hz_500step[0] & 0xC0) | (r_3 >> 4);
		dimming_gamma_120hz_500step[1] = ((r_3 & 0x00F) << 4) | (g_3 >> 6);
		dimming_gamma_120hz_500step[2] = ((g_3 & 0x03F) << 2) | (b_3 >> 8);
		dimming_gamma_120hz_500step[3] = b_3 & 0x0FF;
		dimming_gamma_120hz_500step[4] = (dimming_gamma_120hz_500step[4] & 0xC0) | (r_4 >> 4);
		dimming_gamma_120hz_500step[5] = ((r_4 & 0x00F) << 4) | (g_4 >> 6);
		dimming_gamma_120hz_500step[6] = ((g_4 & 0x03F) << 2) | (b_4 >> 8);
		dimming_gamma_120hz_500step[7] = b_4 & 0x0FF;
		for (i = 0; i < 8; i++) {
		DSI_ERR("dimming_gamma_120hz_500step[%d] = 0x%02x", i, dimming_gamma_120hz_500step[i]);
		}
	}
}

int dsi_panel_dimming_gamma_write(struct dsi_panel *panel)
{
	int i = 0;
	int rc = 0;
	int count = 0;
	unsigned char *payload;
	struct dsi_display_mode *mode;
	struct dsi_cmd_desc *cmds;

	if (!panel || !panel->cur_mode) {
		DSI_ERR("Invalid params\n");
		return -EINVAL;
	}

	if (strcmp(panel->name, "samsung amb655x fhd cmd mode dsc dsi panel") != 0) {
		return 0;
	}

	mode = panel->cur_mode;

	count = mode->priv_info->cmd_sets[DSI_CMD_SET_GAMMA_CHANGE_WRITE].count;
	if (!count) {
		DSI_ERR("This panel does not support gamma change write command\n");
		return -EINVAL;
	} else {
		cmds = mode->priv_info->cmd_sets[DSI_CMD_SET_GAMMA_CHANGE_WRITE].cmds;
		DSI_ERR("Send GAMMA_CHANGE_WRITE cmds start\n");
		if(strcmp(panel->name, "samsung amb655x fhd cmd mode dsc dsi panel") == 0) {
		/* GAMMA change Setting 80nit 40nit 20nit 10nit  (120Hz) */
		payload = (u8 *)cmds[2].msg.tx_buf;
		for (i = 1; i < 9; i++)
			payload[i] = dimming_gamma_120hz[i-1];
		payload = (u8 *)cmds[4].msg.tx_buf;
		for (i = 1; i < 9; i++)
			payload[i] = dimming_gamma_120hz[i+11];
		payload = (u8 *)cmds[6].msg.tx_buf;
		for (i = 1; i < 9; i++)
			payload[i] = dimming_gamma_120hz[i+23];
		payload = (u8 *)cmds[8].msg.tx_buf;
		for (i = 1; i < 9; i++)
			payload[i] = dimming_gamma_120hz[i+35];
		/* GAMMA change Setting (120Hz) 80-500nit */
		payload = (u8 *)cmds[10].msg.tx_buf;
		for (i = 1; i < 9; i++)
			payload[i] = dimming_gamma_120hz_500step[i-1];
		}
		rc = dsi_panel_tx_cmd_set(panel, DSI_CMD_SET_GAMMA_CHANGE_WRITE);
		if (rc) {
			DSI_ERR("Failed to send DSI_CMD_SET_GAMMA_CHANGE_WRITE commands\n");
		}
		DSI_ERR("Send GAMMA_CHANGE_WRITE cmds end\n");
	}

	return rc;
}
int dsi_panel_update_cmd_sets_sub(struct dsi_panel_cmd_set *cmd,
					enum dsi_cmd_set_type type, const char *data, unsigned int length)
{
	int i = 0;
	int rc = 0;
	u32 packet_count = 0;

	if (!data) {
		DSI_ERR("%s commands not defined\n", cmd_set_prop_map[type]);
		rc = -ENOTSUPP;
		goto error;
	}

	DSI_ERR("type=%d, name=%s, length=%d\n", type,
		cmd_set_prop_map[type], length);

	print_hex_dump_debug("", DUMP_PREFIX_NONE,
		       8, 1, data, length, false);

	for (i = 0; i < length; i++) {
		DSI_ERR("data[%d]=%02X", i, data[i]);
	}
	rc = dsi_panel_get_cmd_pkt_count(data, length, &packet_count);
	if (rc) {
		DSI_ERR("commands failed, rc=%d\n", rc);
		goto error;
	}
	DSI_ERR("[%s] packet-count=%d, %d\n", cmd_set_prop_map[type],
		packet_count, length);

	for (i = 0; i < cmd->count; i++) {
		kfree(cmd->cmds[i].msg.tx_buf);
	}
	kfree(cmd->cmds);
	cmd->cmds = NULL;
	DSI_ERR("Free tx_buf and dsi_cmd_desc struct pointers done.");

	rc = dsi_panel_alloc_cmd_packets(cmd, packet_count);
	if (rc) {
		DSI_ERR("failed to allocate cmd packets, rc=%d\n", rc);
		goto error;
	}

	rc = dsi_panel_create_cmd_packets(data, length, packet_count,
					  cmd->cmds);
	if (rc) {
		DSI_ERR("failed to create cmd packets, rc=%d\n", rc);
		goto error_free_mem;
	}

	return rc;

error_free_mem:
	kfree(cmd->cmds);
	cmd->cmds = NULL;

error:
	return rc;

}

int dsi_panel_update_dsi_seed_command(struct dsi_cmd_desc *cmds,
					enum dsi_cmd_set_type type, const char *data)
{
	int i = 0;
	int rc = 0;
	u8 *payload;

	if (!data) {
		DSI_ERR("%s commands not defined\n", cmd_set_prop_map[type]);
		rc = -ENOTSUPP;
		goto error;
	}

	payload = (u8 *)cmds[3].msg.tx_buf;
	for (i = 0; i < 0x16; i++)
		payload[i] = data[i];

error:
	return rc;
}
