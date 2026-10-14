/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for Ezurio Carbon AM62 OSM
 *
 * Copyright (C) 2024 Ezurio
 *
 */

#ifndef __CONFIG_CARBON_AM62_H
#define __CONFIG_CARBON_AM62_H

/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */
#define CFG_SYS_SDRAM_BASE		0x80000000

#endif /* __CONFIG_CARBON_AM62_H */
