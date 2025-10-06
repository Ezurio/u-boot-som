// SPDX-License-Identifier: LicenseRef-Ezurio-Clause
/*
 * Copyright (C) 2022 Ezurio
 */

#include <config.h>
#include <asm/global_data.h>
#include <wdt.h>
#include <net-common.h>
#include <linux/ctype.h>
#include <linux/errno.h>

#include "eeprom-common.h"

#define MINI_MENU_INPUT_SIZE    20

DECLARE_GLOBAL_DATA_PTR;

/* Because console code expects console infrastructure fully initialized, but
 * SPL by default has only uart support, don't utilize console for a simple
 * menu in the SPL.  Ugly, but will keep things cleaner and not require
 * initialization of the full console from the SPL, which is at least
 * non-trivial.  (simply invoking console_init is not appropriate)
 */
static void spl_gets(char* chr, size_t sz)
{
	size_t i = 0;

	for(;;) {
		*chr = getchar();
		if (*chr == '\b' || (*chr == 127)) {
			if (i == 0)
				continue;
			putc('\b');
			chr--;
			i--;
		}
		else {
			putc(*chr);
			if ((*chr == '\r') || (*chr == '\n')) {
				*chr = '\0';
				break;
			}
			chr++;
			i++;
			if (i >= sz) {
				chr--;
				*chr = '\0';
				break;
			}
		}
	}

	puts("\r\n");
}

static void mac_calc_inc(u8 *src, u8 *dst, int inc)
{
	u64 m;
	memcpy(&m, src, ETH_ALEN);
	m = be64_to_cpu(m);
	m += inc << 16;
	m = cpu_to_be64(m);
	memcpy(dst, &m, ETH_ALEN);
}

static void read_show_mac(const char *name, unsigned i)
{
	u8 mac[ETH_ALEN];

	int ret = nvmem_cell_rw(name, false, mac, ETH_ALEN);
	if (ret) {
		printf("EEPROM Read Error %d\n", ret);
		return;
	}

	printf("Eth%u: %02x:%02x:%02x:%02x:%02x:%02x\n", i,
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

/**
 * set_mac_address - stores a MAC address
 *
 * This function takes a pointer to MAC address string
 * (i.e."XX:XX:XX:XX:XX:XX", where "XX" is a two-digit hex number) and
 * stores it as "mac1", storing "mac0" as +1 from it.
 * So "mac1" is written as specified, and "mac0" is +1 from it.
 *
 */
static int set_mac_address(const char *buf)
{
	char *p = (char*)buf;
	unsigned long val;
	int i, ret;
	u8 mac[ETH_ALEN] = { 0,0,0,0,0,0 };

	if (!buf)
		return -ERANGE;

	for (i = 0; *p && i < ETH_ALEN; i++) {
			val = simple_strtoul(p, &p, 16);
			if (val > 0xff)
				break;

			mac[i] = val & 0xff;

			if (*p == ':')
				p++;
			else if (*p)
				break;
	}

	if (i != ETH_ALEN || !is_valid_ethaddr(mac))
		return -ERANGE;

	ret = nvmem_cell_rw("mac-address0", true, mac, ETH_ALEN);
	if (ret)
		return ret;

	mac_calc_inc(mac, mac, 1);

	ret = nvmem_cell_rw("mac-address1", true, mac, ETH_ALEN);
	if (ret)
		return ret;

	return 0;
}

static void set_mac_address_menu(void)
{
	int ret;
	char buf[MINI_MENU_INPUT_SIZE];

	puts("enter mac address\n");
	spl_gets(buf, MINI_MENU_INPUT_SIZE);

	ret = set_mac_address(buf);

	switch (ret) {
	case 0:
		break;

	case -ERANGE:
		printf("Invalid MAC address entered\n");
		break;

	default:
		printf("EEPROM Write Error %d\n", ret);
		break;
	}
}

static void read_id_value(const char *name, const char *id)
{
	int ret;
	u16 val;

	printf("%s: ", name);
	ret = nvmem_cell_rw(id, false, &val, sizeof(val));
	if (ret < 0)
		printf("EEPROM Read Error %d\n", ret);
	else
		printf("0x%x\n", val);
}

static void set_id_value(const char *name, const char *id)
{
	int ret;
	u16 val;
	char buf[MINI_MENU_INPUT_SIZE];

	printf("enter %s in hexadecimal\n", name);
	spl_gets(buf, MINI_MENU_INPUT_SIZE);
	val = simple_strtoul(buf, NULL, 16);
	printf("write %s as 0x%x\n", name, val);

	ret = nvmem_cell_rw(id, true, &val, sizeof(val));
	if (ret)
		printf("EEPROM Write Error %d\n", ret);
}

void spl_display_print(void)
{
	char chr;

#if CONFIG_IS_ENABLED(WATCHDOG) && CONFIG_IS_ENABLED(WDT)
	initr_watchdog();
#endif
	for(;;) {
		puts("\nmini spl menu\n");
		puts("0. read all parameters\n");
		puts("1. mac set\n");
		puts("2. ram type set\n");
		puts("3. radio type set\n");
		puts("a. continue boot\n");

		chr = tolower(getchar());

		switch(chr) {
		case '0':
			read_show_mac("mac-address0", 0);
			read_show_mac("mac-address1", 1);
			read_id_value("RAM TYPE", "ram-type");
			read_id_value("RADIO TYPE", "radio-type");
			break;
		case '1':
			set_mac_address_menu();
			read_show_mac("mac-address0", 0);
			read_show_mac("mac-address1", 1);
			break;
		case '2':
			set_id_value("RAM TYPE", "ram-type");
			read_id_value("RAM TYPE", "ram-type");
			break;
		case '3':
			set_id_value("RADIO TYPE", "radio-type");
			read_id_value("RADIO TYPE", "radio-type");
			break;
		case 'a':
#if CONFIG_IS_ENABLED(WATCHDOG) && CONFIG_IS_ENABLED(WDT)
			wdt_stop_all();
#endif
			return;
		default:
			puts("Unknown option ");
			putc(chr);
			puts("\n");
			break;
		}
	}
}
