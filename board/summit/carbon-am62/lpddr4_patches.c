/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2026 Ezurio
 */

#include <init.h>

#include "../common/k3-common.h"

/* Micron, 2GB - default */

/* Micron, 1 GB */
const static struct ddr_patch_record lpddr4_ctl_1gb[] = {
	{ 317, 0x00000101 },
	{ 318, 0x1FFF0000 },
	{ U32_MAX, 0 },
};

const static struct ddr_patch_record lpddr4_pi_1gb[] = {
	{ 77, 0x08010100 },
	{ U32_MAX, 0 },
};

const static struct ddr_patch_record lpddr4_ctl_2gb_2ranks[] = {
	{ 317, 0x00000101 },
	{ 318, 0x1FFF0000 },
	{ 320, 0x3FFF2000 },
	{ 321, 0x000FFF00 },
	{ 322, 0x0B000001 },
	{ 327, 0x00000C03 },
	{ U32_MAX, 0 },
};

const static struct ddr_patch_record lpddr4_pi_2gb_2ranks[] = {
	{ 13, 0x00030001 },
	{ 26, 0x00030000 },
	{ 45, 0x00030300 },
	{ 56, 0x00001703 },
	{ 66, 0x01010300 },
	{ 77, 0x08010100 },
	{ U32_MAX, 0 },
};

const static struct ddr_patch_record lpddr4_phy_2gb_2ranks[] = {
	{ 1294, 0x00004003 },
	{ U32_MAX, 0 },
};

const static struct ddrss_patch lpddr4_data[] = {
	{
		.id = 1,
		.ctl_patch = lpddr4_ctl_1gb,
		.pi_patch = lpddr4_pi_1gb,
	},
	{
		.id = 12,
		.pll_fhs_cnt = 5,
		.ctl_patch = lpddr4_ctl_2gb_2ranks,
		.pi_patch = lpddr4_pi_2gb_2ranks,
		.phy_patch = lpddr4_phy_2gb_2ranks,
	},
	{ 0 },
};

const struct ddrss_patch* get_lpddr_patch_data(void)
{
	return lpddr4_data;
}
