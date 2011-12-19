/*
 * arch/arm/mach-tegra/board-harmony-sdhci.c
 *
 * Copyright (C) 2010 Google, Inc.
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

#include <linux/resource.h>
#include <linux/platform_device.h>
#include <linux/wlan_plat.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/mmc/host.h>

#include <asm/mach-types.h>
#include <mach/irqs.h>
#include <mach/iomap.h>
#include <mach/sdhci.h>

#include "gpio-names.h"
#include "board.h"

#define ANTARES_WLAN_PWR	TEGRA_GPIO_PK5
#define ANTARES_WLAN_RST	TEGRA_GPIO_PK6

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;
static int antares_wifi_status_register(void (*callback)(int , void *), void *);
static struct clk *wifi_32k_clk;

static int antares_wifi_reset(int on);
static int antares_wifi_power(int on);
static int antares_wifi_set_carddetect(int val);

static struct wifi_platform_data antares_wifi_control = {
	.set_power      = antares_wifi_power,
	.set_reset      = antares_wifi_reset,
	.set_carddetect = antares_wifi_set_carddetect,
};

static struct platform_device antares_wifi_device = {
	.name           = "ath6kl",
	.id             = 1,
	.dev            = {
		//.platform_data = &antares_wifi_control,
	},
};

static int antares_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int antares_wifi_set_carddetect(int val)
{
	pr_info("%s: %d\n", __func__, val);
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

static int antares_wifi_power(int on)
{
	pr_info("%s: %d\n", __func__, on);

	gpio_set_value(ANTARES_WLAN_PWR, on);
	mdelay(100);
	gpio_set_value(ANTARES_WLAN_RST, on);
	mdelay(200);

	if (on)
		clk_enable(wifi_32k_clk);
	else
		clk_disable(wifi_32k_clk);

	return 0;
}

static int antares_wifi_reset(int on)
{
	pr_info("%s: do nothing\n", __func__);
	return 0;
}

int __init betelgeuse_wifi_init(void)
{
	pr_info("%s: WIFI1\n", __func__);
	gpio_request(ANTARES_WLAN_PWR, "wlan_power");
	gpio_request(ANTARES_WLAN_RST, "wlan_rst");

	tegra_gpio_enable(ANTARES_WLAN_PWR);
	tegra_gpio_enable(ANTARES_WLAN_RST);

	gpio_direction_output(ANTARES_WLAN_PWR, 0);
	gpio_direction_output(ANTARES_WLAN_RST, 0);

	platform_device_register(&antares_wifi_device);

	device_init_wakeup(&antares_wifi_device.dev, 1);
	device_set_wakeup_enable(&antares_wifi_device.dev, 0);
	pr_info("%s: WIFI2\n", __func__);

	return 0;
}
