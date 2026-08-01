// SPDX-License-Identifier: LicenseRef-Ezurio-Clause
/*
 * Copyright (C) 2018 Ezurio
 */

#include <init.h>
#include <env.h>
#include <net.h>
#include <debug_uart.h>
#include <version.h>
#include <fdt_support.h>
#include <mtd_node.h>
#include <jffs2/load_kernel.h>

#include <asm/arch/gpio.h>
#include <asm/arch/clk.h>
#include <asm/arch/at91_common.h>

DECLARE_GLOBAL_DATA_PTR;

#define FS_MAX_KEY_SIZE     64
#define FS_KEY_WINDOW       (ATMEL_BASE_SRAM1 + 0xe000)

int __section(".data") save_env = 0;

__weak int board_early_init_f(void)
{
	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
#if   CONFIG_DEBUG_UART_BASE == ATMEL_BASE_DBGU   // serial0
	at91_seriald_hw_init();
#elif CONFIG_DEBUG_UART_BASE == ATMEL_BASE_USART0 // serial1
	at91_serial0_hw_init();
#elif CONFIG_DEBUG_UART_BASE == ATMEL_BASE_USART1 // serial2
	at91_serial1_hw_init();
#elif CONFIG_DEBUG_UART_BASE == ATMEL_BASE_USART2 // serial3
	at91_serial2_hw_init();
#elif CONFIG_DEBUG_UART_BASE == ATMEL_BASE_USART3 // serial4
	at91_pio3_set_b_periph(AT91_PIO_PORTE, 19, 0);	/* TXD4 */
	at91_pio3_set_b_periph(AT91_PIO_PORTE, 18, 1);	/* RXD4 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_USART3);
#elif CONFIG_DEBUG_UART_BASE == ATMEL_BASE_UART0  // serial5
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 30, 0);	/* TXD5 */
	at91_pio3_set_a_periph(AT91_PIO_PORTC, 29, 1);	/* RXD5 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_UART0);
#elif CONFIG_DEBUG_UART_BASE == ATMEL_BASE_UART1  // serial6
	at91_pio3_set_b_periph(AT91_PIO_PORTA, 31, 0);	/* TXD6 */
	at91_pio3_set_b_periph(AT91_PIO_PORTA, 30, 1);	/* RXD6 */

	/* Enable clock */
	at91_periph_clk_enable(ATMEL_ID_UART1);
#else
	#error "Unknown debug port specified"
#endif
}
#endif

#ifdef CONFIG_FIT_SIGNATURE
void som60_fs_key_inject(void)
{
	u8	*key = (u8 *)FS_KEY_WINDOW;
	const void *fs_key;
	int fs_key_len;
	int enc_node;

	enc_node = fdt_subnode_offset(gd->fdt_blob, 0, "encryption");
	if (enc_node < 0) {
		debug("No encryption node found\n");
		return;
	}

	fs_key = fdt_getprop(gd->fdt_blob, enc_node, "summit,fs-key", &fs_key_len);
	if (!fs_key) {
		debug("No fs-key property found\n");
		return;
	}

	if (fs_key_len != FS_MAX_KEY_SIZE) {
		debug("Key must be max size\n");
		return;
	}

	memcpy(key, fs_key, fs_key_len);
}
#else /* ! CONFIG_FIT_SIGNATURE */
static void som60_fs_key_inject(void)
{
	u8	*key = (u8 *)FS_KEY_WINDOW;

	/* Zero out key area just in case */
	memset(key, 0, FS_MAX_KEY_SIZE);
}
#endif /* ! CONFIG_FIT_SIGNATURE */

int board_late_init(void)
{
	char *version;
	int need_version;

#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
	char name[32], *p;

	snprintf(name, sizeof(name), "%s%s", get_cpu_name(), CONFIG_LOCALVERSION);

	for (p = name; *p; ++p)
		*p = tolower(*p);

	env_set("lrd_name", name);
#endif

#ifdef CONFIG_SYS_EEPROM_SETUP
	mac_read_from_eeprom();
#endif

	version = env_get("version");
	need_version = !version || strcmp(version, PLAIN_VERSION);
	if (need_version) {
		env_set("version", PLAIN_VERSION);
		save_env = 1;
	}

	if ((gd->flags & GD_FLG_ENV_DEFAULT) || save_env) {
		puts("Saving default environment...\n");
		env_save();
	}

#ifdef CONFIG_USB_ETHER
	usb_ether_init();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CFG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_MTD_RAW_NAND
	at91_periph_clk_enable(ATMEL_ID_SMC);

	/* Disable Flash Write Protect discrete */
	at91_set_pio_output(AT91_PIO_PORTE, 14, 1);
#endif

	return 0;
}

void board_quiesce_devices(void)
{
#ifdef CONFIG_MTD_RAW_NAND
#if !defined(CONFIG_TARGET_WB50N)
	/* Activate Flash Write Protect discrete,
	 * so that flash enter standby if not used in kernel */
	at91_set_pio_output(AT91_PIO_PORTE, 14, 0);
#endif

#ifndef CONFIG_NAND_BOOT
	at91_periph_clk_disable(ATMEL_ID_SMC);
#endif
#endif

	som60_fs_key_inject();
}

int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)CFG_SYS_SDRAM_BASE,
		CFG_SYS_SDRAM_SIZE);

	return 0;
}

#ifdef CONFIG_OF_BOARD_SETUP
static void dts_set_mac(void *blob, const char *device, const char* envmac)
{
	int offset;
	u8 mac[ETH_ALEN];

	if (!eth_env_get_enetaddr(envmac,  mac))
		return;

	offset = fdt_path_offset(blob, device);
	if (offset >= 0)
		fdt_setprop(blob, offset, "local-mac-address", mac, ETH_ALEN);
}

int ft_board_setup(void *blob, struct bd_info *bd)
{
#ifdef CONFIG_FDT_FIXUP_PARTITIONS
	static const struct node_info nodes[] = {
		{ "atmel,sama5d3-nand-node", MTD_DEV_TYPE_NAND, },
	};

	fdt_fixup_mtdparts(blob, nodes, ARRAY_SIZE(nodes));
#endif

#if defined(CONFIG_TARGET_WB50N)
	dts_set_mac(blob, "/ahb/apb/ethernet@f802c000", "ethaddr");
#else
	dts_set_mac(blob, "/ahb/apb/ethernet@f0028000", "ethaddr");
	dts_set_mac(blob, "/ahb/apb/ethernet@f802c000", "eth1addr");
#endif

	return 0;
}
#endif /* CONFIG_OF_BOARD_SETUP */

