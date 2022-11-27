// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#include "oplus_switching.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include "../oplus_charger.h"
#include "../oplus_vooc.h"

#define VBAT_GAP_STATUS1	800
#define VBAT_GAP_STATUS2	600
#define VBAT_GAP_STATUS3	150
#define VBAT_GAP_STATUS7	400

struct oplus_switch_chip * g_switching_chip;
extern struct oplus_chg_chip* oplus_chg_get_chg_struct(void);

int oplus_switching_get_error_status(void)
{
	if (!g_switching_chip || g_switching_chip->switch_ops ||
	    g_switching_chip->switch_ops->switching_get_fastcharge_current) {
		return 0;
	}

	if (g_switching_chip->error_status) {
		chg_err("error_status:%d\n", g_switching_chip->error_status);
		return g_switching_chip->error_status;
	} else {
		g_switching_chip->switch_ops->switching_get_fastcharge_current();
		chg_err("error_status:%d\n", g_switching_chip->error_status);
		return g_switching_chip->error_status;
	}

	return 0;
}

int oplus_switching_hw_enable(int en)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops ||
	    !g_switching_chip->switch_ops->switching_hw_enable) {
		return -1;
	}

	return g_switching_chip->switch_ops->switching_hw_enable(en);
}

int oplus_switching_set_fastcharge_current(int curr_ma)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops ||
	    !g_switching_chip->switch_ops->switching_set_fastcharge_current) {
		return -1;
	}

	return g_switching_chip->switch_ops->switching_set_fastcharge_current(curr_ma);

}

int oplus_switching_enable_charge(int en)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops ||
	    !g_switching_chip->switch_ops->switching_enable_charge) {
		return -1;
	} else {
		chg_err("success\n");
		return g_switching_chip->switch_ops->switching_enable_charge(en);
	}
}

bool oplus_switching_get_hw_enable(void)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops
		|| !g_switching_chip->switch_ops->switching_get_hw_enable) {
		return -1;
	} else {
		return g_switching_chip->switch_ops->switching_get_hw_enable();
	}
}

bool oplus_switching_get_charge_enable(void)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops
		|| !g_switching_chip->switch_ops->switching_get_charge_enable) {
		return -1;
	} else {
		return g_switching_chip->switch_ops->switching_get_charge_enable();
	}
}

int oplus_switching_get_fastcharge_current(void)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops
		|| !g_switching_chip->switch_ops->switching_get_fastcharge_current) {
		return -1;
	} else {
		return g_switching_chip->switch_ops->switching_get_fastcharge_current();
	}
}

int oplus_switching_get_discharge_current(void)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops
		|| !g_switching_chip->switch_ops->switching_get_discharge_current) {
		return -1;
	} else {
		return g_switching_chip->switch_ops->switching_get_discharge_current();
	}
}

int oplus_switching_set_current(int current_ma)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops
		|| !g_switching_chip->switch_ops->switching_set_fastcharge_current) {
		return -1;
	}

	chg_err("current_ma:%d\n", current_ma);
	g_switching_chip->switch_ops->switching_set_fastcharge_current(current_ma);

	return 0;
}

int oplus_switching_set_discharge_current(int current_ma)
{
	if (!g_switching_chip || !g_switching_chip->switch_ops
		|| !g_switching_chip->switch_ops->switching_set_discharge_current) {
		return -1;
	}

	chg_err("current_ma:%d\n", current_ma);
	g_switching_chip->switch_ops->switching_set_discharge_current(current_ma);

	return 0;
}

int oplus_switching_get_if_need_balance_bat(int vbat0_mv, int vbat1_mv)
{
	int diff_volt = 0;
	struct oplus_chg_chip *chip = oplus_chg_get_chg_struct();
	static int error_count = 0;
	static int pre_error_reason = 0;
	int error_reason = 0;

	chg_err("vbat0_mv:%d, vbat1_mv:%d\n", vbat0_mv, vbat1_mv);
	if (!g_switching_chip) {
		chg_err("fail\n");
		return -1;
	} else {
		diff_volt = abs(vbat0_mv - vbat1_mv);
		if (chip->sub_batt_temperature == FG_I2C_ERROR || chip->temperature == FG_I2C_ERROR) {
			error_reason |= REASON_I2C_ERROR;
		}

		if (oplus_switching_get_error_status()
		    && oplus_switching_support_parallel_chg() == PARALLEL_SWITCH_IC) {
			return PARALLEL_BAT_BALANCE_ERROR_STATUS8;
		}

		if (oplus_vooc_get_fastchg_started() == true) {
			if (oplus_switching_get_hw_enable() &&
			    (abs(chip->sub_batt_icharging) < g_switching_chip->parallel_mos_abnormal_litter_curr ||
			    abs(chip->icharging) < g_switching_chip->parallel_mos_abnormal_litter_curr) &&
			    (abs(chip->icharging - chip->sub_batt_icharging) >= g_switching_chip->parallel_mos_abnormal_gap_curr)) {
				if (error_count < BATT_OPEN_RETRY_COUNT) {
					error_count++;
				} else {
					error_reason |= REASON_MOS_OPEN_ERROR;
				}
			} else {
				error_count = 0;
			}
		}

		if (oplus_switching_support_parallel_chg() == PARALLEL_MOS_CTRL) {
			if (chip->tbatt_status != BATTERY_STATUS__WARM_TEMP
			    && (chip->sw_sub_batt_full || chip->hw_sub_batt_full_by_sw)
			    && !(chip->sw_full || chip->hw_full_by_sw || chip->batt_full)
			    && chip->charger_exist) {
		    		error_reason |= REASON_SUB_BATT_FULL;
			}
			if (diff_volt >= g_switching_chip->parallel_vbat_gap_abnormal) {
				error_reason |= REASON_VBAT_GAP_BIG;
			}
			if ((pre_error_reason & REASON_SUB_BATT_FULL)
			    && !oplus_switching_get_hw_enable()
			    && diff_volt > g_switching_chip->parallel_vbat_gap_full) {
				error_reason &= ~REASON_SUB_BATT_FULL;
				chg_err("sub full,but diff_volt > %d need to recovery MOS\n", g_switching_chip->parallel_vbat_gap_full);
			}
			if ((pre_error_reason & REASON_VBAT_GAP_BIG)
			    && diff_volt > g_switching_chip->parallel_vbat_gap_recov) {
				error_reason |= REASON_VBAT_GAP_BIG;
			}
		}

		if (error_reason != 0) {
			pre_error_reason = error_reason;
			chg_err("mos open %d\n", error_reason);
			if ((error_reason & (REASON_I2C_ERROR | REASON_MOS_OPEN_ERROR)) != 0) {
				return PARALLEL_BAT_BALANCE_ERROR_STATUS8;
			}
			return PARALLEL_BAT_BALANCE_ERROR_STATUS9;
		} else if (oplus_switching_support_parallel_chg() == PARALLEL_MOS_CTRL) {
			pre_error_reason = error_reason;
			return PARALLEL_NOT_NEED_BALANCE_BAT__START_CHARGE;
		}

		if (diff_volt < VBAT_GAP_STATUS3) {
			return PARALLEL_NOT_NEED_BALANCE_BAT__START_CHARGE;
		}

		if (vbat0_mv >= 3400 && vbat1_mv < 3400) {
			if (vbat1_mv < 3100) {
				if (vbat0_mv - vbat1_mv <= 1000) {
					return PARALLEL_NEED_BALANCE_BAT_STATUS5__STOP_CHARGE;
				} else {
					return PARALLEL_BAT_BALANCE_ERROR_STATUS6__STOP_CHARGE;
				}
			} else {
				return PARALLEL_NEED_BALANCE_BAT_STATUS5__STOP_CHARGE;
			}
		} else if (vbat0_mv >= vbat1_mv) {
			if (diff_volt >= VBAT_GAP_STATUS1) {
				return PARALLEL_NEED_BALANCE_BAT_STATUS1__STOP_CHARGE;
			} else if (diff_volt >= VBAT_GAP_STATUS2) {
				return PARALLEL_NEED_BALANCE_BAT_STATUS2__STOP_CHARGE;
			} else if (diff_volt >= VBAT_GAP_STATUS3) {
				return PARALLEL_NEED_BALANCE_BAT_STATUS3__STOP_CHARGE;
			} else {
				return PARALLEL_NOT_NEED_BALANCE_BAT__START_CHARGE;
			}
		} else if (vbat0_mv < vbat1_mv) {
			if (diff_volt >= VBAT_GAP_STATUS7) {
				return PARALLEL_NEED_BALANCE_BAT_STATUS7__START_CHARGE;
			} else if (diff_volt >= VBAT_GAP_STATUS3) {
				return PARALLEL_NEED_BALANCE_BAT_STATUS4__STOP_CHARGE;
			} else {
				return PARALLEL_NOT_NEED_BALANCE_BAT__START_CHARGE;
			}
		}
	}
	return -1;
}

int oplus_switching_set_balance_bat_status(int status)
{
	chg_err("status:%d\n", status);
	switch (status) {
	case PARALLEL_NOT_NEED_BALANCE_BAT__START_CHARGE:
		oplus_switching_hw_enable(1);
		oplus_switching_set_discharge_current(2800);
		oplus_switching_set_current(2800);
		oplus_switching_enable_charge(1);
		break;
	case PARALLEL_NEED_BALANCE_BAT_STATUS1__STOP_CHARGE:
		oplus_switching_hw_enable(1);
		oplus_switching_set_discharge_current(500);
		oplus_switching_set_current(500);
		oplus_switching_enable_charge(1);
		break;
	case PARALLEL_NEED_BALANCE_BAT_STATUS2__STOP_CHARGE:
		oplus_switching_hw_enable(1);
		oplus_switching_set_discharge_current(1000);
		oplus_switching_set_current(1000);
		oplus_switching_enable_charge(1);
		break;
	case PARALLEL_NEED_BALANCE_BAT_STATUS3__STOP_CHARGE:
		oplus_switching_hw_enable(1);
		oplus_switching_set_discharge_current(2800);
		oplus_switching_set_current(2800);
		oplus_switching_enable_charge(1);
		break;
	case PARALLEL_NEED_BALANCE_BAT_STATUS4__STOP_CHARGE:
		oplus_switching_hw_enable(1);
		oplus_switching_set_discharge_current(2800);
		oplus_switching_set_current(2800);
		oplus_switching_enable_charge(1);
		break;
	case PARALLEL_NEED_BALANCE_BAT_STATUS5__STOP_CHARGE:
		oplus_switching_hw_enable(1);
		oplus_switching_set_discharge_current(200);
		oplus_switching_set_current(200);
		oplus_switching_enable_charge(1);
		break;
	case PARALLEL_BAT_BALANCE_ERROR_STATUS6__STOP_CHARGE:
		oplus_switching_hw_enable(0);
		break;
	case PARALLEL_NEED_BALANCE_BAT_STATUS7__START_CHARGE:
		oplus_switching_hw_enable(0);
		break;
	case PARALLEL_BAT_BALANCE_ERROR_STATUS8:
	case PARALLEL_BAT_BALANCE_ERROR_STATUS9:
		oplus_switching_hw_enable(0);
		break;
	default:
		break;
	}
	return 0;
}

static int oplus_switching_parse_dt(struct oplus_switch_chip *chip)
{
	int rc = 0;
	struct device_node *node = NULL;

	if (!chip || !chip->dev) {
		chg_err("oplus_mos_dev null!\n");
		return -1;
	}

	node = chip->dev->of_node;

	rc = of_property_read_u32(node, "qcom,parallel_vbat_gap_abnormal", &chip->parallel_vbat_gap_abnormal);
	if (rc) {
		chip->parallel_vbat_gap_abnormal = 150;
	}

	rc = of_property_read_u32(node, "qcom,parallel_vbat_gap_full", &chip->parallel_vbat_gap_full);
	if (rc) {
		chip->parallel_vbat_gap_full = 200;
	}

	rc = of_property_read_u32(node, "qcom,parallel_vbat_gap_recov", &chip->parallel_vbat_gap_recov);
	if (rc) {
		chip->parallel_vbat_gap_recov = 100;
	}

	rc = of_property_read_u32(node, "qcom,parallel_mos_abnormal_litter_curr", &chip->parallel_mos_abnormal_litter_curr);
	if (rc) {
		chip->parallel_mos_abnormal_litter_curr = 100;
	}

	rc = of_property_read_u32(node, "qcom,parallel_mos_abnormal_gap_curr", &chip->parallel_mos_abnormal_gap_curr);
	if (rc) {
		chip->parallel_mos_abnormal_gap_curr = 2000;
	}
	chg_err("parallel_vbat_gap_abnormal %d,"
		"parallel_vbat_gap_full %d,"
		"parallel_vbat_gap_recov %d,"
		"parallel_mos_abnormal_litter_curr %d,"
		"parallel_mos_abnormal_gap_curr %d \n",
		chip->parallel_vbat_gap_abnormal, chip->parallel_vbat_gap_full,
		chip->parallel_vbat_gap_recov, chip->parallel_mos_abnormal_litter_curr,
		chip->parallel_mos_abnormal_gap_curr);
	return 0;
}

void oplus_switching_init(struct oplus_switch_chip *chip, int type)
{
	if (!chip) {
		chg_err("oplus_switch_chip not specified!\n");
		return;
	}

	g_switching_chip = chip;
	g_switching_chip->ctrl_type = type;

	chg_err("ctrl_type: %d\n", g_switching_chip->ctrl_type);

	oplus_switching_parse_dt(chip);
}

int oplus_switching_support_parallel_chg(void)
{
	if (!g_switching_chip) {
		return NO_PARALLEL_TYPE;
        } else {
		return g_switching_chip->ctrl_type;
	}
}
