// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Ezurio LLC
 */

#include <init.h>
#include <env.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <i2c.h>
#include <dm/uclass.h>

#include "../common/common.h"

int board_late_init(void)
{
	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG)) {
		env_set("board_name", "Nitrogen");
		env_set("board_rev", "iMX8MM");
	}

	set_bootside();

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_adjust_for_smarc_rev2(void *blob)
{
	struct udevice *bus, *dev;
	const char *name;
	int node, ret;

	/* Try to probe rv3028 on I2C to see if it's physically present */
	ret = uclass_get_device_by_seq(UCLASS_I2C, 0, &bus);
	if (ret) {
		debug("%s: Can't find I2C bus 0\n", __func__);
		return 0;
	}

	/* Probe rv3028 at address 0x52 */
	ret = dm_i2c_probe(bus, 0x52, 0, &dev);
	if (ret == 0) {
		/* rv3028 is present on I2C, no action needed */
		debug("%s: rv3028 detected on I2C\n", __func__);
		return 0;
	}

	/* Find rv3028 node with compatible string and name rv3028@52 */
	fdt_for_each_node_by_compatible(node, blob, node,
					"microcrystal,rv3028") {
		/* Check if this node has the name rv3028@52 */
		name = fdt_get_name(blob, node, NULL);
		if (name && strcmp(name, "rv3028@52") == 0) {
			/* Found the correct node */
			break;
		}
	}

	if (node < 0) {
		/* rv3028@52 not found, no action needed */
		return 0;
	}

	/* Disable rv3028 node */
	fdt_status_disabled(blob, node);

	/* Find the reg_slow_clock node */
	node = fdt_path_offset(blob, "/regulator-slow-clock");
	if (node >= 0) {
		/* Set compatible property to regulator-fixed */
		fdt_setprop_string(blob, node, "compatible", "regulator-fixed");
		fdt_setprop_string(blob, node, "pinctrl-names", "default");
	} else {
		debug("Warning: regulator-slow-clock node not found\n");
	}

	/* Get aliases node once for all alias operations */
	int aliases_node = fdt_path_offset(blob, "/aliases");
	if (aliases_node < 0) {
		debug("Warning: /aliases node not found\n");
		return 0;
	}

	/* Get phandle from rtc1 alias and assign it to rtc0 */
	const fdt32_t *rtc1_prop = fdt_getprop(blob, aliases_node, "rtc1",
						NULL);
	if (rtc1_prop) {
		u32 rtc1_phandle = fdt32_to_cpu(*rtc1_prop);
		/* Set rtc0 alias to the same phandle as rtc1 */
		fdt_setprop_u32(blob, aliases_node, "rtc0", rtc1_phandle);
	}

	/* Get phandle from rtc2 alias and assign it to rtc1 */
	const fdt32_t *rtc2_prop = fdt_getprop(blob, aliases_node, "rtc2",
						NULL);
	if (rtc2_prop) {
		u32 rtc2_phandle = fdt32_to_cpu(*rtc2_prop);
		/* Set rtc1 alias to the same phandle as rtc2 */
		fdt_setprop_u32(blob, aliases_node, "rtc1", rtc2_phandle);
	}

	/* Delete rtc2 alias */
	fdt_delprop(blob, aliases_node, "rtc2");

	return 0;
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
	int ret;

	/* Check if board compatible is summit,imx8mm-nitrogen-smarc */
	ret = fdt_node_check_compatible(blob, 0,
					 "summit,imx8mm-nitrogen-smarc");
	if (ret == 0)
		return ft_adjust_for_smarc_rev2(blob);

	return 0;
}
#endif
