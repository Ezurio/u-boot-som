// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for i.MX8MP Summit SOM
 *
 * Copyright (C) 2025 Ezurio
 *
 */

#include <init.h>
#include <dm.h>
#include <i2c.h>
#include <fuse.h>
#include <linux/sizes.h>
#include <asm/io.h>

 #include "../common/common.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, struct bd_info *bd)
{
	if (gd->ram_size <= SZ_1G) {
		int offs = fdt_path_offset(blob, "/mix_gpu_ml");
		if (offs >= 0)
			fdt_setprop_string(blob, offs, "status", "disabled");
		else
			printf("Node /mix_gpu_ml not found.\n");
	}

	return 0;
}
#endif

int board_phys_sdram_size(phys_size_t *memsize)
{
	u32 gp1 = 0;

	fuse_read(14, 0, &gp1);

	switch (gp1 & 0xff) {
	case 1:
		*memsize = SZ_1G;
		break;
	case 2:
	case 5:
		*memsize = SZ_2G;
		break;
	case 3:
		*memsize = SZ_4G;
		break;
	case 4:
		*memsize = SZ_512M;	
		break;
	default:
		if ((readl(0x3d400000) & 0xf000000) == 0x3000000)
			*memsize = SZ_4G;
		else
			*memsize = get_ram_size((void *)PHYS_SDRAM, SZ_2G);
		break;
	}

	return 0;
}

static int __maybe_unused setup_charger(uint8_t i2c_bus, uint8_t addr)
{
	struct udevice *bus;
	struct udevice *i2c_dev = NULL;
	int ret;
	uint8_t valb;

	ret = uclass_get_device_by_seq(UCLASS_I2C, i2c_bus, &bus);
	if (ret) {
		printf("%s: Can't find bus\n", __func__);
		return -EINVAL;
	}

	ret = dm_i2c_probe(bus, addr, 0, &i2c_dev);
	if (ret) {
		printf("%s: Can't find device id=0x%x\n",
			__func__, addr);
		return -ENODEV;
	}

	ret = dm_i2c_read(i2c_dev, 0x48, &valb, 1);
	if (ret) {
		printf("%s dm_i2c_read failed, err %d\n", __func__, ret);
		return -EIO;
	}

	if (valb != 0)
		return 0;

	valb = 5;
	ret = dm_i2c_write(i2c_dev, 0x10, &valb, 1);
	if (ret) {
		printf("%s dm_i2c_write failed, err %d\n", __func__, ret);
		return -EIO;
	}

	valb = 0;
	ret = dm_i2c_write(i2c_dev, 0x11, &valb, 1);
	if (ret) {
		printf("%s dm_i2c_write failed, err %d\n", __func__, ret);
		return -EIO;
	}

	return 0;
}

int board_init(void)
{
	if (IS_ENABLED(CONFIG_SUMMIT_SOM_DVK))
		setup_charger(1, 0x6b);

	return 0;
}

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	env_set("board_name", "Summit SOM");
	env_set("board_rev", "iMX8MP");
#endif

	set_bootside();

	return 0;
}
#endif
