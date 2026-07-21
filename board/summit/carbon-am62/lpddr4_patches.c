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

const static struct ddrss_patch lpddr4_data[] = {
	{
		.id = 1,
		.ctl_patch = lpddr4_ctl_1gb,
		.pi_patch = lpddr4_pi_1gb,
	},
	{ 0 },
};

const struct ddrss_patch* get_lpddr_patch_data(void)
{
	return lpddr4_data;
}
