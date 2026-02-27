// SPDX-License-Identifier: GPL-2.0+
/*
 * DDR patching helpers for Summit SOM i.MX8MP.
 *
 * These patches apply size-specific updates on top of the 4GB DDR tables.
 */

#include <linux/types.h>
#include <linux/sizes.h>
#include <fuse.h>

enum summitsom_imx8mp_ddr_patch_control {
	DDR_PATCH_1GB = 1,
	DDR_PATCH_2GB = 2,
	DDR_PATCH_4GB = 3,
	DDR_PATCH_512MB = 4,
	DDR_PATCH_2GB_R2 = 5,
};

#define MEMORY_FUSE_BANK 14
#define MEMORY_FUSE_WORD 0

#if defined(CONFIG_XPL_BUILD)

#include "../common/imx-common.h"

extern struct dram_timing_info dram_timing;

/* ============================================================================
 * 2GB Configuration Patches (MT53D512M32D2-046 IT:A)
 * ============================================================================ */

static struct dram_cfg_param ddr_ddrc_cfg_2gb_patch[] = {
	{0x3d400000, 0xa1080020},  /* MSTR: 2GB density configuration */
	{0x3d400200, 0x1f},        /* ADDRMAP0: Different address mapping */
};

static struct dram_cfg_param ddr_fsp_common_2gb_patch[] = {
	{0x54012, 0x110},  /* PHYCALMode: 2GB CS configuration */
	{0x5402c, 0x1},    /* DRAMFreq: Adjusted for 2GB */
};

/* ============================================================================
 * 1GB Configuration Patches (MT53E256M32D2-046 IT:B)
 * ============================================================================ */

static struct dram_cfg_param ddr_ddrc_cfg_1gb_patch[] = {
	{0x3d400000, 0xa1080020},  /* MSTR: 1GB density configuration */
	{0x3d400064, 0x7a00b4},    /* RFSHTMG: Refresh timing for 1GB */
	{0x3d400138, 0xbc},        /* ZQCTL0: ZQ calibration timing */
	{0x3d400200, 0x1f},        /* ADDRMAP0: Address mapping */
	{0x3d400218, 0xf070707},   /* ADDRMAP6: Bank/row address mapping */
	{0x3d402064, 0xc0012},     /* PERF Read CAM */
	{0x3d402138, 0x13},        /* DFITMG1: DFI timing */
	{0x3d403064, 0x30005},     /* PERF Read CAM */
	{0x3d403138, 0x5},         /* DFITMG1: DFI timing */
};

static struct dram_cfg_param ddr_fsp_common_1gb_patch[] = {
	{0x54012, 0x110},  /* PHYCALMode: 1GB CS configuration */
	{0x5402c, 0x1},    /* DRAMFreq: Adjusted for 1GB */
};

/* ============================================================================
 * 512MB Configuration Patches (MT53E128M32D2-053 IT:B)
 * NOTE: This configuration runs at 3600MHz instead of 4000MHz
 * ============================================================================ */

static struct dram_cfg_param ddr_ddrc_cfg_512mb_patch[] = {
	{0x3d400000, 0xa1080020},    /* MSTR: 512MB density configuration */
	{0x3d400024, 0x1b77400},     /* RFSHCTL3: Refresh control */
	{0x3d400064, 0x6d0075},      /* RFSHTMG: Refresh timing for 512MB @ 3600MHz */
	{0x3d4000d0, 0xc00306df},    /* INIT0: Initialization timing */
	{0x3d4000d4, 0xb10000},      /* INIT1: Initialization timing */
	{0x3d4000dc, 0xe40036},      /* INIT3: MR0/MR1 programming */
	{0x3d400100, 0x1d241e26},    /* DRAMTMG0: tRAS/tRCD/tRP timing */
	{0x3d400104, 0x70739},       /* DRAMTMG1: tRC/tRFC timing */
	{0x3d40010c, 0xd0d000},      /* DRAMTMG3: tMOD/tMRD timing */
	{0x3d400110, 0x11040911},    /* DRAMTMG4: tRRD/tCCD/tRTP timing */
	{0x3d400114, 0x2050e0e},     /* DRAMTMG5: tCKE/tCKESR timing */
	{0x3d400118, 0x1010008},     /* DRAMTMG8: Post-mpc/mpr timing */
	{0x3d400130, 0x20700},       /* ODTCFG: ODT configuration */
	{0x3d400134, 0xd100002},     /* ODTMAP: ODT mapping */
	{0x3d400138, 0x7c},          /* ZQCTL0: ZQ calibration timing */
	{0x3d400144, 0xb4005a},      /* ZQCTL1: ZQ calibration values */
	{0x3d400180, 0x384001b},     /* DFIUPD0: DFI update timing */
	{0x3d400184, 0x2d06ddd},     /* DFIUPD1: DFI update timing */
	{0x3d400190, 0x49b820c},     /* DFIMISC: DFI misc settings */
	{0x3d4001b4, 0x1b0c},        /* DFISTAT: DFI status */
	{0x3d400108, 0x810191a},     /* DRAMTMG2: Read/write timing */
	{0x3d400200, 0x1f},          /* ADDRMAP0: Address mapping */
	{0x3d400218, 0xf0f0707},     /* ADDRMAP6: Bank/row address mapping */
	{0x3d402064, 0xc000d},       /* PERF Read CAM */
	{0x3d402138, 0xe},           /* DFITMG1: DFI timing */
	{0x3d403064, 0x30004},       /* PERF Read CAM */
	{0x3d403138, 0x4},           /* DFITMG1: DFI timing */
};

static struct dram_cfg_param ddr_ddrphy_cfg_512mb_patch[] = {
	{0x200c5, 0x19},   /* Master DLL override value */
	{0x20008, 0x384},  /* Calibration segment */
};

static struct dram_cfg_param ddr_phy_pie_512mb_patch[] = {
	{0x2000b, 0x3f5},  /* PIE timing parameter 1 */
	{0x2000c, 0xe1},   /* PIE timing parameter 2 */
	{0x2000d, 0x8ca},  /* PIE timing parameter 3 */
};

static struct dram_cfg_param ddr_fsp_common_512mb_patch[] = {
	{0x54003, 0xe10},    /* DFI frequency: 3600MHz (vs 4000MHz) */
	{0x54012, 0x110},    /* PHYCALMode: 512MB CS configuration */
	{0x54019, 0x36e4},   /* MR1/MR2 values for 3600MHz */
	{0x5401f, 0x36e4},   /* MR1/MR2 values for 3600MHz */
	{0x5402c, 0x1},      /* DRAMFreq: Adjusted for 512MB */
	{0x54032, 0xe400},   /* Message block parameters */
	{0x54033, 0x3336},   /* Message block parameters */
	{0x54038, 0xe400},   /* Message block parameters */
	{0x54039, 0x3336},   /* Message block parameters */
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

/*
 * Apply DDR configuration patches for different memory sizes
 * Base configuration is 4GB @ 4000MHz
 */
void summitsom_imx8mp_ddr_patch(void)
{
	u32 ctrl = 0;

	fuse_read(MEMORY_FUSE_BANK, MEMORY_FUSE_WORD, &ctrl);

	switch (ctrl & 0xff) {
	case DDR_PATCH_2GB:
	case DDR_PATCH_2GB_R2:
		patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
			  ddr_ddrc_cfg_2gb_patch,
			  ARRAY_SIZE(ddr_ddrc_cfg_2gb_patch));
		patch_all_fsp(&dram_timing, ddr_fsp_common_2gb_patch,
			      ARRAY_SIZE(ddr_fsp_common_2gb_patch));
		break;

	case DDR_PATCH_1GB:
		patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
			  ddr_ddrc_cfg_1gb_patch,
			  ARRAY_SIZE(ddr_ddrc_cfg_1gb_patch));
		patch_all_fsp(&dram_timing, ddr_fsp_common_1gb_patch,
			      ARRAY_SIZE(ddr_fsp_common_1gb_patch));
		break;

	case DDR_PATCH_512MB:
		patch_ddr(dram_timing.ddrc_cfg, dram_timing.ddrc_cfg_num,
			  ddr_ddrc_cfg_512mb_patch,
			  ARRAY_SIZE(ddr_ddrc_cfg_512mb_patch));
		patch_ddr(dram_timing.ddrphy_cfg, dram_timing.ddrphy_cfg_num,
			  ddr_ddrphy_cfg_512mb_patch,
			  ARRAY_SIZE(ddr_ddrphy_cfg_512mb_patch));
		patch_ddr(dram_timing.ddrphy_pie, dram_timing.ddrphy_pie_num,
			  ddr_phy_pie_512mb_patch,
			  ARRAY_SIZE(ddr_phy_pie_512mb_patch));
		patch_all_fsp(&dram_timing, ddr_fsp_common_512mb_patch,
			      ARRAY_SIZE(ddr_fsp_common_512mb_patch));
		break;

	case DDR_PATCH_4GB:
		/* No patches needed - 4GB is the base configuration */
		break;

	default:
		/* Use 4GB as default */
		ctrl = DDR_PATCH_4GB;
		break;
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
	case DDR_PATCH_1GB:
		*size = SZ_1G;
		break;

	case DDR_PATCH_2GB:
	case DDR_PATCH_2GB_R2:
		*size = SZ_2G;
		break;

	case DDR_PATCH_4GB:
		*size = SZ_4G;
		break;

	case DDR_PATCH_512MB:
		*size = SZ_512M;
		break;

	default:
		*size = SZ_4G;
		break;
	}

	return 0;
}

#endif /* CONFIG_XPL_BUILD */
