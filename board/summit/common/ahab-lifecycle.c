// SPDX-License-Identifier: GPL-2.0+
/*
 * Automatic ELE lifecycle advancement on first boot.
 *
 * Copyright 2026 Ezurio
 *
 * The ELE only permits a lifecycle transition when the current boot was
 * authenticated, which rules out performing the transition from an unsigned
 * USB rescue image.  Instead the transition is performed here, from the signed
 * image that has just been authenticated by the ROM.
 *
 * No policy is duplicated here: the ELE itself refuses the transition when the
 * SRK hash fuses are blank or the current boot was not authenticated, so the
 * request is simply issued and the result reported.
 */

#include <command.h>
#include <linux/types.h>

#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/mach-imx/ele_api.h>

#include "common.h"

/*
 * Lifecycle encoding and the FSB lifecycle register offset are SoC specific.
 * Keep these in sync with display_life_cycle() in arch/arm/mach-imx/ele_ahab.c,
 * which is the source of truth.
 */
#if IS_ENABLED(CONFIG_IMX95) || IS_ENABLED(CONFIG_IMX94) || IS_ENABLED(CONFIG_IMX952)
/*
 * These SoCs have an additional "OEM secure world closed" state (0x20) between
 * OEM open and OEM closed.  It is not part of this transition path, so it falls
 * through to the default case and is reported without being advanced.
 */
#define FSB_LC_OFFSET	0x414
#define LC_OEM_OPEN	0x10
#define LC_OEM_CLOSED	0x40
#define LC_OEM_LOCKED	0x80
#else
#define FSB_LC_OFFSET	0x41c
#define LC_OEM_OPEN	0x8
#define LC_OEM_CLOSED	0x20
#define LC_OEM_LOCKED	0x100
#endif

/* Arguments to ELE_FWD_LIFECYCLE_UP_REQ, as used by ahab_close / ahab_lock */
#define ELE_FWD_OEM_CLOSED	8
#define ELE_FWD_OEM_LOCKED	128

static u32 read_lifecycle(void)
{
	return readl(FSB_BASE_ADDR + FSB_LC_OFFSET) & 0x3ff;
}

/*
 * Request a lifecycle transition and reset so that the new state takes effect.
 *
 * The FSB lifecycle register is a shadow of the fuses latched at POR, so it
 * still reports the old state immediately after a successful transition and
 * cannot be used to confirm it here.  do_ahab_close() and do_ahab_lock() in
 * arch/arm/mach-imx/ele_ahab.c likewise trust the return code alone.
 */
static void forward_lifecycle(u16 request, const char *target)
{
	u32 resp = 0;
	int ret;

	printf("AHAB: advancing lifecycle to %s\n", target);

	ret = ele_forward_lifecycle(request, &resp);
	if (ret) {
		/*
		 * The reason is in the ELE event log, which the ahab_status in
		 * bootcmd decodes a moment later.  Typically the SRK hash fuses
		 * are blank or do not match the key the image was signed with,
		 * so this boot was not authenticated.
		 */
		printf("AHAB: ELE refused the transition to %s (ret %d, response 0x%08x)\n",
		       target, ret, resp);
		return;
	}

	printf("AHAB: transition to %s accepted, resetting\n", target);
	do_reset(NULL, 0, 0, NULL);
}

void summit_ahab_lifecycle_init(void)
{
	u32 lc = read_lifecycle();

	switch (lc) {
	case LC_OEM_LOCKED:
		break;

	case LC_OEM_OPEN:
		forward_lifecycle(ELE_FWD_OEM_CLOSED, "OEM closed");
		break;

	case LC_OEM_CLOSED:
		if (IS_ENABLED(CONFIG_SUMMIT_AHAB_AUTO_LOCK))
			forward_lifecycle(ELE_FWD_OEM_LOCKED, "OEM locked");
		break;

	default:
		printf("AHAB: unexpected lifecycle 0x%08x, not advancing\n", lc);
		break;
	}
}
