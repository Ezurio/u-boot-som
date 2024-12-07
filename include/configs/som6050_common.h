/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Configuration settings for the SOM60, WB50n Modules.
 */

#ifndef __SOM6050_CONFIG_H
#define __SOM6050_CONFIG_H

#include <linux/sizes.h>
#include <asm/arch/sama5d3.h>

/* ARM asynchronous clock */
#define CFG_SYS_AT91_SLOW_CLOCK         32768
#define CFG_SYS_AT91_MAIN_CLOCK         12000000 /* from 12 MHz crystal */

/* PCK = 528MHz, MCK = 132MHz */
#define BOARD_PLLA_SETTINGS (\
	AT91_PMC_PLLAR_29 |\
	AT91_PMC_PLLXR_PLLCOUNT(0x3f) |\
	AT91_PMC_PLLXR_MUL(43) |\
	AT91_PMC_PLLXR_DIV(1))

#define BOARD_PRESCALER_PLLA \
	(AT91_PMC_MCKR_MDIV_4 | AT91_PMC_MCKR_CSS_PLLA)

/* serial console */
#define CFG_USART_BASE                  ATMEL_BASE_DBGU
#define CFG_USART_ID                    ATMEL_ID_SYS

/* SDRAM */
#define CFG_SYS_SDRAM_BASE              ATMEL_BASE_DDRCS

#define CFG_SYS_INIT_RAM_ADDR           ATMEL_BASE_SRAM1
#define CFG_SYS_INIT_RAM_SIZE           SZ_32K

/* NAND flash */
#define CFG_SYS_NAND_BASE               ATMEL_BASE_CS3
/* our ALE is AD21 */
#define CFG_SYS_NAND_MASK_ALE           (1 << 21)
/* our CLE is AD22 */
#define CFG_SYS_NAND_MASK_CLE           (1 << 22)

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
	"bootside:sw,rescueside:sw,fips:dw,fips_wifi:dw,version:sw"
#endif

#endif
