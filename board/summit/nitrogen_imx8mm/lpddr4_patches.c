// SPDX-License-Identifier: GPL-2.0+
/*
 * DDR patching helpers for Nitrogen i.MX8MM.
 *
 * These patches apply size-specific updates on top of the 2GB DDR tables.
 */

#include <linux/types.h> 
#include <linux/sizes.h>
#include <fuse.h>

enum nitrogen_imx8mm_ddr_patch_control {
	DDR_PATCH_1GB = 1,
	DDR_PATCH_2GB = 2,
	DDR_PATCH_4GB = 3,
};

#define MEMORY_FUSE_BANK 14
#define MEMORY_FUSE_WORD 3

#if defined(CONFIG_XPL_BUILD)
#include "../common/imx-common.h"

extern struct dram_timing_info dram_timing;

static struct dram_cfg_param ddr_ddrc_cfg_1gb_patch[] = {
	{0x3d400000, 0xa1081020},
	{0x3d400020, 0x223},
	{0x3d40020c, 0x1f000000},
	{0x3d400204, 0x70707},
	{0x3d400214, 0x6060606},
	{0x3d400218, 0x6060606},
	{0x3d402020, 0x21},
	{0x3d403020, 0x21},
};

static struct dram_cfg_param ddr_ddrphy_cfg_1gb_patch[] = {
	{0x00020018, 0x1},
};

static u32 ddr_ddrphy_cfg_1gb_erase[] = {
	0x0001205f, 0x0001215f, 0x0001305f, 0x0001315f,
	0x0011205f, 0x0011215f, 0x0011305f, 0x0011315f,
	0x0021205f, 0x0021215f, 0x0021305f, 0x0021315f,
	0x0001204d, 0x0001214d, 0x0001304d, 0x0001314d,
	0x0011204d, 0x0011214d, 0x0011304d, 0x0011314d,
	0x0021204d, 0x0021214d, 0x0021304d, 0x0021314d,
	0x00012049, 0x00012149, 0x00013049, 0x00013149,
	0x00112049, 0x00112149, 0x00113049, 0x00113149,
	0x00212049, 0x00212149, 0x00213049, 0x00213149,
	0x00012043, 0x00012143, 0x00013043, 0x00013143,
	0x00112043, 0x00112143, 0x00113043, 0x00113143,
	0x00212043, 0x00212143, 0x00213043, 0x00213143,
};

static u32 ddr_phy_pie_1gb_erase[] = {
	0x00012011, 0x00012012, 0x00012013, 0x00012018,
	0x00012002, 0x000120b2, 0x000121b4, 0x000122b4,
	0x000123b4, 0x000124b4, 0x000125b4, 0x000126b4,
	0x000127b4, 0x000128b4, 0x00013011, 0x00013012,
	0x00013013, 0x00013018, 0x00013002, 0x000130b2,
	0x000131b4, 0x000132b4, 0x000133b4, 0x000134b4,
	0x000135b4, 0x000136b4, 0x000137b4, 0x000138b4,
};

static u32 ddr_fsp_1gb_erase[] = {
	0x0005402b,
	0x0005402c,
};

static struct dram_cfg_param ddr_ddrc_cfg_4gb_patch[] = {
	{0x3d400000, 0xa3080020},
	{0x3d400200, 0x17},
};

static struct dram_cfg_param ddr_fsp_4gb_patch[] = {
	{0x00054012, 0x310},
	{0x0005402c, 0x3},
};

/* Helper function to patch all FSP messages with the same configuration */
static void patch_all_fsp(struct dram_timing_info *timing,
			  struct dram_cfg_param *patch, int patch_size)
{
	for (unsigned int i = 0; i < timing->fsp_msg_num; i++) {
		patch_ddr(timing->fsp_msg[i].fsp_cfg,
			  timing->fsp_msg[i].fsp_cfg_num,
			  patch, patch_size);
	}
}

/* Helper function to erase registers from all FSP messages */
static void erase_all_fsp(struct dram_timing_info *timing,
			  u32 *regs_to_erase, int erase_size)
{
	for (unsigned int i = 0; i < timing->fsp_msg_num; i++) {
		int new_size = erase_ddr(timing->fsp_msg[i].fsp_cfg,
					 timing->fsp_msg[i].fsp_cfg_num,
					 regs_to_erase, erase_size);
		timing->fsp_msg[i].fsp_cfg_num = new_size;
	}
}

void nitrogen_imx8mm_ddr_patch(void)
{
	u32 ctrl = 0;
	int new_size;
	bool done = false;

	fuse_read(MEMORY_FUSE_BANK, MEMORY_FUSE_WORD, &ctrl);

	while (!done) {
		switch (ctrl & 0xff) {
		case DDR_PATCH_1GB:
			/* Erase registers not present in 1GB first */
			new_size = erase_ddr(dram_timing.ddrphy_cfg,
						 dram_timing.ddrphy_cfg_num,
						 ddr_ddrphy_cfg_1gb_erase,
						 ARRAY_SIZE(ddr_ddrphy_cfg_1gb_erase));
			dram_timing.ddrphy_cfg_num = new_size;

			new_size = erase_ddr(dram_timing.ddrphy_pie,
						 dram_timing.ddrphy_pie_num,
						 ddr_phy_pie_1gb_erase,
						 ARRAY_SIZE(ddr_phy_pie_1gb_erase));
			dram_timing.ddrphy_pie_num = new_size;

			erase_all_fsp(&dram_timing, ddr_fsp_1gb_erase,
				      ARRAY_SIZE(ddr_fsp_1gb_erase));

			/* Apply value patches to reduced arrays */
			patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
				  ddr_ddrc_cfg_1gb_patch,
				  ARRAY_SIZE(ddr_ddrc_cfg_1gb_patch));
			patch_ddr(dram_timing.ddrphy_cfg, dram_timing.ddrphy_cfg_num,
				  ddr_ddrphy_cfg_1gb_patch,
				  ARRAY_SIZE(ddr_ddrphy_cfg_1gb_patch));
			done = true;
			break;

		case DDR_PATCH_4GB:
			patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
				  ddr_ddrc_cfg_4gb_patch,
				  ARRAY_SIZE(ddr_ddrc_cfg_4gb_patch));
			patch_all_fsp(&dram_timing, ddr_fsp_4gb_patch,
				      ARRAY_SIZE(ddr_fsp_4gb_patch));
			done = true;
			break;

		case DDR_PATCH_2GB:
			/* 2GB base config, no patches needed */
			done = true;
			break;

		default:
#if defined(CONFIG_NITROGEN_IMX8MM_1GB)
			ctrl = DDR_PATCH_1GB;
#elif defined(CONFIG_NITROGEN_IMX8MM_4GB)
			ctrl = DDR_PATCH_4GB;
#else
			ctrl = DDR_PATCH_2GB;
#endif
			break;
		}
	}
}

#else

int board_phys_sdram_size(phys_size_t *size)
{
	u32 ctrl = 0;

	if (!size)
		return -EINVAL;

	fuse_read(MEMORY_FUSE_BANK, MEMORY_FUSE_WORD, &ctrl);

	switch (ctrl & 0xff) {
	case DDR_PATCH_1GB:
		*size = SZ_1G;
		break;

	case DDR_PATCH_2GB:
		*size = SZ_2G;
		break;

	case DDR_PATCH_4GB:
		*size = SZ_4G;
		break;

	default:
#if defined(CONFIG_NITROGEN_IMX8MM_1GB)
		*size = SZ_1G;
#elif defined(CONFIG_NITROGEN_IMX8MM_4GB)
		*size = SZ_4G;
#else
		*size = SZ_2G;
#endif
		break;
	}

	return 0;
}

#endif
