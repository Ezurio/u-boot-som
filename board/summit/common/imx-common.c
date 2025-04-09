// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP eMMC Common boot 
 *
 * Copyright (C) 2023 Ezurio
 *
 */
#include <env.h>
#include <mmc.h>
#include <env_internal.h>
#include <linux/sizes.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h> 

DECLARE_GLOBAL_DATA_PTR;

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
int board_mmc_get_env_dev(int devno)
{
	return devno;
}

#ifdef CONFIG_SYS_MMC_ENV_PART
uint mmc_get_env_part(struct mmc *mmc)
{
	enum boot_device bdev = get_boot_device();
	int devno;

	switch (bdev) {
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
		return CONFIG_SYS_MMC_ENV_PART;

	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		devno = bdev - MMC1_BOOT;
		return get_boot_side(devno);

	default:
		return 0;
	}
}
#endif
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	u32 bdev = get_boot_device();

	if (prio)
		return ENVL_UNKNOWN;

	switch (bdev) {
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_FAT))
			return ENVL_FAT;
		else
			return ENVL_NOWHERE;

	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		if (CONFIG_IS_ENABLED(ENV_IS_IN_MMC))
			return ENVL_MMC;

	default:
		return ENVL_NOWHERE;
	};
}

void set_bootside(void)
{
	enum boot_device bdev = get_boot_device();
	int devno, side;

	switch (bdev) {
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
		devno = bdev - SD1_BOOT;
		env_set_ulong("mmcdev", devno);
		env_set("boot_src", "sd");
		env_set("bootside", "a");
		printf("Booting from SD, side a\n");
		break;

	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		devno = bdev - MMC1_BOOT;
		env_set_ulong("mmcdev", devno);
		env_set("boot_src", "emmc");
		side = get_boot_side(devno);
		env_set("bootside", side == 2 ? "b" : "a");
		printf("Booting from eMMC, side %s\n", side == 2 ? "b" : "a");
		break;

	default:
		break;
	}

	if (gd->flags & GD_FLG_ENV_DEFAULT) {
		puts("Saving default environment...\n");
		env_save();
	}
}
