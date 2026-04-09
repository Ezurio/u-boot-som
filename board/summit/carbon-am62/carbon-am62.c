// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Carbon AM62 OSM module
 *
 * Copyright (C) 2024 Ezurio
 *
 */

#include <init.h>
#include <spl.h>
#include <fdt_support.h>
#include <env.h>
#include <asm/arch/hardware.h>
#include <asm/arch/k3-ddr.h>

#include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
#if IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
	env_set("board_name", "Carbon AM62x");
	env_set("board_rev", "AM62x");
#endif

	set_bootside();

	return 0;
}
#endif

#if defined(CONFIG_XPL_BUILD)
#if defined(CONFIG_SPL_BOARD_INIT)
#if defined(CONFIG_CPU_V7R)
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

	/*
	 * Setup debounce time registers.
	 * arbitrary values. Times are approx
	 */
	/* 1.9ms debounce @ 32k */
	writel(0x1, CTRLMMR_DBOUNCE_CFG(1));
	/* 5ms debounce @ 32k */
	writel(0x5, CTRLMMR_DBOUNCE_CFG(2));
	/* 20ms debounce @ 32k */
	writel(0x14, CTRLMMR_DBOUNCE_CFG(3));
	/* 46ms debounce @ 32k */
	writel(0x18, CTRLMMR_DBOUNCE_CFG(4));
	/* 100ms debounce @ 32k */
	writel(0x1c, CTRLMMR_DBOUNCE_CFG(5));
	/* 156ms debounce @ 32k */
	writel(0x1f, CTRLMMR_DBOUNCE_CFG(6));
}
#else
void spl_board_init(void)
{
	enable_caches();
}
#endif /* CONFIG_CPU_V7R */
#endif /* CONFIG_SPL_BOARD_INIT */

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	fixup_memory_node(spl_image);
}
#endif /* CONFIG_XPL_BUILD */
