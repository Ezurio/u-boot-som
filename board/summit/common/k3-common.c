// SPDX-License-Identifier: GPL-2.0+
/*
 * k3 eMMC Common boot 
 *
 * Copyright (C) 2023 Ezurio
 *
 */
#include <spl.h>
#include <env.h>
#include <mmc.h>
#include <env.h>
#include <env_internal.h>
#include <limits.h>
#include <linux/sizes.h>
#include <linux/libfdt.h> 
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

	return CONFIG_SYS_MMC_ENV_DEV;
}

#ifdef CONFIG_SYS_MMC_ENV_PART
uint mmc_get_env_part(struct mmc *mmc)
{
	u32 bdev = get_boot_device();
	int devno;

	switch (bdev) {
	case BOOT_DEVICE_MMC:
		return CONFIG_SYS_MMC_ENV_PART;

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
	case BOOT_DEVICE_GPMC_NAND:
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
	const char *side_str;

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
		side = emmc_get_boot_side(devno);
		env_set("bootside", side == 2 ? "b" : "a");
		printf("Booting from eMMC, side %s\n", side == 2 ? "b" : "a");
		break;

	case BOOT_DEVICE_GPMC_NAND:
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

#if defined(CONFIG_XPL_BUILD)
#if IS_ENABLED(CONFIG_K3_DDRSS)
static int fdt_patch_table(void *fdt, int mem_offset, const char *name,
			   const struct ddr_patch_record *patch)
{
	if (!patch)
		return 0;

	while (patch->off != UINT32_MAX) {
		u32 val = cpu_to_fdt32(patch->val);

		int ret = fdt_setprop_inplace_namelen_partial(fdt, mem_offset,
			name, strlen(name), patch->off * sizeof(val), &val, sizeof(val));
		if (ret)
			return ret;

		patch++;
	}

	return 0;
}

static int fdt_update_ram_timings(void *fdt, const struct ddrss_patch *patch)
{
	int ret;
	int mem_offset;

	if (!patch)
		return 0;

	mem_offset = fdt_path_offset(fdt, "/memorycontroller@f300000");
	if (mem_offset < 0)
		return -ENODEV;

	ret = fdt_patch_table(fdt, mem_offset, "ti,ctl-data", patch->ctl_patch);
	if (ret)
		return ret;

	ret = fdt_patch_table(fdt, mem_offset, "ti,pi-data", patch->pi_patch);
	if (ret)
		return ret;

	ret = fdt_patch_table(fdt, mem_offset, "ti,phy-data", patch->phy_patch);
	if (ret)
		return ret;

	return 0;
}

static const struct ddrss_patch *
get_ddrss_patch(u16 id, const struct ddrss_patch *patches)
{
	if (!patches || !id)
		return NULL;

	while (patches->id) {
		if (patches->id == id)
			return patches;
		patches++;
	}

	return NULL;
}	

int setup_ram(const struct ddrss_patch *patches)
{
	u64 start[CONFIG_NR_DRAM_BANKS] = { CFG_SYS_SDRAM_BASE, 0x880000000ULL };
	u64 size[CONFIG_NR_DRAM_BANKS] = { 0, 0 };

	u64 ram_size;
	void *fdt = (void *)gd->fdt_blob;
	int banks;
	int ret;
	u16 ram_type;

	fdtdec_setup_mem_size_base_lowest();

	ram_size = gd->ram_size;

	ret = nvmem_cell_rw("ram-type", false, &ram_type, sizeof(ram_type));
	if (!ret) {
		switch (ram_type) {
		case 1:
			ram_size = SZ_1G;
			break;
		case 2:
			ram_size = SZ_2G;
			break;
		case 3:
			ram_size = SZ_4G;
			break;
		case 4:
			ram_size = SZ_8G;
			break;
		}
	}

	if (gd->ram_size != ram_size) {
#if !CONFIG_IS_ENABLED(PHYS_64BIT) || CONFIG_NR_DRAM_BANKS < 2
		if (ram_size > SZ_2G)
			ram_size = SZ_2G;
#endif

		if (ram_size <= SZ_2G) {
			banks = 1;
			size[0] = ram_size;
			size[1] = 0;
		} else {
			banks = 2;
			size[0] = SZ_2G;
			size[1] = ram_size - SZ_2G;
		}
		gd->ram_size = size[0];

		ret = fdt_fixup_memory_banks(fdt, start, size, banks);
		if (ret)
			return ret;
	}

	return fdt_update_ram_timings(fdt, get_ddrss_patch(ram_type, patches));
}
#endif /* CONFIG_K3_DDRSS */
#endif /* CONFIG_XPL_BUILD */
