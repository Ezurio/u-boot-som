// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Ezurio
 */

#include <init.h>
#include <asm/arch/clock.h>
#include <env.h>

#include "../common/common.h" 

int board_early_init_f(void)
{
	/* UART1: A55, UART2: M33, UART3: M7 */
	init_uart_clk(0);

	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Nitrogen");
	env_set("board_rev", "iMX95");
#endif

	set_bootside();

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	*size = PHYS_SDRAM_SIZE + PHYS_SDRAM_2_SIZE;

	return 0;
}
