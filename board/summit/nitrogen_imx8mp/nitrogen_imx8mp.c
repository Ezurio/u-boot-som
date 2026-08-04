// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for Nitrogen i.MX8MP SMARC
 *
 * Copyright (C) 2025 Ezurio
 *
 */

#include <env.h>

#include "../common/common.h"

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Nitrogen");
	env_set("board_rev", "iMX8MP");
#endif

	set_serial_number();
	set_bootside();

	return 0;
}
