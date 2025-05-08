// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Carbon AM62 OSM module
 *
 * Copyright (C) 2024 Ezurio
 *
 */

#include <env.h>
#include <spl.h>
#include <init.h>
#include <k3-ddrss.h>
#include <fdt_support.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/k3-ddr.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Carbon AM62x");
	env_set("board_rev", "AM62x");
#endif

	set_bootside();

	return 0;
}
#endif

#if defined(CONFIG_XPL_BUILD)
void spl_board_init(void)
{
	u32 val;

	/* We have 32k crystal, so lets enable it */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Add any TRIM needed for the crystal here.. */
	/* Make sure to mux up to take the SoC 32k from the crystal */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
}

void spl_perform_fixups(struct spl_image_info *spl_image)
{
#if !IS_ENABLED(CONFIG_K3_DDRSS)
	fixup_memory_node(spl_image);
#endif
}
#endif

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(blob, "/soc/ethernet@4a100000");
	if (nodeoffset < 0) {
		printf("Error: Could not find ethernet node\n");
		return -1;
	}

	fdt_setprop_string(blob, nodeoffset, "model", "Ezurio Carbon AM62x");

	return 0;
}
#if defined(CONFIG_OF_LIBFDT_OVERLAY)
int ft_board_setup_overlay(void *blob, struct bd_info *bd)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(blob, "/soc/ethernet@4a100000");
	if (nodeoffset < 0) {
		printf("Error: Could not find ethernet node\n");
		return -1;
	}

	fdt_setprop_string(blob, nodeoffset, "model", "Ezurio Carbon AM62x");

	return 0;
}
#endif
int board_fit_config_name_match(const char *name)
{
	if (strcmp(name, "Ezurio-Carbon-AM62x") == 0)
		return 0;

	return -1;
}
