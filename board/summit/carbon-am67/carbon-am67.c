// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Carbon AM67 OSM module
 *
 * Copyright (C) 2025 Ezurio
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

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
#if IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
	env_set("board_name", "Carbon AM67x");
	env_set("board_rev", "AM67x");
#endif

	set_bootside();

	return 0;
}
#endif

#if defined(CONFIG_XPL_BUILD)
#if defined(CONFIG_SPL_BOARD_INIT)

#define WKUP_CTRL_DEVICE_CLKOUT_CTRL		(WKUP_CTRL_MMR0_BASE + 0x8020)
#define WKUP_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL	(0x1)

void mmr_unlock(uintptr_t base, u32 partition);

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

	/* Unlock WKUP MMRs, so we can write there */
	mmr_unlock(WKUP_CTRL_MMR0_BASE, 2);

	/* Set WKUP CLKOUT0_SEL to LFOSC0 */
	writel(WKUP_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
		WKUP_CTRL_DEVICE_CLKOUT_CTRL);

	/* Verify CLKOUT0 is set to LFOSC */
	if (readl(WKUP_CTRL_DEVICE_CLKOUT_CTRL) !=
		WKUP_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL)
		printf("Failed to set WKUP_CLKOUT0 to LFOSC\n");

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
#endif /* CONFIG_SPL_BOARD_INIT */

void spl_perform_fixups(struct spl_image_info *spl_image)
{
	fixup_memory_node(spl_image);
}
#endif /* CONFIG_XPL_BUILD */
