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
#include <net-common.h> 

#include <asm/mach-imx/boot_mode.h>
#include <asm/arch/ddr.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>

#include "imx-common.h"

DECLARE_GLOBAL_DATA_PTR;

void set_serial_number(void)
{
	char serialbuf[13];
	unsigned char mac[8];

	if (!IS_ENABLED(CONFIG_USB_GADGET))
		return;

	if (env_get("serial#"))
		return;

	imx_get_mac_from_fuse(0, mac);
	if (!is_valid_ethaddr(mac)) {
		printf("fuse not set, can't set serial\n");
		return;
	}

	snprintf(serialbuf, sizeof(serialbuf),
		 "%02x%02x%02x%02x%02x%02x", mac[0],
		 mac[1], mac[2], mac[3],
		 mac[4], mac[5]);
	printf("serial: %s\n", serialbuf);
	env_set("serial#", serialbuf);
}

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
	enum boot_device bdev = get_boot_device();
	int devno;

	switch (bdev) {
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
		return CONFIG_ENV_MMC_EMMC_HW_PARTITION;

	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		devno = bdev - MMC1_BOOT;
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
	case SD1_BOOT:
	case SD2_BOOT:
	case SD3_BOOT:
		return ENVL_FAT;
#endif

#if CONFIG_IS_ENABLED(ENV_IS_IN_MMC)
	case MMC1_BOOT:
	case MMC2_BOOT:
	case MMC3_BOOT:
		return ENVL_MMC;
#endif

#if CONFIG_IS_ENABLED(ENV_IS_IN_NAND)
	case NAND_BOOT:
		return ENVL_NAND;
#endif

	default:
		return ENVL_NOWHERE;
	}
}

void set_bootside(void)
{
	enum boot_device bdev = get_boot_device();
	int devno, side;
	const char *side_str;

	switch (bdev) {
#if (CONFIG_BOOTDELAY <= 0)
	case USB_BOOT:
		bdev = SD2_BOOT;
		fallthrough;
#endif

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
		side = emmc_get_boot_side(devno);
		env_set("bootside", side == 2 ? "b" : "a");
		printf("Booting from eMMC, side %s\n", side == 2 ? "b" : "a");
		break;

	case NAND_BOOT:
		env_set("mmcdev", NULL);
		env_set("boot_src", "nand");
		side_str = env_get("bootside");
		if (!side_str) {
			side_str = "a";
			env_set("bootside", side_str);
		}
		printf("Booting from NAND, side %s\n", side_str);
		break;

	default:
		break;
	}

	if (gd->flags & GD_FLG_ENV_DEFAULT) {
		puts("Saving default environment...\n");
		env_save();
	}
}

void patch_ddr(struct dram_cfg_param *data, int data_size, 
	struct dram_cfg_param *patch, int patch_size)
{
	int i, j;
	int start_pos = 0;
	bool found;

	for (i = 0; i < patch_size; i++) {
		found = false;
		/* Start search from last found position since registers are in order */
		for (j = start_pos; j < data_size; j++) {
			if (data[j].reg == patch[i].reg) {
				data[j].val = patch[i].val;
				start_pos = j + 1;
				found = true;
				break;
			}
		}
		if (!found) {
			printf("DDR patch error: register 0x%x not found in data\n", 
			       patch[i].reg);
		}
	}
}

int erase_ddr(struct dram_cfg_param *data, int data_size,
	u32 *regs_to_erase, int erase_size)
{
	int i, j, k;
	int end_pos = data_size - 1;
	bool found;

	/* Iterate backwards to erase from end */
	for (i = erase_size - 1; i >= 0; i--) {
		found = false;
		/* Search backwards from last found position since registers are in order */
		for (j = end_pos; j >= 0; j--) {
			if (data[j].reg == regs_to_erase[i]) {
				/* Shift remaining elements down */
				for (k = j; k < data_size - 1; k++) {
					data[k] = data[k + 1];
				}
				data_size--;
				end_pos = j - 1;  /* Next search ends before current position */
				found = true;
				break;
			}
		}
		if (!found) {
			printf("DDR erase warning: register 0x%x not found in data\n", 
			       regs_to_erase[i]);
		}
	}
	
	return data_size;
};
