// SPDX-License-Identifier: GPL-2.0+
/*
 * Automatic HAB4 secure-boot fuse programming on first boot.
 *
 * Copyright 2026 Ezurio LLC
 */

#include <command.h>
#include <errno.h>
#include <fuse.h>
#include <linux/types.h>

#include <asm/mach-imx/hab.h>

#include "common.h"

#define HAB4_SEC_CONFIG_BANK	1
#define HAB4_SEC_CONFIG_WORD	3
#define HAB4_SEC_CONFIG_MASK	0x02000000

/* i.MX8M SRKH occupies words 0-3 in banks 6 and 7. */
#define HAB4_SRKH_FIRST_BANK	6
#define HAB4_SRKH_LAST_BANK	7
#define HAB4_SRKH_FIRST_WORD	0
#define HAB4_SRKH_LAST_WORD	3

static int hab4_hab_events_clear(void)
{
	enum hab_config config;
	enum hab_state state;

	/*
	 * An open part still runs the ROM and SPL HAB authentication
	 * routines and logs an event for any image that fails to verify
	 * against the programmed SRK hash; it just carries on booting
	 * afterwards.  A clean event log is therefore the check NXP
	 * documents before closing a device.
	 *
	 * The reported state cannot be used as an additional gate: an open
	 * part stays at HAB_STATE_NONSECURE even when every image in the
	 * boot chain verified, and only moves to HAB_STATE_TRUSTED once the
	 * secure-boot fuse is programmed.
	 */
	if (hab_rvt_report_status(&config, &state) != HAB_SUCCESS) {
		printf("HAB4: HAB events present (config 0x%02x, state 0x%02x)\n",
		       config, state);
		return 0;
	}

	return 1;
}

static int hab4_srkh_is_programmed(void)
{
	u32 value;
	bool any_programmed = false;
	bool all_programmed = true;
	int bank;

	for (bank = HAB4_SRKH_FIRST_BANK; bank <= HAB4_SRKH_LAST_BANK;
	     bank++) {
		int word;

		for (word = HAB4_SRKH_FIRST_WORD; word <= HAB4_SRKH_LAST_WORD;
		     word++) {
			/*
			 * Sense the physical OTP rather than reading the
			 * shadow register, which can be overridden at runtime
			 * and would make blank SRKH fuses look provisioned.
			 */
			if (fuse_sense(bank, word, &value)) {
				printf("HAB4: unable to sense SRKH fuse %u:%u\n",
				       bank, word);
				return -EIO;
			}

			if (value)
				any_programmed = true;
			else
				all_programmed = false;
		}
	}

	if (all_programmed)
		return 1;
	if (any_programmed)
		return -EINVAL;

	return 0;
}

void summit_hab_lifecycle_init(void)
{
	u32 sec_config;
	int ret;

	ret = fuse_read(HAB4_SEC_CONFIG_BANK, HAB4_SEC_CONFIG_WORD,
			&sec_config);
	if (ret) {
		printf("HAB4: unable to read secure-boot fuse\n");
		return;
	}

	if (sec_config & HAB4_SEC_CONFIG_MASK) {
		return;
	}

	if (!hab4_hab_events_clear()) {
		printf("HAB4: HAB status is not clean; not closing device\n");
		return;
	}

	/*
	 * A clean event log does not imply the SRK hash is usable, so this
	 * check is not redundant.  With a blank SRKH there is nothing for the
	 * ROM to compare the SRK table against, so it logs no events and an
	 * unprovisioned part looks identical to a correctly provisioned one.
	 * Closing in that state burns in a hash no image can ever match.
	 */
	ret = hab4_srkh_is_programmed();
	if (ret == -EINVAL) {
		printf("HAB4: SRKH fuses are only partially programmed; not closing device\n");
		return;
	}
	if (ret < 0)
		return;
	if (!ret) {
		printf("HAB4: SRKH fuses are blank; not closing device\n");
		return;
	}

	printf("HAB4: programming OEM closed fuse\n");
	ret = fuse_prog(HAB4_SEC_CONFIG_BANK, HAB4_SEC_CONFIG_WORD,
			HAB4_SEC_CONFIG_MASK);
	if (ret) {
		printf("HAB4: failed to program OEM closed fuse (%d)\n", ret);
		return;
	}

	printf("HAB4: OEM closed fuse programmed, resetting\n");
	do_reset(NULL, 0, 0, NULL);
}
