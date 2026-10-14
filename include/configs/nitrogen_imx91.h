/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 Ezurio LLC
 */

#ifndef __NITROGEN_IMX91_H
#define __NITROGEN_IMX91_H

#define CFG_SYS_UBOOT_BASE	\
	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR        0x80000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE           0x80000000
#define PHYS_SDRAM                   0x80000000
#define PHYS_SDRAM_SIZE			     0x80000000 /* 2GB DDR */

#define WDOG_BASE_ADDR               WDG3_BASE_ADDR

#endif
