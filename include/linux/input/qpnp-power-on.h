/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2012-2015, 2017-2019, The Linux Foundation.
 * All rights reserved.
 */

#ifndef QPNP_PON_H
#define QPNP_PON_H

#include <dt-bindings/input/qcom,qpnp-power-on.h>
#include <linux/errno.h>
#include <linux/types.h>

/**
 * enum pon_trigger_source: List of PON trigger sources
 * %PON_SMPL:		PON triggered by Sudden Momentary Power Loss (SMPL)
 * %PON_RTC:		PON triggered by real-time clock (RTC) alarm
 * %PON_DC_CHG:		PON triggered by insertion of DC charger
 * %PON_USB_CHG:	PON triggered by insertion of USB
 * %PON_PON1:		PON triggered by other PMIC (multi-PMIC option)
 * %PON_CBLPWR_N:	PON triggered by power-cable insertion
 * %PON_KPDPWR_N:	PON triggered by long press of the power-key
 */
enum pon_trigger_source {
	PON_SMPL = 1,
	PON_RTC,
	PON_DC_CHG,
	PON_USB_CHG,
	PON_PON1,
	PON_CBLPWR_N,
	PON_KPDPWR_N,
};

/**
 * enum pon_power_off_type: Possible power off actions to perform
 * %PON_POWER_OFF_RESERVED:          Reserved, not used
 * %PON_POWER_OFF_WARM_RESET:        Reset the MSM but not all PMIC peripherals
 * %PON_POWER_OFF_SHUTDOWN:          Shutdown the MSM and PMIC completely
 * %PON_POWER_OFF_HARD_RESET:        Reset the MSM and all PMIC peripherals
 * %PON_POWER_OFF_MAX_TYPE:          Reserved, not used
 */
enum pon_power_off_type {
	PON_POWER_OFF_RESERVED		= 0x00,
	PON_POWER_OFF_WARM_RESET	= PON_POWER_OFF_TYPE_WARM_RESET,
	PON_POWER_OFF_SHUTDOWN		= PON_POWER_OFF_TYPE_SHUTDOWN,
	PON_POWER_OFF_HARD_RESET	= PON_POWER_OFF_TYPE_HARD_RESET,
	PON_POWER_OFF_MAX_TYPE		= 0x10,
};

enum pon_restart_reason {
	PON_RESTART_REASON_UNKNOWN		= 0x00,
	PON_RESTART_REASON_RECOVERY		= 0x01,
	PON_RESTART_REASON_BOOTLOADER		= 0x02,
	PON_RESTART_REASON_RTC			= 0x03,
	PON_RESTART_REASON_DMVERITY_CORRUPTED	= 0x04,
	PON_RESTART_REASON_DMVERITY_ENFORCE	= 0x05,
	PON_RESTART_REASON_KEYS_CLEAR		= 0x06,
#ifdef VENDOR_EDIT
	PON_RESTART_REASON_SILENCE			= 0x21,
	PON_RESTART_REASON_SAU				= 0x22,
	PON_RESTART_REASON_RF				= 0x23,
	PON_RESTART_REASON_WLAN				= 0x24,
	PON_RESTART_REASON_MOS				= 0x25,
	PON_RESTART_REASON_FACTORY			= 0x26,
	PON_RESTART_REASON_KERNEL			= 0x27,
	PON_RESTART_REASON_MODEM			= 0x28,
	PON_RESTART_REASON_ANDROID			= 0x29,
	PON_RESTART_REASON_SAFE				= 0x2A,
	//#ifdef OPLUS_FEATURE_AGINGTEST  
	/*xiaofan.yang, 2019/01/07,Add for factory agingtest*/
	PON_RESTART_REASON_SBL_DDRTEST                  = 0x2B,
	PON_RESTART_REASON_SBL_DDR_CUS                  = 0x2C,
	PON_RESTART_REASON_MEM_AGING                    = 0x2D,
	/*0x2E is SBLTEST FAIL, just happen in ddrtest fail when xbl setup*/
	//#endif/*OPLUS_FEATURE_AGINGTEST*/
	PON_RESTART_REASON_BOOT_NO_VIBRATION            = 0x2F,
	PON_RESTART_REASON_NORMAL			= 0x3E,
#endif /*VENDOR_EDIT*/
};

#if IS_ENABLED(CONFIG_INPUT_QPNP_POWER_ON)
int qpnp_pon_system_pwr_off(enum pon_power_off_type type);
int qpnp_pon_is_warm_reset(void);
int qpnp_pon_trigger_config(enum pon_trigger_source pon_src, bool enable);
int qpnp_pon_wd_config(bool enable);
int qpnp_pon_set_restart_reason(enum pon_restart_reason reason);
bool qpnp_pon_check_hard_reset_stored(void);
int qpnp_pon_modem_pwr_off(enum pon_power_off_type type);

#else

static int qpnp_pon_system_pwr_off(enum pon_power_off_type type)
{
	return -ENODEV;
}

static inline int qpnp_pon_is_warm_reset(void)
{
	return -ENODEV;
}

static inline int qpnp_pon_trigger_config(enum pon_trigger_source pon_src,
							bool enable)
{
	return -ENODEV;
}

int qpnp_pon_wd_config(bool enable)
{
	return -ENODEV;
}

static inline int qpnp_pon_set_restart_reason(enum pon_restart_reason reason)
{
	return -ENODEV;
}

static inline bool qpnp_pon_check_hard_reset_stored(void)
{
	return false;
}

static inline int qpnp_pon_modem_pwr_off(enum pon_power_off_type type)
{
	return -ENODEV;
}

#endif

#endif
