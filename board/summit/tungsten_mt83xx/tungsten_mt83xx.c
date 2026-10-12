// SPDX-License-Identifier: GPL-2.0
/* Summit Tungsten MT83XX board support. */

#include <dm.h>
#include <env.h>
#include <cpu_func.h>
#include <fastboot.h>
#include <init.h>
#include <spl.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/sizes.h>

#include "../common/common.h"

#if defined(CONFIG_SUMMIT_TUNGSTEN_MT83XX_UBOOT_RAM_DA) && \
	defined(CONFIG_XPL_BUILD)
DECLARE_GLOBAL_DATA_PTR;

u32 mtk_board_boot_device(u32 boot_dev)
{
	return BOOT_DEVICE_RAM;
}

void spl_board_init(void)
{
	/* Same flow as U-Boot proper: reserve the TLB at the top of RAM, then enable caches. */
	dram_init();
	gd->ram_top = gd->ram_base + gd->ram_size;
	gd->relocaddr = board_get_usable_ram_top(0);
	arm_reserve_mmu();
	enable_caches();

	/* Download straight to the FIT load address that SPL boots from. */
	if (fastboot_usb_run(0, (void *)CONFIG_SPL_LOAD_FIT_ADDRESS, SZ_4M))
		pr_err("Fastboot failed\n");
}
#endif

int board_init(void)
{
	struct udevice *dev;
	int ret;

	if (CONFIG_IS_ENABLED(USB_GADGET)) {
		ret = uclass_get_device(UCLASS_USB_GADGET_GENERIC, 0, &dev);
		if (ret) {
			pr_err("%s: Cannot find USB device\n", __func__);
			return ret;
		}
	}

	/* Enable the SCP SRAM used by the Genio platform. */
	for (u32 val = 0xffffffff; val != 0; ) {
		val >>= 1;
		writel(val, 0x1072102c);
	}

	return 0;
}

int board_late_init(void)
{
#if defined(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)
	env_set("board_name", "Tungsten MT83XX");
	env_set("board_rev",
		ofnode_device_is_compatible(ofnode_root(), "mediatek,mt8370") ?
		"MT8370" : "MT8390");
#endif

	set_bootside();

	return 0;
}