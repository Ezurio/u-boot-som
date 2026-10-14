/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 Ezurio LLC
 */

#ifndef __NITROGEN_IMX95_H
#define __NITROGEN_IMX95_H

#include <asm/arch/imx-regs.h>
#include <linux/sizes.h>

#define CFG_SYS_INIT_RAM_ADDR        0x90000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE           0x90000000
#define PHYS_SDRAM                   0x90000000
/* Totally 8GB */
#define PHYS_SDRAM_SIZE              0x70000000  /* 2GB - 256MB DDR */
#define PHYS_SDRAM_2_SIZE            0x180000000 /* 6GB */

#define WDOG_BASE_ADDR               WDG3_BASE_ADDR

#endif
