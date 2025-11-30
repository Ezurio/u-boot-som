// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2025 Ezurio
 */

#include "../common/common.h"    

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "NITROGEN");
	env_set("board_rev", "iMX91");
#endif

	set_bootside();

	return 0;
}
