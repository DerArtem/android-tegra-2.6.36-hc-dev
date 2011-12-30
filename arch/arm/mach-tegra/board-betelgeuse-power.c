/*
 * Copyright (C) 2010 NVIDIA, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */
#include <linux/i2c.h>
#include <linux/reboot.h>
#include <linux/platform_device.h>
#include <linux/resource.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/tps6586x.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/mfd/nvec.h>

#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/system.h>
#include <mach/suspend.h>

#include <generated/mach-types.h>

#include "gpio-names.h"
#include "fuse.h"
#include "power.h"
#include "wakeups-t2.h"
#include "board.h"
#include "board-betelgeuse.h"

#define PMC_CTRL		0x0
#define PMC_CTRL_INTR_LOW	(1 << 17)

static struct regulator_consumer_supply tps658621_sm0_supply[] = {
	REGULATOR_SUPPLY("vdd_core", NULL),
};
static struct regulator_consumer_supply tps658621_sm1_supply[] = {
	REGULATOR_SUPPLY("vdd_cpu", NULL),
};
static struct regulator_consumer_supply tps658621_sm2_supply[] = {
	REGULATOR_SUPPLY("vdd_sm2", NULL),
};
static struct regulator_consumer_supply tps658621_ldo0_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo0", NULL),
	REGULATOR_SUPPLY("vdd_pex_clk_1", NULL)
	//REGULATOR_SUPPLY("p_cam_avdd", NULL),
};
static struct regulator_consumer_supply tps658621_ldo1_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo1", NULL),
        REGULATOR_SUPPLY("pll_a", NULL),
        REGULATOR_SUPPLY("pll_m", NULL),
        REGULATOR_SUPPLY("pll_p", NULL),
        REGULATOR_SUPPLY("pll_c", NULL),
        REGULATOR_SUPPLY("pll_u", NULL),
        REGULATOR_SUPPLY("pll_u1", NULL),
        REGULATOR_SUPPLY("pll_s", NULL),
        REGULATOR_SUPPLY("pll_x", NULL)
//	REGULATOR_SUPPLY("avdd_pll", NULL),
};
static struct regulator_consumer_supply tps658621_ldo2_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo2", NULL),
	REGULATOR_SUPPLY("vdd_rtc", NULL),
};
static struct regulator_consumer_supply tps658621_ldo3_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo3", NULL),
        REGULATOR_SUPPLY("avdd_usb", NULL),
        REGULATOR_SUPPLY("avdd_usb_pll", NULL),
        REGULATOR_SUPPLY("vddio_nand_3v3", NULL), // AON
        REGULATOR_SUPPLY("sdio", NULL), /* vddio_sdio */
        REGULATOR_SUPPLY("vmmc", NULL), /* vddio_mmc, but sdhci.c requires it to be called vmmc*/
        REGULATOR_SUPPLY("vddio_vi", NULL),
        REGULATOR_SUPPLY("avdd_lvds", NULL),
        REGULATOR_SUPPLY("tmon0", NULL),
	REGULATOR_SUPPLY("vdd_vcore_temp", NULL),
};
static struct regulator_consumer_supply tps658621_ldo4_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo4", NULL),
        REGULATOR_SUPPLY("avdd_osc", NULL),
        REGULATOR_SUPPLY("vddio_sys", NULL),
        REGULATOR_SUPPLY("vddio_lcd", NULL),       //AON
        REGULATOR_SUPPLY("vddio_audio", NULL),     //AON
        REGULATOR_SUPPLY("vddio_ddr", NULL),       //AON
        REGULATOR_SUPPLY("vddio_uart", NULL),      //AON
        REGULATOR_SUPPLY("vddio_bb", NULL),        //AON
        REGULATOR_SUPPLY("vddhostif_bt", NULL),
        REGULATOR_SUPPLY("vddio_wlan", NULL),
	REGULATOR_SUPPLY("vmic", NULL)
};

//Unused
static struct regulator_consumer_supply tps658621_ldo5_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo5", NULL),
};
static struct regulator_consumer_supply tps658621_ldo6_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo6", NULL),
	//REGULATOR_SUPPLY("vmic", "soc-audio"),
	REGULATOR_SUPPLY("vddio vdac", NULL),
        REGULATOR_SUPPLY("avdd_vdac", NULL)
};
static struct regulator_consumer_supply tps658621_ldo7_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo7", NULL),
	REGULATOR_SUPPLY("avdd_hdmi", NULL),
	//REGULATOR_SUPPLY("vdd_fuse", NULL),
};
static struct regulator_consumer_supply tps658621_ldo8_supply[] = {
	REGULATOR_SUPPLY("vdd_ldo8", NULL),
	REGULATOR_SUPPLY("avdd_hdmi_pll", NULL),
};
static struct regulator_consumer_supply tps658621_ldo9_supply[] = {
	//REGULATOR_SUPPLY("vdd_ldo9", NULL),
	//REGULATOR_SUPPLY("avdd_2v85", NULL),
	REGULATOR_SUPPLY("vdd_ddr_rx", NULL),
	//REGULATOR_SUPPLY("avdd_amp", NULL),
};

/*
 * Current TPS6586x is known for having a voltage glitch if current load changes
 * from low to high in auto PWM/PFM mode for CPU's Vdd line.
 */
static struct tps6586x_settings sm1_config = {
	.sm_pwm_mode = PWM_ONLY,
};

#define REGULATOR_INIT(_id, _minmv, _maxmv, on, config)			\
	{								\
		.constraints = {					\
			.min_uV = (_minmv)*1000,			\
			.max_uV = (_maxmv)*1000,			\
			.valid_modes_mask = (REGULATOR_MODE_NORMAL |	\
					     REGULATOR_MODE_STANDBY),	\
			.valid_ops_mask = (REGULATOR_CHANGE_MODE |	\
					   REGULATOR_CHANGE_STATUS |	\
					   REGULATOR_CHANGE_VOLTAGE),	\
			.always_on = on,				\
		},							\
		.num_consumer_supplies = ARRAY_SIZE(tps658621_##_id##_supply),\
		.consumer_supplies = tps658621_##_id##_supply,		\
		.driver_data = config,					\
	}

#define ON	1
#define OFF	0

static struct regulator_init_data sm0_data = REGULATOR_INIT(sm0, 625, 2700, ON, NULL); //1200
static struct regulator_init_data sm1_data = REGULATOR_INIT(sm1, 625, 2700, ON, &sm1_config); // 1000
static struct regulator_init_data sm2_data = REGULATOR_INIT(sm2, 3000, 4550, ON, NULL); //3700 *ok*
static struct regulator_init_data ldo0_data = REGULATOR_INIT(ldo0, 1250, 3350, OFF, NULL); //3300
static struct regulator_init_data ldo1_data = REGULATOR_INIT(ldo1, 725, 1500, ON, NULL); //1100
static struct regulator_init_data ldo2_data = REGULATOR_INIT(ldo2, 725, 1500, OFF, NULL); //1200
static struct regulator_init_data ldo3_data = REGULATOR_INIT(ldo3, 1250, 3350, OFF, NULL); //3300
static struct regulator_init_data ldo4_data = REGULATOR_INIT(ldo4, 1700, 2000, ON, NULL); //1800
static struct regulator_init_data ldo5_data = REGULATOR_INIT(ldo5, 1250, 3350, ON, NULL); //2850
static struct regulator_init_data ldo6_data = REGULATOR_INIT(ldo6, 1250, 3350, OFF, NULL); //2850
static struct regulator_init_data ldo7_data = REGULATOR_INIT(ldo7, 1250, 3350, OFF, NULL); //3300
static struct regulator_init_data ldo8_data = REGULATOR_INIT(ldo8, 1250, 3350, OFF, NULL); //1800
static struct regulator_init_data ldo9_data = REGULATOR_INIT(ldo9, 1250, 3350, ON, NULL); //2850

static struct tps6586x_rtc_platform_data rtc_data = {
	.irq = TEGRA_NR_IRQS + TPS6586X_INT_RTC_ALM1,
	.start = {
		.year = 2009,
		.month = 1,
		.day = 1,
	},
	.cl_sel = TPS6586X_RTC_CL_SEL_1_5PF /* use lowest (external 20pF cap) */
};

#define TPS_REG(_id, _data)			\
	{					\
		.id = TPS6586X_ID_##_id,	\
		.name = "tps6586x-regulator",	\
		.platform_data = _data,		\
	}

static struct tps6586x_subdev_info tps_devs[] = {
	TPS_REG(SM_0, &sm0_data),
	TPS_REG(SM_1, &sm1_data),
	TPS_REG(SM_2, &sm2_data),
	TPS_REG(LDO_0, &ldo0_data),
	TPS_REG(LDO_1, &ldo1_data),
	TPS_REG(LDO_2, &ldo2_data),
	TPS_REG(LDO_3, &ldo3_data),
	TPS_REG(LDO_4, &ldo4_data),
	TPS_REG(LDO_5, &ldo5_data),
	TPS_REG(LDO_6, &ldo6_data),
	TPS_REG(LDO_7, &ldo7_data),
	TPS_REG(LDO_8, &ldo8_data),
	TPS_REG(LDO_9, &ldo9_data),
	{
		.id	= 0,
		.name	= "tps6586x-rtc",
		.platform_data = &rtc_data,
	},
};

static struct tps6586x_platform_data tps_platform = {
	.irq_base = TPS6586X_INT_BASE,
	.num_subdevs = ARRAY_SIZE(tps_devs),
	.subdevs = tps_devs,
	.gpio_base = TPS6586X_GPIO_BASE,
};

static struct i2c_board_info __initdata betelgeuse_regulators[] = {
	{
		I2C_BOARD_INFO("tps6586x", 0x34),
		.irq		= INT_EXTERNAL_PMU,
		.platform_data	= &tps_platform,
	},
};

static void reg_off(const char *reg)
{
	int rc;
	struct regulator *regulator;

	regulator = regulator_get(NULL, reg);

	if (IS_ERR(regulator)) {
		pr_err("%s: regulator_get returned %ld\n", __func__,
			PTR_ERR(regulator));
		return;
	}

	/* force disabling of regulator to turn off system */
	rc = regulator_force_disable(regulator);
	if (rc)
		pr_err("%s: regulator_disable returned %d\n", __func__, rc);
	regulator_put(regulator);
}

static void reg_on(const char *reg)
{
	int rc;
	struct regulator *regulator;

	regulator = regulator_get(NULL, reg);

	if (IS_ERR(regulator)) {
	pr_err("%s: regulator_get returned %ld\n", __func__,
		PTR_ERR(regulator));
	return;
	}

	/* enable the regulator */
	rc = regulator_enable(regulator);
	if (rc)
		pr_err("%s: regulator_enable returned %d\n", __func__, rc);
	regulator_put(regulator);
}

static void betelgeuse_power_off(void)
{
	/* Power down through NvEC */
	nvec_poweroff();

	/* Then try by powering off supplies */
	reg_off("vdd_sm2");
	reg_off("vdd_core");
	reg_off("vdd_cpu");
	reg_off("vdd_soc");
	local_irq_disable();
	while (1) {
		dsb();
		__asm__ ("wfi");
	}
}

static void tegra_sys_reset(char mode, const char *cmd)
{
	/* use *_related to avoid spinlock since caches are off */
	u32 reg;
	void __iomem *car_reset = IO_ADDRESS(TEGRA_CLK_RESET_BASE + 0x04);
	void __iomem *sys_reset = IO_ADDRESS(TEGRA_PMC_BASE + 0x00);

	/* CAR reset */
	reg = readl_relaxed(car_reset);
	reg |= 0x04;
	writel_relaxed(reg, car_reset);

	/* System reset */
	reg = readl_relaxed(sys_reset);
	reg |= 0x10;
	writel_relaxed(reg, sys_reset);
}

static int tegra_reboot_notify(struct notifier_block *nb,
                                unsigned long event, void *data)
{
	switch (event) {
	case SYS_RESTART:
	case SYS_HALT:
	case SYS_POWER_OFF:
		/* USB power rail must be enabled during boot or we won't reboot*/
		reg_on("avdd_usb");

		return NOTIFY_OK;
	}
	return NOTIFY_DONE;
}

static struct notifier_block tegra_reboot_nb = {
	.notifier_call = tegra_reboot_notify,
	.next = NULL,
	.priority = 0
};

static void __init tegra_setup_reboot(void)
{
	int rc = register_reboot_notifier(&tegra_reboot_nb);
	if (rc)
		pr_err("%s: failed to register platform reboot notifier\n",__func__);
	/*arm_pm_restart = shuttle_restart;             */
	tegra_reset = tegra_sys_reset;
}

int __init betelgeuse_power_init(void)
{
	int err;
	void __iomem *pmc = IO_ADDRESS(TEGRA_PMC_BASE);
	u32 pmc_ctrl;

	/* configure the power management controller to trigger PMU
	 * interrupts when low */
	pmc_ctrl = readl(pmc + PMC_CTRL);
	writel(pmc_ctrl | PMC_CTRL_INTR_LOW, pmc + PMC_CTRL);

	err = i2c_register_board_info(4, betelgeuse_regulators, 1);
	if (err < 0)
		pr_warning("Unable to initialize regulator\n");

	/* register the poweroff callback */
	pm_power_off = betelgeuse_power_off;

	/* And the restart callback */
	tegra_setup_reboot();

	regulator_has_full_constraints();

	return 0;
}
