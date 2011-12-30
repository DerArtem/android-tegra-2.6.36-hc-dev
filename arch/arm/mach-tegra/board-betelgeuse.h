/*
 * arch/arm/mach-tegra/board-betelgeuse.h
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

#ifndef _MACH_TEGRA_BOARD_BETELGEUSE_H
#define _MACH_TEGRA_BOARD_BETELGEUSE_H

void betelgeuse_pinmux_init(void);
void __init betelgeuse_clocks_init(void);
int __init betelgeuse_power_init(void);
int betelgeuse_panel_init(void);
int betelgeuse_sdhci_init(void);
void betelgeuse_emc_init(void);
int betelgeuse_kbc_init(void);
int __init betelgeuse_i2c_init(void);
int __init betelgeuse_sensors_init(void);
extern int betelgeuse_keyboard_register_devices(void);
int __init betelgeuse_usb_init(void);
int __init betelgeuse_audio_init(void);
int __init betelgeuse_wired_jack_init(void);
int __init betelgeuse_wifi_init(void);
int __init betelgeuse_camera_init(void);
int __init betelgeuse_nvec_init(void);
int __init antares_ec_init(void);
int betelgeuse_wifi_status_register(void (*callback)(int , void *), void *);

/* TPS6586X gpios */
#define TPS6586X_GPIO_BASE      TEGRA_NR_GPIOS
#define AVDD_DSI_CSI_ENB_GPIO   (TPS6586X_GPIO_BASE + 1) /* gpio2 */

/* WM8903 gpios */
#define WM8903_GPIO_BASE        (TEGRA_NR_GPIOS + 32)
#define WM8903_GP1              (WM8903_GPIO_BASE + 0)
#define WM8903_GP2              (WM8903_GPIO_BASE + 1)
#define WM8903_GP3              (WM8903_GPIO_BASE + 2)
#define WM8903_GP4              (WM8903_GPIO_BASE + 3)
#define WM8903_GP5              (WM8903_GPIO_BASE + 4)

/* Interrupt numbers from external peripherals */
#define TPS6586X_INT_BASE       TEGRA_NR_IRQS
#define TPS6586X_INT_END        (TPS6586X_INT_BASE + 32)

#define TEGRA_GPIO_USB1		TEGRA_GPIO_PU3

#define SHUTTLE_FB_PAGES        2			/* At least, 2 video pages */
#define SHUTTLE_FB_HDMI_PAGES   2			/* At least, 2 video pages for HDMI */

#define SHUTTLE_MEM_SIZE	SZ_512M			/* Total memory */
#define SHUTTLE_GPU_MEM_SIZE	SZ_128M			/* Memory reserved for GPU */

#define BETELGEUSE_CAMERA_POWER	TEGRA_GPIO_PV4

#define BETELGEUSE_WLAN_PWR	TEGRA_GPIO_PK5
#define BETELGEUSE_WLAN_RST	TEGRA_GPIO_PK6
#define AKM8975_IRQ_GPIO	TEGRA_GPIO_PV1
#define ADT7461_IRQ_GPIO	TEGRA_GPIO_PN6
#define AC_PRESENT_GPIO		TEGRA_GPIO_PV3
#define BETELGEUSE_NVEC_REQ	TEGRA_GPIO_PBB1	
#define BETELGEUSE_WAKE_FROM_KEY TEGRA_GPIO_PA0 // EC Keyboard Wakeup
#define BETELGEUSE_LOW_BATT	TEGRA_GPIO_PW3

/* Video stuff */
#define BETALGEUSE_BL_ENB		TEGRA_GPIO_PB5
#define BETALGEUSE_LVDS_SHUTDOWN	TEGRA_GPIO_PB2
#define BETALGEUSE_EN_VDD_PANEL		TEGRA_GPIO_PC6
#define BETALGEUSE_BL_VDD		TEGRA_GPIO_PW0
#define BETALGEUSE_BL_PWM		TEGRA_GPIO_PB4 /* PWM - Not used now */
#define BETALGEUSE_HDMI_HPD		TEGRA_GPIO_PN7 /* 1=HDMI plug detected */

/* Active input GPIOs in 2.6.32 - not active in 2.6.36:
* gpio-0   (nvrm_gpio           ) in  hi irq-192 (default) wakeup TEGRA_GPIO_PA0 EC Keyboard Wakeup
* gpio-12  (nvrm_gpio           ) in  lo			TEGRA_GPIO_PB4 LCD_BL_PWM?
* gpio-154 (nvrm_gpio           ) in  lo			TEGRA_GPIO_PT2 s_Vddio_Vid_En
* gpio-161 (nvrm_gpio           ) in  lo irq-353 (default)	TEGRA_GPIO_PU1 CAP_INT
* gpio-162 (nvrm_gpio           ) in  hi irq-354 (default)	TEGRA_GPIO_PU2
* gpio-170 (nvrm_gpio           ) in  hi irq-362 (default)	TEGRA_GPIO_PV2 DOCK_IN_DET
* gpio-175 (nvrm_gpio           ) in  lo irq-367 (default)	TEGRA_GPIO_PV7 GSensor
* gpio-178 (nvrm_gpio           ) in  lo irq-370 (default)	TEGRA_GPIO_PW2 USB hub suspend
* gpio-184 (nvrm_gpio           ) in  hi			TEGRA_GPIO_PX0 s_PCB_ID_GPIO
* gpio-186 (nvrm_gpio           ) in  lo			TEGRA_GPIO_PX2 s_PCB_ID_GPIO
* gpio-187 (nvrm_gpio           ) in  hi irq-379 (default)	TEGRA_GPIO_PX3
*
* Active output GPIOs:
* gpio-220 (nvrm_gpio           ) out hi			TEGRA_GPIO_PBB4 s_VSleep_En
*/

#endif
