/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2024 Ezurio
 */

#ifndef __NITROGEN_IMX8MP_H
#define __NITROGEN_IMX8MP_H
#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define CFG_MXC_UART_BASE		UART2_BASE_ADDR

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x80000

/* Totally 6GB DDR */
#define CFG_SYS_SDRAM_BASE		0x40000000

#define PHYS_SDRAM              0x40000000
#define PHYS_SDRAM_SIZE         (SZ_2G + SZ_1G)
#define PHYS_SDRAM_2            0x100000000
#define PHYS_SDRAM_2_SIZE       (SZ_2G + SZ_1G)

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
	"bootside:sw,rescueside:sw,fips:dw,fips_wifi:dw,version:sw,conf:sw"
#endif

#endif
