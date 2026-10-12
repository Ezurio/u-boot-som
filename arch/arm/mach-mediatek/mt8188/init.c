// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 MediaTek Inc.
 * Copyright (C) 2025 BayLibre, SAS
 * Author: Julien Masson <jmasson@baylibre.com>
 *         Chris-QJ Chen <chris-qj.chen@mediatek.com>
 */

#include <asm/global_data.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <dm/uclass.h>
#include <u-boot/crc.h>
#include <linux/kernel.h>
#include <linux/sizes.h>
#include <wdt.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_TARGET_SUMMIT_TUNGSTEN_MT83XX
#define SUMMIT_BOOT_ARGUMENT_ADDR	(CONFIG_MTK_MEM_MAP_DDR_BASE_PHY + 0x100)

struct summit_boot_argument {
	u32 magic_number;
	u64 dram_size;
	u32 crc;	/* CRC32 of dram_size */
};
#endif

int dram_init(void)
{
	int ret;

	ret = fdtdec_setup_mem_size_base();
	if (ret)
		return ret;

#ifdef CONFIG_TARGET_SUMMIT_TUNGSTEN_MT83XX
	const volatile struct summit_boot_argument *boot_argument =
		(const volatile struct summit_boot_argument *)(uintptr_t)
		SUMMIT_BOOT_ARGUMENT_ADDR;

	if (crc32(0, (const void *)&boot_argument->dram_size,
		  sizeof(boot_argument->dram_size)) == boot_argument->crc) {
		gd->ram_base = CONFIG_MTK_MEM_MAP_DDR_BASE_PHY;
		gd->ram_size = boot_argument->dram_size;
		mem_map[0].size = boot_argument->dram_size;
	}
#endif

	return 0;
}

phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	/*
	 * Limit gd->ram_top not exceeding SZ_4G. Because some peripherals like
	 * MMC requires DMA buffer allocated below SZ_4G.
	 */
	return min(gd->ram_top, SZ_4G);
}

void reset_cpu(void)
{
	struct udevice *wdt;

	if (CONFIG_IS_ENABLED(PSCI_RESET)) {
		psci_system_reset();
	} else {
		uclass_first_device(UCLASS_WDT, &wdt);
		if (wdt)
			wdt_expire_now(wdt, 0);
	}
}

int print_cpuinfo(void)
{
	printf("CPU:   MediaTek MT8188\n");
	return 0;
}
