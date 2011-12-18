/*
 * arch/arm/mach-tegra/board-betelgeuse-clocks.c
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
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/fsl_devices.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/i2c-tegra.h>
#include <linux/memblock.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/io.h>
#include <mach/w1.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/nand.h>
#include <mach/sdhci.h>
#include <mach/gpio.h>
#include <mach/clk.h>
#include <mach/usb_phy.h>
#include <mach/i2s.h>
#include <mach/system.h>
#include <mach/nvmap.h>

#include "board.h"
#include "board-betelgeuse.h"
#include "clock.h"
#include "gpio-names.h"
#include "devices.h"

/* Be careful here: Most clocks have restrictions on parent and on
   divider/multiplier ratios. Check tegra2clocks.c before modifying
   this table ! */
static __initdata struct tegra_clk_init_table betelgeuse_clk_init_table[] = {
	/* name			parent				rate	enabled */
	/* 32khz system clock */
	{ "clk_32k",		NULL,			32768,		true},		/* always on */
	{ "rtc",		"clk_32k",			32768,		true},		/* rtc-tegra : must be always on */
	{ "kbc",		"clk_32k",			32768,		true},		/* tegra-kbc */
	{ "blink",		"clk_32k",			32768,		false},		/* used for bluetooth */
	{ "pll_s",		"clk_32k",			32768,		true},		/* must be always on */
	/* Master clock */
	{ "clk_m",		NULL,					0,		true},	 	/* must be always on - Frequency will be autodetected */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	{ "pcie_xclk",	"clk_m",		12000000,		false},		/* pcie controller */
	{ "afi",		"clk_m",		12000000,		false},		/* pcie controller */
	{ "pex",		"clk_m",		12000000,		false},		/* pcie controller */
#endif
	{ "csus",		"clk_m",		12000000,		false},		/* tegra_camera */
	{ "isp",		"clk_m",		12000000,		false},		/* tegra_camera */
	{ "usb3",		"clk_m",		12000000,		true},		/* tegra_ehci.2 */
	{ "usb2",		"clk_m",		12000000,		true},		/* tegra_ehci.1 */
	{ "usbd",		"clk_m",		12000000,		true},		/* fsl-tegra-udc , utmip-pad , tegra_ehci.0 , tegra_otg */
	{ "disp2",		"clk_m",		12000000,		false},		/* tegradc.1 */	
	{ "tvdac",		"clk_m",		12000000,		false},
	{ "hdmi",		"clk_m",		12000000,		false},		/* tegra_dc.0, tegra_dc.1 */
	{ "tvo",		"clk_m",		12000000,		false},
	{ "cve",		"clk_m",		12000000,		false},
	{ "uarte",		"clk_m",		12000000,		false},		/* tegra_uart.4 uart.4 */
	{ "uartd",		"clk_m",		12000000,		false},		/* tegra_uart.3 uart.3 */
	{ "uartc",		"clk_m",		12000000,		false},		/* tegra_uart.2 uart.2 */
	{ "uartb",		"clk_m",		12000000,		false},		/* tegra_uart.1 uart.1 */
	{ "dvc",		"clk_m",		800000,		false},		/* tegra-i2c.3 */
	{ "i2c3",		"clk_m",		800000,		true},		/* tegra-i2c.2 */
	{ "i2c2",		"clk_m",		400000,		false},		/* tegra-i2c.1 */
	{ "i2c1",		"clk_m",		3000000,		false},		/* tegra-i2c.0 */
	{ "mipi",		"clk_m",		12000000,	false},
	{ "nor",		"clk_m",		12000000,	false},
	{ "owr",		"clk_m",		12000000,	false},		/* tegra_w1 */
	{ "la",			"clk_m",		12000000,	false},
	{ "bsev",		"clk_m",		12000000,	true},		/* tegra_aes */
	{ "bsea",		"clk_m",		12000000,	false},		/* tegra_avp */
	{ "vcp",		"clk_m",		12000000,	false},		/* tegra_avp */
	{ "sdmmc3",		"clk_m",		12000000,	false},
	{ "sdmmc2",		"clk_m",		400000,		true},
	{ "vfir",		"clk_m",		12000000,		false},
	{ "ndflash",	"pll_p",		12000000,		false},
	{ "ide",		"clk_m",		12000000,		false},
	{ "sbc4",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.3 */
	{ "sbc3",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.2 */
	{ "sbc2",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.1 */
	{ "sbc1",		"clk_m",		12000000,		false}, 	/* tegra_spi_slave.0 */
	{ "twc",		"clk_m",		12000000,		false},
	{ "xio",		"clk_m",		12000000,		false},
	{ "spi",		"clk_m",		12000000,		false},
	{ "pwm",		"clk_m",		93385,		true},		/* tegra-pwm.0 tegra-pwm.1 tegra-pwm.2 tegra-pwm.3*/
	{ "kfuse",		"clk_m",		12000000,		false},		/* kfuse-tegra */ /* always on - no init req */
	{ "timer",		"clk_m",		12000000,		true},		/* timer */ /* always on - no init req */
	{ "clk_d",		"clk_m",		24000000,		true},
	// Use default for now
	//{ "pll_e",		"clk_m",		1200000000,		false},
	{ "pll_x",		"clk_m",		1000000000,		true}, // check here
	{ "cclk",		"pll_x",		1000000000,		true},
	{ "cpu",		"cclk",			1000000000,		true},
	{ "pll_u",		"clk_m",		480000000,		true},		/* USB ulpi clock */
	{ "pll_d",		"clk_m",		1000000,		true},		/* hdmi clock */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	{ "pll_d_out0",		"pll_d",		500000,			false},		/* hdmi clock */
	{ "dsi",		"pll_d",		1000000,		false},		/* tegra_dc.0, tegra_dc.1 */
#else
	{ "pll_d_out0",		"pll_d",		1000000,		true},		/* hdmi clock */
	{ "dsi",		"pll_d_out0",		1000000,		false},		/* tegra_dc.0, tegra_dc.1 - bug on kernel 2.6.36*/
#endif
	{ "pll_p",		"clk_m",		216000000,	true},		/* must be always on */
	{ "host1x",		"pll_p",		108000000,	false},		/* tegra_grhost */
	{ "uarta",		"pll_p",		216000000,	false},		/* tegra_uart.0 uart.0 */
	{ "csite",		"pll_p",		144000000,	true},		/* csite - coresite */ /* always on */
	{ "sdmmc4",		"pll_p",		48000000,	false},		/* sdhci-tegra.3 */
	{ "sdmmc1",		"pll_p",		48000000,	true},		/* sdhci-tegra.0 */
	{ "spdif_in",		"pll_p",		36000000,	false},
	{ "pll_p_out4",		"pll_p",		24000000,	true},		/* must be always on - USB ulpi */
	{ "pll_p_out3",		"pll_p",		72000000,	true},		/* must be always on - i2c, camera */
	{ "csi",		"pll_p_out3",		72000000,	false},		/* tegra_camera */
	{ "dvc_i2c",		"pll_p_out3",		72000000,	true},		/* tegra-i2c.3 */
	{ "i2c3_i2c",		"pll_p_out3",		72000000,	true},		/* tegra-i2c.2 */
	{ "i2c2_i2c",		"pll_p_out3",		72000000,	true},		/* tegra-i2c.1 */
	{ "i2c1_i2c",		"pll_p_out3",		72000000,	true},		/* tegra-i2c.0 */
	{ "pll_p_out2",		"pll_p",		108000000,	true},		/* must be always on */
	{ "sclk",		"pll_p_out2",		108000000,	true},		/* must be always on */
	{ "avp.sclk",		"sclk",			108000000,	false},		/* must be always on */
	{ "cop",		"sclk",			108000000,	false},		/* must be always on */
	{ "hclk",		"sclk",			108000000,	true},		/* must be always on */
	{ "pclk",		"hclk",			54000000,	true},		/* must be always on */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
	{ "apbdma",		"pclk",			54000000,	false}, 	/* tegra-dma */
#endif
	{ "pll_p_out1",	"pll_p",		28800000,	true},		/* must be always on - audio clocks ...*/
	{ "pll_a",		"pll_p_out1",	56448000,	true},		/* always on - audio clocks */
	{ "pll_a_out0",	"pll_a",		11289600,	true},		/* always on - i2s audio */
	{ "audio", 		"pll_a_out0",	11289600,	true},
	{ "audio_2x",	"audio",		22579200,	false},
	{ "spdif_out",	"pll_a_out0",	5644800,	false},
	{ "i2s2",		"pll_a_out0",	470400,		false},		/* i2s.1 */
	{ "i2s1",		"pll_a_out0",	2822400,	true},		/* i2s.0 */
	{ "pll_c",		"clk_m",		600000000,	true},		/* always on - graphics and camera clocks */
	{ "disp1",		"pll_c",		600000000,	true},		/* tegradc.0 */
	{ "epp",		"pll_c",		300000000,	false},		/* tegra_grhost */
	{ "2d",			"pll_c",		300000000,	false},		/* tegra_grhost, gr2d */
	{ "vde",		"pll_p",		240000000,	true},		/* tegra-avp */
	{ "pll_c_out1",	"pll_c",		150000000,	true},		/* must be always on - system clock */
	{ "pll_m",		"clk_m",		666000000,	true},		/* always on - memory clocks */
	{ "mpe",		"pll_m",		111000000,	false},		/* tegra_grhost */
	{ "vi_sensor",	"pll_m",		111000000,	false},		/* tegra_camera : unused on shuttle */
	{ "vi",			"pll_m",		111000000,	false},		/* tegra_camera : unused on shuttle */
	{ "3d",     	"pll_m",    	333000000,  false},		/* tegra_grhost, gr3d */
	{ "emc",		"pll_m",		666000000,	true},		/* always on */
	{ "pll_m_out1",	"pll_m",		222000000,	true},		/* always on - unused ?*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,38)
#       define CDEV1 "cdev1"
#       define CDEV2 "cdev2"
#else
#       define CDEV1 "clk_dev1"
#       define CDEV2 "clk_dev2"
#endif

	{ CDEV1,		"pll_a_out0",		11289600,	false},		/* used as audio CODEC MCLK */	
	{ CDEV2,		"pll_p_out4",		24000000,	false}, 	/* probably used as USB clock - perhaps 24mhz ?*/	
	{ NULL,			NULL,			0,		0},
};

void __init betelgeuse_clocks_init(void)
{
	tegra_clk_init_from_table(betelgeuse_clk_init_table);
}
