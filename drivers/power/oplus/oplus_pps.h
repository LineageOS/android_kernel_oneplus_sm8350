// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2018-2020 Oplus. All rights reserved.
 */
#ifndef _OPLUS_PPS_H_
#define _OPLUS_PPS_H_
#include <linux/time.h>
#include <linux/list.h>
#ifndef CONFIG_OPLUS_CHARGER_MTK
#include <linux/usb/typec.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
#include <linux/usb/usbpd.h>
#endif
#endif
#include <linux/random.h>
#include <linux/device.h>
#include <linux/types.h>
#define pps_err(fmt, ...) \
        printk(KERN_ERR "[OPLUS_PPS][%s]"fmt, __func__, ##__VA_ARGS__)
#define OPLUS_CHG_UPDATE_PPS_DELAY            round_jiffies_relative(msecs_to_jiffies(500))
#define PD_PPS_STATUS_VOLT(pps_status)        (((pps_status) >> 0) & 0xFFFF)
#define PD_PPS_STATUS_CUR(pps_status)         (((pps_status) >> 16) & 0xFF)

#define OPLUS_PPS_10V_STATUS_1               12
#define OPLUS_PPS_10V_STATUS_2               6

#define OPLUS_PPS_20V_STATUS_1               20
#define OPLUS_PPS_20V_STATUS_2               6
#define PPS_KEY_NUMBER                       4
#define PPS_CURVE_65W                        CURVE_2

#define PPS_BYPASS_MODE                      0
#define PPS_SC_MODE                          1

#define PPS_CURRENT_MAX               		8000
#define PPS_CURRENT_3A               		3000
#define PPS_CURRENT_2A               		2000
#define OVER_CURRENT_VALUE_125W                   17000
#define OVER_CURRENT_VALUE_30W                    4000
#define PPS_START_125W_CURR_THD                   3500

#define PPS_TIMEOUT_30W                     9000
#define PPS_TIMEOUT_125W                    7200
#define PPS_VOLT_STEP                       20
#define PPS_CURR_STEP                       50

#define R1_LIMIT                             50
#define R2_LIMIT                             20
#define RMOS_MOHM                            8
#define CABLE_CURRENT_LIMIT                  1000

#define BCC_CURRENT_MIN               		(1000/100)
#define PPS_CP_TDIE_OVER_COUNTS				5
#define PPS_CP_TDIE_MAX						110

#define BATT_PPS_SYS_MAX                     40
#define FULL_PPS_SYS_MAX                     6

#define TEMP_DELTAT                          10
#define PPS_R_AVG_NUM                        10
#define PPS_R_ROW_NUM                        7

#define UPDATE_VCP_TIME                      1
#define UPDATE_PDO_TIME                      5
#define UPDATE_FASTCHG_TIME                  1
#define UPDATE_TEMP_TIME                     1
#define UPDATE_IBAT_TIME                     1
#define UPDATE_BCC_TIME                 	2

#define PPS_DUMP_REG_CNT 10
#define OPLUS_EXTEND_IMIN		6250
#define OPLUS_EXTEND_VMIN		20000
#define OPLUS_PPS_IMIN			5000
#define OPLUS_PPS_POWER_V2		0X96
#define OPLUS_PPS_POWER_V1		0X7D
#define OPLUS_PPS_POWER_CLR		0X0
#define CP_DIFF_LIMIT                        1000
#define CP_OCP_LIMIT                         4500
#define CP_NORMAL_IBUS_LIMIT                 8000
#define CP_ABNORMAL_IBUS_LIMIT               3000
#define CP_DISABLE_CURRENT                   250

#define R_COOL_DOWN_LEVEL_DEFAULT            8000
#define R_COOL_DOWN_LEVEL_1                  3000
#define R_COOL_DOWN_LEVEL_2                  2000
#define PPS_COOL_DOWN_MAX_NUM                12

#define SINGLE_CP_CURRENT_THD                1300
#define MULT_CP_CURRENT_THD                  1700

#define ADJUST_VOLT_THD_1                    2000
#define ADJUST_VOLT_THD_2                    1000
#define ADJUST_VOLT_THD_3                    500
#define ADJUST_VOLT_THD_4                    300
#define ADJUST_VOLT_STEP_1                   2000
#define ADJUST_VOLT_STEP_2                   1000
#define ADJUST_VOLT_STEP_3                   500
#define ADJUST_VOLT_STEP_4                   100

#define ADJUST_CURR_THD_1                    300
#define ADJUST_CURR_STEP_1                   300
#define ADJUST_CURR_STEP_2                   50

#define DEFAULT_CHARGER_VOLT                 5500
#define DEFAULT_CHARGER_CURRENT              1000
#define DEFAULT_CHARGER_LOW_CURRENT          500
#define DISCONNECT_CURRENT_THD               300
#define VBATT_VOLT                           10000
#define UC_LIMINT                            1000
#define EMERGENCY_TEMP                       85
#define IBAT_LOW_THD                         2000

#define HIGH_VOLT_CURVE                      20000
#define LOW_VOLT_CURVE                       10000

#define INTERVAL_PERIOD_PPS_START            200
#define INTERVAL_PERIOD_OPEN_MOS             50
#define INTERVAL_PERIOD_VOLT_CHANGE          200
#define INTERVAL_PERIOD_CUR_CHANGE           200
#define INTERVAL_PERIOD_STATUS_CHECK         500

enum {
	PPS_BAT_TEMP_NATURAL = 0,
	PPS_BAT_TEMP_HIGH0,
	PPS_BAT_TEMP_HIGH1,
	PPS_BAT_TEMP_HIGH2,
	PPS_BAT_TEMP_HIGH3,
	PPS_BAT_TEMP_HIGH4,
	PPS_BAT_TEMP_HIGH5,
	PPS_BAT_TEMP_LOW0,
	PPS_BAT_TEMP_LOW1,
	PPS_BAT_TEMP_LOW2,
	PPS_BAT_TEMP_LITTLE_COOL,
	PPS_BAT_TEMP_LITTLE_COOL_LOW,
	PPS_BAT_TEMP_COOL,
	PPS_BAT_TEMP_NORMAL_LOW, /*13*/
	PPS_BAT_TEMP_NORMAL_HIGH,
	PPS_BAT_TEMP_LITTLE_COLD,
	PPS_BAT_TEMP_WARM,
	PPS_BAT_TEMP_EXIT,
};
enum {
	PPS_TEMP_RANGE_INIT = 0,
	PPS_TEMP_RANGE_LITTLE_COLD,/*0 ~ 5*/
	PPS_TEMP_RANGE_COOL, /*5 ~ 12*/
	PPS_TEMP_RANGE_LITTLE_COOL, /*12~16*/
	PPS_TEMP_RANGE_NORMAL_LOW, /*16~25*/
	PPS_TEMP_RANGE_NORMAL_HIGH, /*25~43*/
	PPS_TEMP_RANGE_WARM, /*43-52*/
	PPS_TEMP_RANGE_NORMAL,
};

enum {
	OPLUS_PPS_STATUS_START = 0,
	OPLUS_PPS_STATUS_OPEN_MOS,
	OPLUS_PPS_STATUS_VOLT_CHANGE,
	OPLUS_PPS_STATUS_CUR_CHANGE,
	OPLUS_PPS_STATUS_CHECK,
	OPLUS_PPS_STATUS_FFC,
};

enum {
	PPS_ADAPTER_10V_30W = 0,
	PPS_ADAPTER_20V_125W,
	PPS_ADAPTER_20V_150W,
	PPS_ADAPTER_20V_200W,
	PPS_ADAPTER_UNKNOWN,
};

enum {
	PPS_NOT_SUPPORT = 0,
	PPS_CHECKING,
	PPS_CHARGERING,
	PPS_CHARGE_END,
};

enum {
	PPS_SUPPORT_NOT = 0,
	PPS_SUPPORT_MCU,
	PPS_SUPPORT_2CP,
	PPS_SUPPORT_VOOCPHY,
};
enum {
	BATT_CURVE_TEMP_RANGE_LITTLE_COLD,/*0 ~ 5*/
	BATT_CURVE_TEMP_RANGE_COOL, /*5 ~ 12*/
	BATT_CURVE_TEMP_RANGE_LITTLE_COOL,/*12~20*/
	BATT_CURVE_TEMP_RANGE_NORMAL_LOW,/*20~35*/
	BATT_CURVE_TEMP_RANGE_NORMAL_HIGH,/*35~43*/
	BATT_CURVE_TEMP_RANGE_WARM,/*43~51*/
	BATT_CURVE_TEMP_RANGE_MAX,
};

enum {
	BATT_CURVE_SOC_RANGE_MIN,
	BATT_CURVE_SOC_RANGE_LOW,
	BATT_CURVE_SOC_RANGE_MID_LOW,
	BATT_CURVE_SOC_RANGE_MID,
	BATT_CURVE_SOC_RANGE_MID_HIGH,
	BATT_CURVE_SOC_RANGE_HIGH,
	BATT_CURVE_SOC_RANGE_MAX,
};

enum {
	LOW_CURR_FULL_CURVE_TEMP_LITTLE_COOL,
	LOW_CURR_FULL_CURVE_TEMP_NORMAL_LOW,
	LOW_CURR_FULL_CURVE_TEMP_NORMAL_HIGH,
	LOW_CURR_FULL_CURVE_TEMP_MAX,
};


typedef enum {
	PPS_STOP_VOTER_NONE = 0,
	PPS_STOP_VOTER_FULL = (1 << 0),
	PPS_STOP_VOTER_BTB_OVER = (1 << 1),
	PPS_STOP_VOTER_TBATT_OVER = (1 << 2),
	PPS_STOP_VOTER_RESISTENSE_OVER = (1 << 3),
	PPS_STOP_VOTER_IBAT_OVER = (1 << 4),
	PPS_STOP_VOTER_DISCONNECT_OVER = (1 << 5),
	PPS_STOP_VOTER_CABLE_OVER = (1 << 6),
	PPS_STOP_VOTER_TIME_OVER = (1 << 7),
	PPS_STOP_VOTER_PDO_ERROR = (1 << 8),
	PPS_STOP_VOTER_TYPE_ERROR = (1 << 9),
	PPS_STOP_VOTER_MMI_TEST = (1 << 10),
	PPS_STOP_VOTER_BATCELL_VOL_DIFF = (1 << 11),
	PPS_STOP_VOTER_TDIE_OVER 			=	(1 << 12),
	PPS_STOP_VOTER_OTHER_ABORMAL = (1 << 15),
} PPS_STOP_VOTER;

struct batt_curve {
	unsigned int target_vbus;
	unsigned int target_vbat;
	unsigned int target_ibus;
	bool exit;
	unsigned int target_time;
};

struct batt_curves {
	struct batt_curve batt_curves[BATT_PPS_SYS_MAX];
	int batt_curve_num;
};

struct batt_curves_soc {
	struct batt_curves batt_curves_temp[BATT_CURVE_TEMP_RANGE_MAX];
};

struct full_curve {
	unsigned int iterm;
	unsigned int vterm;
	bool exit;
};

struct full_curves_temp {
	struct full_curve full_curves[FULL_PPS_SYS_MAX];
	int full_curve_num;
};

typedef struct oplus_pps_r_info
{
	int r0;
	int r1;
	int r2;
	int r3;
	int r4;
	int r5;
	int r6;
} PPS_R_INFO;

typedef struct oplus_pps_r_limit
{
	int limit_exit_mohm;
	int limit_1a_mohm;
	int limit_2a_mohm;
	int limit_3a_mohm;
	int limit_4a_mohm;
} PPS_R_LIMIT;

typedef enum oplus_pps_r_cool_down_ilimit
{
	R_COOL_DOWN_NOLIMIT = 0,
	R_COOL_DOWN_4A,
	R_COOL_DOWN_3A,
	R_COOL_DOWN_2A,
	R_COOL_DOWN_1A,
    R_COOL_DOWN_EXIT,
} PPS_R_COOL_DOWN_ILIMIT;

struct oplus_pps_timer {
	struct timespec pdo_timer;
	struct timespec vcp_timer;
	struct timespec fastchg_timer;
	struct timespec temp_timer;
	struct timespec ibat_timer;
	int batt_curve_time;
	int pps_timeout_time;
	bool set_pdo_flag;
	bool set_vcp_flag;
	bool set_temp_flag;
	bool check_ibat_flag;
	int work_delay;
};

struct pps_protection_counts {
	int cool_fw;
	int sw_full;
	int hw_full;
	int low_curr_full;
	int ibat_low;
	int ibat_high;
	int btb_high;
	int tbatt_over;
	int output_low;
	int cable_over;
	int ibus_over;
	int tdie_over;
	int tdie_exit;
};


struct oplus_pps_limits {
	int default_pps_normal_high_temp;
	int default_pps_little_cool_temp;
	int default_pps_cool_temp;
	int default_pps_little_cold_temp;
	int default_pps_normal_low_temp;
	int pps_warm_allow_vol;
	int pps_warm_allow_soc;

	int pps_batt_over_low_temp;
	int pps_little_cold_temp;
	int pps_cool_temp;
	int pps_little_cool_temp;
	int pps_normal_low_temp;
	int pps_normal_high_temp;
	int pps_batt_over_high_temp;
	int pps_strategy_temp_num;

	int pps_strategy_soc_over_low;
	int pps_strategy_soc_min;
	int pps_strategy_soc_low;
	int pps_strategy_soc_mid_low;
	int pps_strategy_soc_mid;
	int pps_strategy_soc_mid_high;
	int pps_strategy_soc_high;
	int pps_strategy_soc_num;

	int pps_strategy_normal_current;
	int pps_strategy_batt_high_temp0;
	int pps_strategy_batt_high_temp1;
	int pps_strategy_batt_high_temp2;
	int pps_strategy_batt_low_temp2;
	int pps_strategy_batt_low_temp1;
	int pps_strategy_batt_low_temp0;
	int pps_strategy_high_current0;
	int pps_strategy_high_current1;
	int pps_strategy_high_current2;
	int pps_strategy_low_current2;
	int pps_strategy_low_current1;
	int pps_strategy_low_current0;

	int pps_low_curr_full_cool_temp;
	int pps_low_curr_full_little_cool_temp;
	int pps_low_curr_full_normal_low_temp;
	int pps_low_curr_full_normal_high_temp;

	int pps_over_high_or_low_current;
	int pps_strategy_change_count;
	int pps_full_cool_sw_vbat;
	int pps_full_normal_sw_vbat;
	int pps_full_normal_hw_vbat;
	int pps_warm_full_voltage;
	int pps_full_ffc_vbat;
	int pps_full_cool_sw_vbat_30w;
	int pps_full_normal_sw_vbat_30w;
	int pps_full_normal_hw_vbat_30w;
};

struct oplus_pps_chip {
	struct device *dev;
	struct oplus_pps_operations *ops;

	struct list_head temp_list;
	struct power_supply *pps_usb_psy;
	struct notifier_block pps_psy_nb;
	struct delayed_work pps_stop_work;
	struct delayed_work pps_ffc_work;
	struct delayed_work update_pps_work;

/*curve data*/
	struct batt_curves_soc batt_curves_third_soc[BATT_CURVE_SOC_RANGE_MAX];
	struct batt_curves_soc batt_curves_oplus_soc[BATT_CURVE_SOC_RANGE_MAX];
	struct batt_curves batt_curves;
	struct full_curves_temp low_curr_full_curves_temp[LOW_CURR_FULL_CURVE_TEMP_MAX];
	struct oplus_pps_limits limits;
	struct oplus_pps_timer timer;
	PPS_R_INFO r_column[PPS_R_AVG_NUM];
	PPS_R_INFO r_avg;
	PPS_R_INFO r_default;
	PPS_R_LIMIT r_limit;
	int rmos_mohm;

/*pps charging data*/
	struct pps_protection_counts count;
	int batt_input_current;
	int ap_batt_volt;
	int ap_batt_current;
	int ap_batt_soc;
	int ap_batt_temperature;
	int charger_output_volt;
	int charger_output_current;
	int current_adapter_max;
	int ap_input_volt;
	int slave_input_volt;
	int ap_input_current;

	int cp_master_ibus;
	int cp_slave_ibus;
	int cp_master_vac;
	int cp_slave_vac;
	int cp_master_vout;
	int cp_slave_vout;
	int cp_master_tdie;
	int cp_slave_tdie;
/*action data*/
	int target_charger_volt;
	int target_charger_current;
	int ask_charger_volt;
	int ask_charger_current;
	int ask_charger_volt_last;
	int ask_charger_current_last;
	int target_charger_volt_pre;
	int target_charger_current_pre;
/*pps current*/
	int current_batt_curve;
	int current_batt_temp;
	int current_cool_down;
	int current_normal_cool_down;
	int cp_ibus_down;
	int cp_r_down;
	int	current_bcc;
	int	cp_tdie_down;
/*curve data*/
	bool need_change_curve;
	int batt_curve_index;

/*pps status*/
	int pps_fastchg_batt_temp_status;
	int pps_temp_cur_range;
	int pps_low_curr_full_temp_status;
	int pps_chging;
	int pps_power;
	int pps_adapter_type;
	int pps_status;
	int pps_stop_status;
	int pps_support_type;
	int pps_ffc_volt_thd;
	int pps_dummy_started;
	int pps_fastchg_started;

/*other*/
	int cp_slave_enable;
	int cp_pmid2vout_enable;
	int vbat0;
	int cp_master_abnormal;
	int cp_slave_abnormal;
	int pps_iic_err;
	int pps_iic_err_num;
	int master_enable_err_num;
	int bcc_max_curr;
	int bcc_min_curr;
	int bcc_exit_curr;
	int pps_imax;
	int pps_vmax;
	u8  int_column[PPS_DUMP_REG_CNT];
	u8  reg_dump[PPS_DUMP_REG_CNT];
};

struct oplus_pps_operations {
	void (*set_mcu_pps_mode)(bool mode);
	int (*get_mcu_pps_mode)(void);
	int (*get_vbat0_volt)(void);
	int (*check_btb_temp)(void);
	int (*pps_mos_ctrl)(int on);
	int (*pps_get_authentiate)(void);
	void (*pps_cp_hardware_init)(void);
	void (*pps_cp_reset)(void);
	void (*pps_cp_pmid2vout_enable)(bool enable);
	int (*pps_get_cp_master_vbus)(void);
	int (*pps_get_cp_slave_vbus)(void);
	int (*pps_get_cp_master_ibus)(void);
	int (*pps_get_cp_slave_ibus)(void);
	int (*pps_mos_slave_ctrl)(int on);
	int (*pps_get_r_cool_down)(void);
	int (*pps_get_ucp_flag)(void);
	int (*pps_pdo_select)(int vbus_mv, int ibus_ma);
	u32 (*get_pps_status)(void);
	int (*get_pps_max_cur)(int vbus_mv);
	int (*pps_cp_mode_init)(int mode);
	int (*pps_get_cp_master_vac)(void);
	int (*pps_get_cp_slave_vac)(void);
	int (*pps_get_cp_master_vout)(void);
	int (*pps_get_cp_slave_vout)(void);
	int (*pps_get_cp_master_tdie)(void);
	int (*pps_get_cp_slave_tdie)(void);
};

struct oplus_temp_chip {
	struct device *dev;
	struct list_head temp_list;
	/* ep_queue() func will add
	 a request->queue into a udc_ep->queue 'd tail */
	int (*get_temp)(struct device *);
};

struct oplus_pps_chip *oplus_pps_get_pps_chip(void);
int oplus_pps_register_ops(struct oplus_pps_operations *ops);
int oplus_pps_init(struct oplus_chg_chip *chip);
int oplus_pps_get_mcu_pps_mode(void);
void oplus_pps_set_mcu_pps_mode(void);
void oplus_pps_set_mcu_vooc_mode(void);
void oplus_pps_variables_reset(bool in);
int oplus_pps_get_chg_status(void);
int oplus_pps_set_chg_status(int status);
int oplus_pps_start(int authen);
void oplus_pps_hardware_init(void);
void oplus_pps_cp_reset(void);
void oplus_pps_stop_disconnect(void);
void oplus_pps_set_mmi_status(bool mmi);
void oplus_pps_set_batcell_vol_diff_status(bool diff);
bool oplus_is_pps_charging(void);
void oplus_pps_set_power(int pps_ability, int imax, int vmax);
int oplus_pps_get_power(void);
bool oplus_pps_check_adapter_ability(void);
bool oplus_pps_is_allow_real(void);
void oplus_get_props_from_adsp_by_buffer(void);
extern int oplus_pps_get_authenticate(void);
extern int oplus_chg_set_pps_config(int vbus_mv, int ibus_ma);
extern u32 oplus_chg_get_pps_status(void);
extern int oplus_chg_get_max_cur(int vbus_mv);
extern int oplus_chg_pps_cp_mode_init(int mode);
extern int oplus_pps_get_ffc_started(void);
extern int oplus_pps_get_ffc_volt_thd(void);
extern int oplus_pps_set_ffc_started(bool status);
extern int oplus_pps_get_pps_mos_started(void);
extern int oplus_pps_get_pps_disconnect(void);
extern void oplus_pps_reset_stop_status(void);
int oplus_pps_get_adapter_type(void);
extern int oplus_pps_get_stop_status(void);
extern bool oplus_pps_get_pps_dummy_started(void);
extern void oplus_pps_set_pps_dummy_started(bool enable);
extern bool oplus_pps_get_pps_fastchg_started(void);
extern void oplus_pps_print_log(void);
extern bool oplus_pps_voter_charging_start(void);
extern int oplus_pps_get_support_type(void);
extern int oplus_pps_get_master_ibus(void);
extern int oplus_pps_get_slave_ibus(void);
extern void oplus_pps_read_ibus(void);
int oplus_pps_get_adsp_authenticate(void);
int oplus_chg_pps_get_batt_curve_current(void);
int oplus_chg_pps_get_current_cool_down(void);
int oplus_chg_pps_get_current_normal_cool_down(void);
void oplus_pps_set_bcc_current(int val);
int oplus_pps_get_bcc_max_curr(void);
int oplus_pps_get_bcc_min_curr(void);
int oplus_pps_get_bcc_exit_curr(void);
bool oplus_pps_bcc_get_temp_range(void);
#endif /*_OPLUS_PPS_H_*/

