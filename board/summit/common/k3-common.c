// SPDX-License-Identifier: GPL-2.0+
/*
 * k3 eMMC Common boot 
 *
 * Copyright (C) 2024 Ezurio
 *
 */
#include <init.h>
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

#define AM64_DDRSS_SS_BASE	0x0F300000
#define DDRSS_V2A_CTL_REG	0x0020

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
static u64 __section(".data") ram_size = 0;
static u16 __section(".data") ram_type = 0;

int do_board_detect(void)
{
	int ret = nvmem_cell_rw("ram-type", false, &ram_type, sizeof(ram_type));
	if (ret)
		return 0;

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
		ram_size = SZ_4G * 2;
		break;
	default:
		return 0;
	}

#if !IS_ENABLED(CONFIG_PHYS_64BIT) || CONFIG_NR_DRAM_BANKS < 2
	ram_size = ram_size > SZ_2G ? SZ_2G : ram_size;
#endif

// DRSS driver retrieve memory info only from fdt, so we need to fixup fdt here
#if IS_ENABLED(CONFIG_K3_DDRSS)
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	void *fdt = (void *)gd->fdt_blob;
	int bank;

	dram_init();
	dram_init_banksize();

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = gd->bd->bi_dram[bank].start;
		size[bank] = gd->bd->bi_dram[bank].size;
	}

	return fdt_fixup_memory_banks(fdt, start, size, CONFIG_NR_DRAM_BANKS);
#else
	return 0;
#endif
}

int dram_init(void)
{
	if (!ram_size)
		return fdtdec_setup_mem_size_base_lowest();

	gd->ram_base = (phys_addr_t)CFG_SYS_SDRAM_BASE;
	gd->ram_size = (phys_size_t)(ram_size > SZ_2G ? SZ_2G : ram_size);

#if IS_ENABLED(CONFIG_K3_DDRSS) && IS_ENABLED(CONFIG_SOC_K3_AM625)
	/*
	 * HACK: ddrss driver support 2GB RAM by default
	 * V2A_CTL_REG should be updated to support other RAM size
	 */
	if (ram_size > SZ_2G)
		writel(0x00000210, AM64_DDRSS_SS_BASE + DDRSS_V2A_CTL_REG);
#endif

	return 0;
}

int dram_init_banksize(void)
{
	if (!ram_size)
		return fdtdec_setup_memory_banksize();

	gd->bd->bi_dram[0].start = (phys_addr_t)CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =
		(phys_size_t)(ram_size > SZ_2G ? SZ_2G : ram_size);

#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = (phys_addr_t)0x880000000ULL;
	gd->bd->bi_dram[1].size =
		(phys_size_t)(ram_size - gd->bd->bi_dram[0].size);
#endif

	return 0;
}

#if IS_ENABLED(CONFIG_K3_DDRSS)
const struct ddrss_patch* __weak get_lpddr_patch_data(void)
{
	return NULL;
}

static int ctl_reg_update(u32 *ctl_regs, const struct ddr_patch_record *patch)
{
	if (!patch)
		return 0;

	while (patch->off != UINT32_MAX) {
		ctl_regs[patch->off] = patch->val;
		patch++;
	}

	return 0;
}

void k3_lpddr4_patch(u32* ctl_regs, u32* pi_regs, u32* phy_regs)
{
	const struct ddrss_patch *patch = get_lpddr_patch_data();

	// Do nothing if no patch data or ram_type is not set
	if (!patch || !ram_type)
		return;

	// Find the matching patch data for the detected ram_type
	while (patch->id && patch->id != ram_type)
		patch++;

	// Exit if no matching patch found
	if (!patch->id)
		return;

	// Apply the patches
	ctl_reg_update(ctl_regs, patch->ctl_patch);
	ctl_reg_update(pi_regs, patch->pi_patch);
	ctl_reg_update(phy_regs, patch->phy_patch);
}
#endif /* CONFIG_K3_DDRSS */
#else
int dram_init(void)
{
	return fdtdec_setup_mem_size_base_lowest();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}
#endif /* CONFIG_XPL_BUILD */
