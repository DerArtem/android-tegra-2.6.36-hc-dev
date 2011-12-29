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
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/akm8975.h>
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

static struct i2c_board_info __initdata atd7461_board_info = {
	I2C_BOARD_INFO("atd7461", 0x4c), /* aka lm90 */
	.irq		= TEGRA_GPIO_TO_IRQ(ATD7461_IRQ_GPIO),
};

static void betelgeuse_atd7461_init(void)
{
	tegra_gpio_enable(ATD7461_IRQ_GPIO);
	gpio_request(ATD7461_IRQ_GPIO, "atd7461");
	gpio_direction_input(ATD7461_IRQ_GPIO);
	i2c_register_board_info(0, &atd7461_board_info, 1);
}

int __init betelgeuse_sensors_init(void)
{
	betelgeuse_akm8975_init();
	betelgeuse_atd7461_init();
	return 0;
}
