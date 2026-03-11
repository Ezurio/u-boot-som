// SPDX-License-Identifier: GPL-2.0+
/*
 * k3 eMMC Common boot 
 *
 * Copyright (C) 2024 Ezurio
 *
 */
#include <init.h>
#include <spl.h>
#include <mmc.h>
#include <env.h>
#include <env_internal.h>
#include <asm/arch/hardware.h>
#include <asm/global_data.h> 

#include "common.h"
#include "k3-common.h"
#include "eeprom-common.h"

DECLARE_GLOBAL_DATA_PTR;

static int __maybe_unused emmc_get_boot_side(int dev)
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
int mmc_get_env_dev(void)
{
	u32 bdev = get_boot_device();

	switch (bdev) {
	case BOOT_DEVICE_EMMC:
		return 0;

	case BOOT_DEVICE_MMC:
		return 1;
	}

	return CONFIG_ENV_MMC_DEVICE_INDEX;
}

#ifdef CONFIG_ENV_MMC_EMMC_HW_PARTITION
uint mmc_get_env_part(struct mmc *mmc)
{
	u32 bdev = get_boot_device();
	int devno;

	switch (bdev) {
	case BOOT_DEVICE_MMC:
		return CONFIG_ENV_MMC_EMMC_HW_PARTITION;

	case BOOT_DEVICE_EMMC:
		devno = 0;
		return emmc_get_boot_side(devno);

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
#if CONFIG_IS_ENABLED(ENV_IS_IN_FAT)
	case BOOT_DEVICE_MMC:
		return ENVL_FAT;
#endif

#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC)
	case BOOT_DEVICE_EMMC:
		return ENVL_MMC;
#endif

#if CONFIG_IS_ENABLED(ENV_IS_IN_NAND)
	case BOOT_DEVICE_NAND:
		return ENVL_NAND;
#endif

	default:
		return ENVL_NOWHERE;
	}
}

void set_bootside(void)
{
	u32 bdev = get_boot_device();
	int devno, side;
	const char __maybe_unused *side_str;

	switch (bdev) {
#if CONFIG_IS_ENABLED(MMC)
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
		side = emmc_get_boot_side(devno);
		env_set("bootside", side == 2 ? "b" : "a");
		printf("Booting from eMMC, side %s\n", side == 2 ? "b" : "a");
		break;
#endif

#if CONFIG_IS_ENABLED(MTD_RAW_NAND)
	case BOOT_DEVICE_NAND:
		env_set("mmcdev", NULL);
		env_set("boot_src", "nand");
		side_str = env_get("bootside");
		if (!side_str) {
			side_str = "a";
			env_set("bootside", side_str);
		}
		printf("Booting from NAND, side %s\n", side_str);
		break;
#endif

	default:
		break;
	}

	if (gd->flags & GD_FLG_ENV_DEFAULT) {
		puts("Saving default environment...\n");
		env_save();
	}
}

int dram_init(void)
{
	if (!gd->ram_size)
		return fdtdec_setup_mem_size_base();

	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}
