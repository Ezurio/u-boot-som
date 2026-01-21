// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Ezurio
 */

#include <init.h>
#include <env.h>

#include "../common/common.h"    

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Nitrogen");
	env_set("board_rev", "iMX93");
#endif

	set_bootside();

	return 0;
}
