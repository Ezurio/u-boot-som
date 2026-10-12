// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Ryder Lee <ryder.lee@mediatek.com>
 */

#include <clk.h>
#include <hang.h>
#include <init.h>
#include <spl.h>

__weak int mtk_soc_early_init(void)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret)
		hang();

	/* enable console uart printing */
	preloader_console_init();

	/* soc early initialization */
	ret = mtk_soc_early_init();
	if (ret)
		hang();
}

__weak u32 mtk_board_boot_device(u32 boot_dev)
{
	return boot_dev;
}

u32 spl_boot_device(void)
{
	u32 boot_dev = BOOT_DEVICE_NONE;

	if (IS_ENABLED(CONFIG_MTK_BROM_DOWNLOAD_AGENT_UBOOT_FASTBOOT))
		boot_dev = BOOT_DEVICE_RAM;
	else if (CONFIG_IS_ENABLED(SPI))
		boot_dev = BOOT_DEVICE_SPI;
	else if (CONFIG_IS_ENABLED(MMC))
		boot_dev = BOOT_DEVICE_MMC1;
	else if (CONFIG_IS_ENABLED(NAND_SUPPORT))
		boot_dev = BOOT_DEVICE_NAND;
	else if (CONFIG_IS_ENABLED(NOR_SUPPORT))
		boot_dev = BOOT_DEVICE_NOR;

	return mtk_board_boot_device(boot_dev);
}
