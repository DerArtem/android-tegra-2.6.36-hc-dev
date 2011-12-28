/*
 * arch/arm/mach-tegra/board-betelgeuse-wifi.c
 *
 * Copyright (C) 2010 Google, Inc.
 * Copyright (C) 2011 Artem Makhutov
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
#include "board-betelgeuse.h"

static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int betelgeuse_wifi_reset(int on);
static int betelgeuse_wifi_power(int on);
static int betelgeuse_wifi_set_carddetect(int val);

/* This is used for the ath6kl driver - not used for ar6000.ko */
static struct platform_device betelgeuse_wifi_device = {
	.name           = "ath6kl",
	.id             = 1,
	.dev            = {
	},
};

int betelgeuse_wifi_status_register(
		void (*callback)(int card_present, void *dev_id),
		void *dev_id)
{
	if (wifi_status_cb)
		return -EAGAIN;
	wifi_status_cb = callback;
	wifi_status_cb_devid = dev_id;
	return 0;
}

static int betelgeuse_wifi_set_carddetect(int val)
{
	pr_info("%s: %d\n", __func__, val);
	if (wifi_status_cb)
		wifi_status_cb(val, wifi_status_cb_devid);
	else
		pr_warning("%s: Nobody to notify\n", __func__);
	return 0;
}

static int betelgeuse_wifi_power(int on)
{
	pr_info("%s: %d\n", __func__, on);

	gpio_set_value(BETELGEUSE_WLAN_PWR, on);
	mdelay(100);
	gpio_set_value(BETELGEUSE_WLAN_RST, on);
	mdelay(200);

	return 0;
}

/* This function is called from ar6000.ko */
/* For some reason this does not work so we will just enable power */
static int wlan_setup_power(int on, int detect)
{
	pr_info("%s: WIFI Power: on: %i, detect:%i\n", __func__, on, detect);
	return 0;
}
EXPORT_SYMBOL_GPL(wlan_setup_power);

static int betelgeuse_wifi_reset(int on)
{
	pr_info("%s: do nothing\n", __func__);
	return 0;
}

int __init betelgeuse_wifi_init(void)
{
	pr_info("%s: WIFI init start\n", __func__);
	gpio_request(BETELGEUSE_WLAN_PWR, "wlan_power");
	gpio_request(BETELGEUSE_WLAN_RST, "wlan_rst");

	tegra_gpio_enable(BETELGEUSE_WLAN_PWR);
	tegra_gpio_enable(BETELGEUSE_WLAN_RST);

	gpio_direction_output(BETELGEUSE_WLAN_PWR, 0);
	gpio_direction_output(BETELGEUSE_WLAN_RST, 0);

	platform_device_register(&betelgeuse_wifi_device);

	// Lets just power on wifi
	betelgeuse_wifi_power(1);
	betelgeuse_wifi_reset(1);
	betelgeuse_wifi_set_carddetect(1);

	device_init_wakeup(&betelgeuse_wifi_device.dev, 1);
	device_set_wakeup_enable(&betelgeuse_wifi_device.dev, 0);
	pr_info("%s: WIFI init finished\n", __func__);

	return 0;
}
