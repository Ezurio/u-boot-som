/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 Ezurio LLC
 */

#ifndef __NITROGEN_IMX95_H
#define __NITROGEN_IMX95_H

#include <asm/arch/imx-regs.h>

#define CFG_SYS_INIT_RAM_ADDR        0x90000000
#define CFG_SYS_INIT_RAM_SIZE        0x200000

#define CFG_SYS_SDRAM_BASE           0x90000000
#define PHYS_SDRAM                   0x90000000
/* Totally 16GB */
#define PHYS_SDRAM_SIZE              0x70000000  /* 2GB - 256MB DDR */
#define PHYS_SDRAM_2_SIZE            0x180000000 /* 6GB */

#define WDOG_BASE_ADDR               WDG3_BASE_ADDR

#ifdef CONFIG_ENV_WRITEABLE_LIST
#ifdef CONFIG_BOOTCOUNT_ENV
#define CFG_ENV_FLAGS_LIST_BOOTCOUNT \
	"bootcount:dw,bootlimit:dw,upgrade_available:dw,"
#else
#define CFG_ENV_FLAGS_LIST_BOOTCOUNT ""
#endif

#ifdef CONFIG_NET_CMD
#define CFG_ENV_FLAGS_LIST_NET "ethaddr:mw,eth1addr:mw,"
#else
#define CFG_ENV_FLAGS_LIST_NET "ethaddr:sw,eth1addr:sw,"
#endif

#define CFG_ENV_FLAGS_LIST_STATIC \
	CFG_ENV_FLAGS_LIST_BOOTCOUNT CFG_ENV_FLAGS_LIST_NET \
	"bootside:sw,rescueside:sw,fips:dw,fips_wifi:dw,version:sw,conf:sw,RegDomain:sw"
#endif

#endif
