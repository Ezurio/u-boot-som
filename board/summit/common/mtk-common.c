// SPDX-License-Identifier: GPL-2.0+
/*
 * Mediatek eMMC Common boot
 *
 * Copyright (C) 2023 Ezurio
 *
 */
#include <env.h>
#include <mmc.h>
#include <env_internal.h>
#include <asm/global_data.h>

#include "../common/common.h"

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
int board_mmc_get_env_dev(int devno)
{
	return devno;
}

#ifdef CONFIG_ENV_MMC_EMMC_HW_PARTITION
uint mmc_get_env_part(struct mmc *mmc)
{
	int devno = 0;
	return emmc_get_boot_side(devno);
}
#endif
#endif

enum env_location env_get_location(enum env_operation op, int prio)
{
	/* env_init() and env_load() iterate priorities until ENVL_UNKNOWN. */
	if (prio)
		return ENVL_UNKNOWN;

#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC)
		return ENVL_MMC;
#else
		return ENVL_NOWHERE;
#endif
}

void set_bootside(void)
{
#if CONFIG_IS_ENABLED(MMC)
	int devno = 0, side;

	env_set_ulong("mmcdev", devno);
	env_set("boot_src", "emmc");
	side = emmc_get_boot_side(devno);
	env_set("bootside", side == 2 ? "b" : "a");
	printf("Booting from eMMC, side %s\n", side == 2 ? "b" : "a");
#else
	env_set("bootside", "a");
#endif

	if (gd->flags & GD_FLG_ENV_DEFAULT) {
		puts("Saving default environment...\n");
		env_save();
	}
}