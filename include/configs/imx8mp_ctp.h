/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 NXP
 * Copyright 2024 Ezurio
 */

#ifndef __IMX8MP_CTP_H
#define __IMX8MP_CTP_H

#include <linux/sizes.h>
#include <asm/arch/imx-regs.h>

#define CFG_SYS_UBOOT_BASE	(QSPI0_AMBA_BASE + CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_SECTOR * 512)

#define CFG_MXC_UART_BASE		UART2_BASE_ADDR

/* Link Definitions */

#define CFG_SYS_INIT_RAM_ADDR	0x40000000
#define CFG_SYS_INIT_RAM_SIZE	0x80000

/* Totally 4GB DDR */
#define CFG_SYS_SDRAM_BASE		0x40000000

#define PHYS_SDRAM              0x40000000
#define PHYS_SDRAM_SIZE         SZ_2G
#define PHYS_SDRAM_2            0x100000000
#define PHYS_SDRAM_2_SIZE       SZ_2G

#endif
