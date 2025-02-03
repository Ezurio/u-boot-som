// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Carbon AM62 OSM module
 *
 * Copyright (C) 2024 Ezurio
 *
 */

#include <cpu_func.h>
#include <display_options.h>
#include <env.h>
#include <spl.h>
#include <init.h>
#include <video.h>
#include <splash.h>
#include <k3-ddrss.h>
#include <fdt_support.h>
#include <fdt_simplefb.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <dm/uclass.h>
#include <power/regulator.h>
#include <env_internal.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(SPLASH_SCREEN)
// This is security risk, we need to secure this somehow
// there is a way to load splash screen from the fit image
static struct splash_location default_splash_locations[] = {
	{
		.name		= "mmc",
		.storage	= SPLASH_STORAGE_MMC,
		.flags		= SPLASH_STORAGE_FS,
		.devpart	= "1:1",
	},
};

int splash_screen_prepare(void)
{
	return splash_source_load(default_splash_locations,
				ARRAY_SIZE(default_splash_locations));
}
#endif

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((long *)CFG_SYS_SDRAM_BASE, SZ_2G);

#if defined(CONFIG_TARGET_CARBON_AM62_R5)
	puts("DDR RAM detected Size: ");
	print_size(gd->ram_size, "\n");
#endif

	return 0;
}

static int __maybe_unused get_boot_side(int dev)
{
	struct mmc *mmc;

	mmc = find_mmc_device(dev);
	if (!mmc)
		return 0;

	if (!mmc_getcd(mmc))
		mmc->has_init = 0;

	if (mmc_init(mmc))
		return 0;

	if (IS_SD(mmc))
		return 0;

	return EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config);
}

#if IS_ENABLED(CONFIG_ENV_IS_IN_FAT) || IS_ENABLED(CONFIG_ENV_IS_IN_MMC)
#ifdef CONFIG_SYS_MMC_ENV_PART
uint mmc_get_env_part(struct mmc *mmc)
{
	u32 bdev = get_boot_device();
	int devno;

	switch (bdev) {
	case BOOT_DEVICE_MMC2:
		return CONFIG_SYS_MMC_ENV_PART;

	case BOOT_DEVICE_MMC1:
		devno = bdev - BOOT_DEVICE_MMC1;
		return get_boot_side(devno);

	default:
		return 0;
	}
}
#endif

int mmc_get_env_dev(void)
{
	u32 bdev = get_boot_device();

	switch (bdev) {
	case BOOT_DEVICE_MMC1:
		return 0;
	case BOOT_DEVICE_MMC2:
		return 1;
	};

	return CONFIG_SYS_MMC_ENV_DEV;
}
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bdev = get_boot_device();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bdev) {
	case BOOT_DEVICE_MMC2:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_FAT))
			return ENVL_FAT;
		else
			return ENVL_NOWHERE;

	case BOOT_DEVICE_MMC1:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;

	default:
		return ENVL_NOWHERE;
	};
}


#if CONFIG_IS_ENABLED(BOARD_LATE_INIT)
static void set_bootside(void)
{
	u32 bdev = get_boot_device();
	int devno, side;

	switch (bdev) {
	case BOOT_DEVICE_MMC:
		devno = 1;
		env_set_ulong("mmcdev", devno);
		env_set("boot_src", "sd");
		env_set("bootside", "a");
		printf("Booting from SD, side a\n");
		break;

	case BOOT_DEVICE_EMMC:
		devno = 0;
		env_set_ulong("mmcdev", devno);
		env_set("boot_src", "emmc");
		side = get_boot_side(devno);
		env_set("bootside", side == 2 ? "b" : "a");
		printf("Booting from eMMC, side %s\n", side == 2 ? "b" : "a");
		break;

	default:
		break;
	}
}

int board_late_init(void)
{
	set_bootside();

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Carbon AM62x");
	env_set("board_rev", "AM62x");
#endif

	if (gd->flags & GD_FLG_ENV_DEFAULT) {
		puts("Saving default environment...\n");
		env_save();
	}

	return 0;
}
#endif


#if defined(CONFIG_SPL_BUILD)
void spl_board_init(void)
{
	u32 val;

	/* We have 32k crystal, so lets enable it */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Add any TRIM needed for the crystal here.. */
	/* Make sure to mux up to take the SoC 32k from the crystal */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);

	enable_caches();
	if (IS_ENABLED(CONFIG_SPL_SPLASH_SCREEN) && IS_ENABLED(CONFIG_SPL_BMP))
		splash_display();
}
#endif

#if defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret = -1;

	if (IS_ENABLED(CONFIG_FDT_SIMPLEFB))
		ret = fdt_simplefb_enable_and_mem_rsv(blob);

	/* If simplefb is not enabled and video is active, then at least reserve
	 * the framebuffer region to preserve the splash screen while OS is booting
	 */
	if (IS_ENABLED(CONFIG_VIDEO) && IS_ENABLED(CONFIG_OF_LIBFDT)) {
		if (ret && video_is_active())
			return fdt_add_fb_mem_rsv(blob);
	}

	return 0;
}
#endif
