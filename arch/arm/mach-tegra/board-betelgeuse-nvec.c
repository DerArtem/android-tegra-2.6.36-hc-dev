/*
 * arch/arm/mach-tegra/board-betelgeuse-camera.c
 *
 * Copyright (C) 2011 Artem Makhutov <artem@makhutov.org>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/console.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/memblock.h>

#include <linux/power/nvec_power.h>
#include <linux/mfd/nvec.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/io.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/nand.h>
#include <mach/iomap.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "clock.h"
#include "gpio-names.h"
#include "devices.h"

/* Power controller of Nvidia embedded controller platform data */
static struct nvec_power_platform_data nvec_power_pdata = {
        .low_batt_irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PW3),    /* If there is a low battery IRQ */
        .in_s3_state_gpio = TEGRA_GPIO_PAA7,                  /* Gpio pin used to flag that system is suspended */
        .low_batt_alarm_percent = 5,                          /* Percent of batt below which system is forcibly turned off */
};

/* Power controller of Nvidia embedded controller */
static struct nvec_subdev_info nvec_subdevs[] = {
        {
                .name = "nvec-power",
                .id   = 1,
                .platform_data = &nvec_power_pdata,
        },
        {
                .name = "nvec-kbd",
                .id   = 1,
        },
        {
                .name = "nvec-mouse",
                .id   = 1,
        },
};

/* The NVidia Embedded controller */
static struct nvec_platform_data nvec_mfd_platform_data = {
        .i2c_addr       = 0x8a,
        .gpio           = TEGRA_GPIO_PD0,
        .irq            = INT_I2C3,
        .base           = TEGRA_I2C3_BASE,
        .size           = TEGRA_I2C3_SIZE,
        .clock          = "tegra-i2c.2",
        .subdevs        = nvec_subdevs,
        .num_subdevs = ARRAY_SIZE(nvec_subdevs),
};

static struct platform_device shuttle_nvec_mfd = {
        .name = "nvec",
        .dev = {
                .platform_data = &nvec_mfd_platform_data,
        },
};

static struct platform_device *shuttle_power_devices[] __initdata = {
        &shuttle_nvec_mfd,
};


int __init betelgeuse_nvec_init(void)
{
	tegra_gpio_enable(TEGRA_GPIO_PW3);
	tegra_gpio_enable(TEGRA_GPIO_PD0);
	tegra_gpio_enable(TEGRA_GPIO_PAA7);
	return platform_add_devices(shuttle_power_devices, ARRAY_SIZE(shuttle_power_devices));
}
