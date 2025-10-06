// SPDX-License-Identifier: LicenseRef-Ezurio-Clause
/*
 * Copyright (C) 2022 Ezurio
 */

#include <config.h>
#include <dm.h>
#include <nvmem.h>
#include <misc.h>

#include "eeprom-common.h"

int nvmem_cell_rw(const char *name, bool write, void *p, size_t size)
{
	struct udevice *dev;
	struct nvmem_cell cell;
	ofnode node;
	int ret;

	node = ofnode_by_compatible(ofnode_null(), "summit,board-info");
	if (!ofnode_valid(node)) {
		printf("No summit,board-info node found\n");
		return -ENODEV;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_MISC, node, &dev);
	if (ret) {
		printf("No NVMEM device found %d\n", ret);
		return ret;
	}

	/* Check if the device has a valid MAC address in nvmem */
	ret = nvmem_cell_get_by_name(dev, name, &cell);
	if (ret) {
		printf("No NVMEM cell found %s %d\n", name, ret);
		return ret;
	}

	if (write)
		return nvmem_cell_write(&cell, p, size);
	else
		return nvmem_cell_read(&cell, p, size);
}

static const struct udevice_id summit_board_info_ids[] = {
	{ .compatible = "summit,board-info" },
	{ }
};

U_BOOT_DRIVER(summit_board_info) = {
	.name	= "summit_board_info",
	.id	= UCLASS_MISC,
	.of_match = summit_board_info_ids,
};
