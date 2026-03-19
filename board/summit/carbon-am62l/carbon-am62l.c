// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Carbon AM62Lx OSM module
 *
 * Copyright (C) 2026 Ezurio LLC
 *
 */

#include <env.h>

#include "../common/common.h"

int board_init(void)
{
	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Carbon AM62Lx");
	env_set("board_rev", "AM62Lx");
#endif

	set_bootside();

	return 0;
}
#endif
