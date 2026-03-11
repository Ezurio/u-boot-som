/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2025 Ezurio
 */

#include <init.h>

#include "../common/k3-common.h"

/* Micron, MT53E1G32D2FW-046 IT:B, 4GB - default */

/* Micron, MT53E2G32D4DE-046 AIT:C, 8 GB */
const static struct ddr_patch_record lpddr4_ctl_8gb[] = {
	{ 323, 0x00000101 },
	{ 324, 0x1FFF0000 },
	{ 326, 0x3FFF2000 },
	{ U32_MAX, 0 },
};

const static struct ddr_patch_record lpddr4_pi_8gb[] = {
	{ 80, 0x01010001 },
	{ U32_MAX, 0 },
};

const static struct ddrss_patch lpddr4_data[] = {
	{
		.id = 4, 
		.ctl_patch = lpddr4_ctl_8gb,
		.pi_patch = lpddr4_pi_8gb,
		.phy_patch = NULL,
	},
	{ 0 },
};

const struct ddrss_patch* get_lpddr_patch_data(void)
{
	return lpddr4_data;
}
