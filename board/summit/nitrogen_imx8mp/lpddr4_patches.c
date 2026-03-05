// SPDX-License-Identifier: GPL-2.0+
/*
 * DDR patching helpers for Nitrogen i.MX8MP.
 *
 * These patches apply size-specific updates on top of the 4GB DDR tables.
 */

#include <linux/types.h> 
#include <linux/sizes.h>
#include <fuse.h>

enum nitrogen_imx8mp_ddr_patch_control {
	DDR_PATCH_2GB_R1 = 1,
	DDR_PATCH_2GB_R2 = 2,
	DDR_PATCH_4GB = 3,
	DDR_PATCH_8GB = 4,
};

#define MEMORY_FUSE_BANK 14
#define MEMORY_FUSE_WORD 3

#if defined(CONFIG_XPL_BUILD)

#include "../common/imx-common.h"

extern struct dram_timing_info dram_timing;

static struct dram_cfg_param ddr_ddrc_cfg_2gb_r1_patch[] = {
	{0x3d400000, 0xa1080020},
	{0x3d400020, 0x1223},
	{0x3d400024, 0x186a000},
	{0x3d400064, 0x6100e0},
	{0x3d4000d0, 0xc003061c},
	{0x3d4000d4, 0x9e0000},
	{0x3d4000dc, 0xd4002d},
	{0x3d4000e0, 0x310000},
	{0x3d400100, 0x1a201b22},
	{0x3d400104, 0x60633},
	{0x3d40010c, 0xc0c000},
	{0x3d400110, 0xf04080f},
	{0x3d400114, 0x2040c0c},
	{0x3d400118, 0x1010007},
	{0x3d40011c, 0x402},
	{0x3d400130, 0x20600},
	{0x3d400134, 0xc100002},
	{0x3d400138, 0xe6},
	{0x3d400144, 0xa00050},
	{0x3d400180, 0x3200018},
	{0x3d400184, 0x28061a8},
	{0x3d400190, 0x497820a},
	{0x3d4001b4, 0x170a},
	{0x3d4000f4, 0x699},
	{0x3d400108, 0x70e1617},
	{0x3d400200, 0x1f},
};

static struct dram_cfg_param ddr_ddrphy_cfg_2gb_r1_patch[] = {
	{0x000200c5, 0x19},
	{0x00020008, 0x320},
};

static struct dram_cfg_param ddr_phy_pie_2gb_r1_patch[] = {
	{0x0002000b, 0x384},
	{0x0002000c, 0xc8},
	{0x0002000d, 0x7d0},
};

static struct dram_cfg_param ddr_fsp_common_2gb_patch[] = {
	{0x00054012, 0x110},
	{0x0005402c, 0x1},
};

static struct dram_cfg_param ddr_fsp0_and_2d_specific_2gb_r1_patch[] = {
	{0x00054003, 0xc80},
	{0x00054019, 0x2dd4},
	{0x0005401a, 0x31},
	{0x0005401f, 0x2dd4},
	{0x00054020, 0x31},
	{0x00054032, 0xd400},
	{0x00054033, 0x312d},
	{0x00054038, 0xd400},
	{0x00054039, 0x312d},
};

static struct dram_cfg_param ddr_ddrc_cfg_2gb_r2_patch[] = {
	{0x3d400000, 0xa1080020},
	{0x3d400200, 0x1f},
};

static struct dram_cfg_param ddr_ddrc_cfg_8gb_patch[] = {
	{0x3d400064, 0x7a017c},
	{0x3d400138, 0x184},
	{0x3d400200, 0x18},
	{0x3d40021c, 0xf07},
	{0x3d402064, 0xc0026},
	{0x3d402138, 0x27},
	{0x3d403064, 0x3000a},
	{0x3d403138, 0xa},
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

void nitrogen_imx8mp_ddr_patch(void)
{
	u32 ctrl = 0;
	bool done = false;

	fuse_read(MEMORY_FUSE_BANK, MEMORY_FUSE_WORD, &ctrl);

	while (!done) {
		switch (ctrl & 0xff) {
		case DDR_PATCH_2GB_R1:
			patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
				  ddr_ddrc_cfg_2gb_r1_patch,
				  ARRAY_SIZE(ddr_ddrc_cfg_2gb_r1_patch));
			patch_ddr(dram_timing.ddrphy_cfg, dram_timing.ddrphy_cfg_num,
				  ddr_ddrphy_cfg_2gb_r1_patch,
				  ARRAY_SIZE(ddr_ddrphy_cfg_2gb_r1_patch));
			patch_ddr(dram_timing.ddrphy_pie, dram_timing.ddrphy_pie_num,
				  ddr_phy_pie_2gb_r1_patch,
				  ARRAY_SIZE(ddr_phy_pie_2gb_r1_patch));
			/* Apply common FSP settings to all FSPs */
			patch_all_fsp(&dram_timing, ddr_fsp_common_2gb_patch,
				      ARRAY_SIZE(ddr_fsp_common_2gb_patch));
			/* Apply FSP0-specific settings */
			patch_ddr(dram_timing.fsp_msg[0].fsp_cfg, dram_timing.fsp_msg[0].fsp_cfg_num,
				  ddr_fsp0_and_2d_specific_2gb_r1_patch,
				  ARRAY_SIZE(ddr_fsp0_and_2d_specific_2gb_r1_patch));
			/* Apply FSP3 (2D) specific settings */
			patch_ddr(dram_timing.fsp_msg[3].fsp_cfg, dram_timing.fsp_msg[3].fsp_cfg_num,
				  ddr_fsp0_and_2d_specific_2gb_r1_patch,
				  ARRAY_SIZE(ddr_fsp0_and_2d_specific_2gb_r1_patch));

			for (size_t i = 0; i < dram_timing.fsp_msg_num; i++) {
				if (dram_timing.fsp_msg[i].drate == 4000)
					dram_timing.fsp_msg[i].drate = 3200;
			}
			if (ARRAY_SIZE(dram_timing.fsp_table) > 0)
				dram_timing.fsp_table[0] = 3200;
			done = true;
			break;

		case DDR_PATCH_2GB_R2:
			patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
				  ddr_ddrc_cfg_2gb_r2_patch,
				  ARRAY_SIZE(ddr_ddrc_cfg_2gb_r2_patch));
			patch_all_fsp(&dram_timing, ddr_fsp_common_2gb_patch,
				      ARRAY_SIZE(ddr_fsp_common_2gb_patch));
			done = true;
			break;

		case DDR_PATCH_4GB:
			/* 4GB base config, no patches needed */
			done = true;
			break;

		case DDR_PATCH_8GB:
			patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
				  ddr_ddrc_cfg_8gb_patch,
				  ARRAY_SIZE(ddr_ddrc_cfg_8gb_patch));
			done = true;
			break;

		default:
#if defined(CONFIG_NITROGEN_IMX8MP_2GB_R1)
			ctrl = DDR_PATCH_2GB_R1;
#elif defined(CONFIG_NITROGEN_IMX8MP_2GB_R2)
			ctrl = DDR_PATCH_2GB_R2;
#elif defined(CONFIG_NITROGEN_IMX8MP_8GB)
			ctrl = DDR_PATCH_8GB;
#else
			ctrl = DDR_PATCH_4GB;
#endif
			break;
		}
	}
}

#else /* CONFIG_XPL_BUILD */

int board_phys_sdram_size(phys_size_t *size)
{
	u32 ctrl = 0;

	if (!size)
		return -EINVAL;

	fuse_read(MEMORY_FUSE_BANK, MEMORY_FUSE_WORD, &ctrl);

	switch (ctrl & 0xff) {
	case DDR_PATCH_2GB_R1:
	case DDR_PATCH_2GB_R2:
		*size = SZ_2G;
		break;

	case DDR_PATCH_4GB:
		*size = SZ_4G;
		break;

	case DDR_PATCH_8GB:
		*size = (SZ_4G + SZ_4G);
		break;

	default:
#if defined(CONFIG_NITROGEN_IMX8MP_2GB_R1) || \
    defined(CONFIG_NITROGEN_IMX8MP_2GB_R2)
		*size = SZ_2G;
#elif defined(CONFIG_NITROGEN_IMX8MP_4GB)
		*size = SZ_4G;
#elif defined(CONFIG_NITROGEN_IMX8MP_8GB)
		*size = (SZ_4G + SZ_4G);
#else
		*size = SZ_4G;
#endif
		break;
	}

	return 0;
}

#endif /* CONFIG_XPL_BUILD */
