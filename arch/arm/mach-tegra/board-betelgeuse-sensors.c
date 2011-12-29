/*
 * arch/arm/mach-tegra/board-betelgeuse-sensors.c
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
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/akm8975.h>
#include <linux/adt7461.h>
#include <linux/mpu.h>

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

extern void tegra_throttling_enable(bool enable);

static struct i2c_board_info __initdata ak8975_device = {
	I2C_BOARD_INFO("akm8975", 0x0c),
	.irq            = TEGRA_GPIO_TO_IRQ(AKM8975_IRQ_GPIO),
};

static void betelgeuse_akm8975_init(void)
{
	tegra_gpio_enable(AKM8975_IRQ_GPIO);
	gpio_request(AKM8975_IRQ_GPIO, "akm8975");
	gpio_direction_input(AKM8975_IRQ_GPIO);
	i2c_register_board_info(0, &ak8975_device, 1);
}

static struct adt7461_platform_data betelgeuse_adt7461_pdata = {
        .supported_hwrev = true,
        .ext_range = false,
        .therm2 = true,
        .conv_rate = 0x05,
        .offset = 0,
        .hysteresis = 0,
        .shutdown_ext_limit = 115,
        .shutdown_local_limit = 120,
        .throttling_ext_limit = 90,
        .alarm_fn = tegra_throttling_enable,
};

static struct i2c_board_info __initdata adt7461_board_info = {
	I2C_BOARD_INFO("adt7461", 0x4c), /* aka lm90 */
	.irq		= TEGRA_GPIO_TO_IRQ(ADT7461_IRQ_GPIO),
	.platform_data 	= &betelgeuse_adt7461_pdata,
};

static void betelgeuse_adt7461_init(void)
{
	tegra_gpio_enable(ADT7461_IRQ_GPIO);
	gpio_request(ADT7461_IRQ_GPIO, "adt7461");
	gpio_direction_input(ADT7461_IRQ_GPIO);
	i2c_register_board_info(0, &adt7461_board_info, 1);
}

int __init betelgeuse_sensors_init(void)
{
	betelgeuse_akm8975_init();
	betelgeuse_adt7461_init();
	return 0;
}
