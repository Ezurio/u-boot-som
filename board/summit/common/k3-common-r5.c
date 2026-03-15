// SPDX-License-Identifier: GPL-2.0+
/*
 * k3 Memory setup for R5 SPL
 *
 * Copyright (C) 2026 Ezurio
 *
 */
#include <init.h>
#include <spl.h>
#include <limits.h>
#include <linux/sizes.h>
#include <linux/libfdt.h> 
#include <asm/arch/hardware.h>
#include <asm/global_data.h> 

#include "k3-common.h"
#include "eeprom-common.h"

DECLARE_GLOBAL_DATA_PTR;

static __section(".data") u16 ram_type = 0;
static __section(".data") u64 ram_size = 0;

int do_board_detect(void)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int bank;

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

	dram_init();
	dram_init_banksize();

	// DRSS driver retrieve memory info only from fdt, so we need to fixup fdt here
	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] = gd->bd->bi_dram[bank].start;
		size[bank] = gd->bd->bi_dram[bank].size;
	}

	return fdt_fixup_memory_banks((void *)gd->fdt_blob, start, size,
		CONFIG_NR_DRAM_BANKS);
}

const struct ddrss_patch* __weak get_lpddr_patch_data(void)
{
	return NULL;
}

static int ctl_reg_update(u32 *ctl_regs, const struct ddr_patch_record *patch)
{
	if (!patch)
		return 0;

	while (patch->reg != UINT32_MAX) {
		ctl_regs[patch->reg] = patch->val;
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

int dram_init(void)
{
	if (!ram_size)
		return fdtdec_setup_mem_size_base();

	gd->ram_base = (phys_addr_t)CFG_SYS_SDRAM_BASE;
	gd->ram_size = (phys_size_t)(ram_size > SZ_2G ? SZ_2G : ram_size);

	return 0;
}

int dram_init_banksize(void)
{
	if (!ram_size)
		return fdtdec_setup_memory_banksize();

	gd->bd->bi_dram[0].start = (phys_addr_t)CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = ram_size > SZ_2G ? SZ_2G : ram_size;

#if CONFIG_NR_DRAM_BANKS > 1
	gd->bd->bi_dram[1].start = (phys_addr_t)0x880000000ULL;
	gd->bd->bi_dram[1].size =
		(phys_size_t)ram_size - gd->bd->bi_dram[0].size;
#endif

	return 0;
}
