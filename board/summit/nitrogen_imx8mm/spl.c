// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019, 2021 NXP
 */

#include <hang.h>
#include <init.h>
#include <spl.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <asm/sections.h>

#include <dm/uclass.h>
#include <dm/device.h>
#include <i2c.h>

#define PF8100	0x08
#define SW1_CONFIG2 0x4e
#define SW2_VOLT	0x59
#define SW3_CONFIG2	0x5e
#define SW3_VOLT	0x61
#define SW4_CONFIG2	0x66
#define SW4_MODE1	0x68
#define SW4_VOLT	0x69
#define SW5_VOLT	0x71
#define LDO2_CONFIG2	0x8c
#define LDO3_CONFIG2	0x92

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	switch (boot_dev_spl) {
	case USB_BOOT:
#if !defined(CONFIG_SPL_USB_SDP_SUPPORT)
		return BOOT_DEVICE_MMC2;
#else
		return BOOT_DEVICE_BOARD;
#endif
	case SD1_BOOT:
	case MMC1_BOOT:
		return BOOT_DEVICE_MMC1;
	case SD2_BOOT:
	case MMC2_BOOT:
		return BOOT_DEVICE_MMC2;
	default:
		return BOOT_DEVICE_NONE;
	}
}

static void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

void spl_board_init(void)
{
	arch_misc_init();
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

static int power_init_board(void)
{
	struct udevice *dev;
	struct udevice *i2c_dev = NULL;
	int ret;

	static const struct pmic_cmds {
		u8 addr;
		u8 clr;
		u8 set;
	} cmds[] = {
		{ SW2_VOLT,     0xff, 0x58 }, // DRAM/GPU 0.95V - (0.95 − 0.4) / 0.00625
		{ SW3_VOLT,     0xff, 0x58 }, // ARM 0.95V - (0.95 − 0.4) / 0.00625
		{ SW4_VOLT,     0xff, 0x50 }, // AUX 0.9V - (0.9 − 0.4) / 0.00625
		{ SW5_VOLT,     0xff, 0x58 }, // VPU 0.95V - (0.95 − 0.4) / 0.00625
		{ LDO2_CONFIG2, 0x18, 0x18 }, // VSELECT On
		{ SW1_CONFIG2,  0x1f, 0x1f }, // Phase 0, Max current
		{ SW3_CONFIG2,  0x20, 0x20 }, // Fast Slew Rate
		{ SW4_CONFIG2,  0x20, 0x20 }, // Fast Slew Rate
		{ SW4_MODE1,    0x03, 0x00 }, // Set SW4 to Off mode
		{ LDO3_CONFIG2, 0x02, 0x00 }, // LDO3 Off
	};


	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &dev);
	if (ret) {
		debug("%s: Can't find bus\n", __func__);
		return ret;
	}

	ret = dm_i2c_probe(dev, PF8100, 0, &i2c_dev);
	if (ret) {
		debug("%s: Can't find device id=0x%x\n", __func__, PF8100);
		return ret;
	}

	for (int i = 0; i < ARRAY_SIZE(cmds); i++) {
		if (cmds[i].clr == 0xff)
			ret = dm_i2c_reg_write(i2c_dev, cmds[i].addr,
					      cmds[i].set);
		else
			ret = dm_i2c_reg_clrset(i2c_dev, cmds[i].addr,
						cmds[i].clr, cmds[i].set);
		if (ret) {
			debug("%s dm_i2c_reg_clrset failed, err %d\n",
			      __func__, ret);
			return ret;
		}
	}

	/*
	 * Make sw3 a 180 degree phase shift from sw4,
	 * in case pmic not programmed for dual mode
	 */
	ret = dm_i2c_reg_read(i2c_dev, SW4_CONFIG2);
	if (ret < 0) {
		debug("%s dm_i2c_reg_read failed, err %d\n", __func__, ret);
		return ret;
	}
	
	/* 180 degree phase */
	ret = dm_i2c_reg_write(i2c_dev, SW3_CONFIG2, ret ^ 4);
	if (ret) {
		debug("%s dm_i2c_reg_write failed, err %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

void board_init_f(ulong dummy)
{
	struct udevice *dev;
	int ret;

	arch_cpu_init();

	init_uart_clk(1);

	timer_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	ret = uclass_get_device_by_name(UCLASS_CLK,
					"clock-controller@30380000",
					&dev);
	if (ret < 0) {
		printf("Failed to find clock node. Check device tree\n");
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}
