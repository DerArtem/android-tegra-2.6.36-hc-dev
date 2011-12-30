/*
 * arch/arm/mach-tegra/board-betelgeuse.c
 *
 * Copyright (C) 2010 Google, Inc.
 *               2011 Artem Makhutov <artem@makhutov.org>
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

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/clk.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/dma-mapping.h>
#include <linux/i2c.h>
#include <linux/i2c-tegra.h>
#include <linux/gpio.h>
#include <linux/input/eeti_ts.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/platform_data/tegra_usb.h>
#include <linux/usb/android_composite.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>
#include <asm/setup.h>

#include <mach/audio.h>
#include <mach/iomap.h>
#include <mach/irqs.h>
#include <mach/clk.h>
#include <mach/usb_phy.h>
#include <mach/suspend.h>
#include <mach/spdif.h>
#include <mach/tegra_das.h>

#include <sound/wm8903.h>

#include "clock.h"
#include "board.h"
#include "board-betelgeuse.h"
#include "devices.h"
#include "gpio-names.h"
#include "wakeups-t2.h"

/* NVidia bootloader tags */
#define ATAG_NVIDIA			0x41000801
#define MAX_MEMHDL			8

struct tag_tegra {
        __u32 bootarg_len;
        __u32 bootarg_key;
        __u32 bootarg_nvkey;
        __u32 bootarg[];
};

struct memhdl {
        __u32 id;
        __u32 start;
        __u32 size;
};

enum {  
        RM = 1,
        DISPLAY,
        FRAMEBUFFER,
        CHIPSHMOO,
        CHIPSHMOO_PHYS,
        CARVEOUT,
        WARMBOOT,
};

static int num_memhdl = 0;

static struct memhdl nv_memhdl[MAX_MEMHDL];
static size_t fb_addr;

static const char atag_ids[][16] = {
        "RM             ",
        "DISPLAY        ",
        "FRAMEBUFFER    ",
        "CHIPSHMOO      ",
        "CHIPSHMOO_PHYS ",
        "CARVEOUT       ",
        "WARMBOOT       ",
};

static int __init parse_tag_nvidia(const struct tag *tag)
{
        int i;
        struct tag_tegra *nvtag = (struct tag_tegra *)tag;
        __u32 id;

        switch (nvtag->bootarg_nvkey) {
        case FRAMEBUFFER:
                id = nvtag->bootarg[1];
                for (i=0; i<num_memhdl; i++)
                        if (nv_memhdl[i].id == id)
                                fb_addr = nv_memhdl[i].start;
                break;
        case WARMBOOT:
                id = nvtag->bootarg[1];
                for (i=0; i<num_memhdl; i++) {
                        if (nv_memhdl[i].id == id) {
                                tegra_lp0_vec_start = nv_memhdl[i].start;
                                tegra_lp0_vec_size = nv_memhdl[i].size;
                        }
                }
                break;
        }

        if (nvtag->bootarg_nvkey & 0x10000) {
                char pmh[] = " PreMemHdl     ";
                id = nvtag->bootarg_nvkey;
                if (num_memhdl < MAX_MEMHDL) {
                        nv_memhdl[num_memhdl].id = id;
                        nv_memhdl[num_memhdl].start = nvtag->bootarg[1];
                        nv_memhdl[num_memhdl].size = nvtag->bootarg[2];
                        num_memhdl++;
                }
                pmh[11] = '0' + id;
                print_hex_dump(KERN_INFO, pmh, DUMP_PREFIX_NONE,
                                32, 4, &nvtag->bootarg[0], 4*(tag->hdr.size-2), false);
        }
        else if (nvtag->bootarg_nvkey <= ARRAY_SIZE(atag_ids))
                print_hex_dump(KERN_INFO, atag_ids[nvtag->bootarg_nvkey-1], DUMP_PREFIX_NONE,
                                32, 4, &nvtag->bootarg[0], 4*(tag->hdr.size-2), false);
        else
                pr_warning("unknown ATAG key %d\n", nvtag->bootarg_nvkey);

        return 0;
}
__tagtable(ATAG_NVIDIA, parse_tag_nvidia);

static struct tegra_suspend_platform_data betelgeuse_suspend = {
	.cpu_timer	= 5000,
	.cpu_off_timer	= 5000,
	.core_timer	= 0x7e7e,
	.core_off_timer	= 0x7f,
	.corereq_high	= false,
	.sysclkreq_high	= true,
	.suspend_mode   = TEGRA_SUSPEND_LP0,
	.wake_enb	= TEGRA_WAKE_GPIO_PA0,
	.wake_high	= 0,
	.wake_low	= TEGRA_WAKE_GPIO_PA0,
	.wake_any	= 0,
};

static struct plat_serial8250_port debug_uart_platform_data[] = {
	{
		.membase	= IO_ADDRESS(TEGRA_UARTD_BASE),
		.mapbase	= TEGRA_UARTD_BASE,
		.irq		= INT_UARTD,
		.flags		= UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
		.uartclk	= 216000000,
	}, {
		.flags		= 0
	}
};

static struct platform_device debug_uart = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = debug_uart_platform_data,
	},
};

//touch screen
static const struct i2c_board_info betelgeuse_i2c_bus1_touch_info[] = {
	{
		I2C_BOARD_INFO("egalax_i2c", 0x04),
		.irq = TEGRA_GPIO_TO_IRQ(TEGRA_GPIO_PU4),
	},
};

static int __init betelgeuse_touch_init_egalax(void)
{
	tegra_gpio_enable(TEGRA_GPIO_PU4);
	i2c_register_board_info(0, betelgeuse_i2c_bus1_touch_info, 1);
	return 0;
}

static void __init tegra_betelgeuse_fixup(struct machine_desc *desc,
	struct tag *tags, char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].size  = SHUTTLE_MEM_SIZE;
	mi->bank[0].size  = SHUTTLE_MEM_SIZE - SHUTTLE_GPU_MEM_SIZE;
}

static struct platform_device *betelgeuse_devices[] __initdata = {
        &debug_uart,
        &tegra_udc_device,
        &tegra_spi_device1,
        &tegra_spi_device2,
        &tegra_spi_device3,
        &tegra_spi_device4,
        &tegra_gart_device,
        &tegra_avp_device,
	&tegra_aes_device,
	&tegra_wdt_device,
};

static void __init tegra_betelgeuse_init(void)
{
	tegra_common_init();
	tegra_init_suspend(&betelgeuse_suspend);
//	betelgeuse_emc_init();
	betelgeuse_pinmux_init();
	betelgeuse_clocks_init();
	betelgeuse_i2c_init();
	betelgeuse_power_init();
	betelgeuse_nvec_init();
	betelgeuse_usb_init();
	//UART
	//SPI
	betelgeuse_sdhci_init();
	betelgeuse_panel_init();
	betelgeuse_audio_init();
	//betelgeuse_wired_jack_init();
	betelgeuse_sensors_init();

	platform_add_devices(betelgeuse_devices, ARRAY_SIZE(betelgeuse_devices));

	betelgeuse_kbc_init();
	betelgeuse_touch_init_egalax();
	betelgeuse_wifi_init();
	betelgeuse_camera_init();
}

MACHINE_START(TEGRA_LEGACY, "tegra_legacy")
	.boot_params  = 0x00000100,
	.phys_io        = IO_APB_PHYS,
	.io_pg_offst    = ((IO_APB_VIRT) >> 18) & 0xfffc,
	.fixup          = tegra_betelgeuse_fixup,
	.init_irq       = tegra_init_irq,
	.init_machine   = tegra_betelgeuse_init,
	.map_io         = tegra_map_common_io,
	.timer          = &tegra_timer,
MACHINE_END
