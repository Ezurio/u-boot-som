/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration header file for Ezurio Carbon AM62L OSM
 *
 * Copyright (C) 2026 Ezurio
 *
 */

#ifndef __CONFIG_CARBON_AM62L_H
#define __CONFIG_CARBON_AM62L_H

/*
 * DDR information.  If the CONFIG_NR_DRAM_BANKS is not defined,
 * we say (for simplicity) that we have 1 bank, always, even when
 * we have more.  We always start at 0x80000000, and we place the
 * initial stack pointer in our SRAM. Otherwise, we can define
 * CONFIG_NR_DRAM_BANKS before including this file.
 */
#define CFG_SYS_SDRAM_BASE		0x80000000

/* NAND Driver config */
#define CFG_SYS_NAND_BASE            0x51000000

#define CFG_SYS_NAND_ECCPOS		{ 2, 3, 4, 5, 6, 7, 8, 9, \
					 10, 11, 12, 13, 14, 15, 16, 17, \
					 18, 19, 20, 21, 22, 23, 24, 25, \
					 26, 27, 28, 29, 30, 31, 32, 33, \
					 34, 35, 36, 37, 38, 39, 40, 41, \
					 42, 43, 44, 45, 46, 47, 48, 49, \
					 50, 51, 52, 53, 54, 55, 56, 57, }

#define CFG_SYS_NAND_ECCSIZE         512

#define CFG_SYS_NAND_ECCBYTES        14
/*-- end NAND config --*/

#endif /* __CONFIG_CARBON_AM62L_H */
