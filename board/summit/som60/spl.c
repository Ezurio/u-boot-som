// SPDX-License-Identifier: LicenseRef-Ezurio-Clause
/*
 * Copyright (C) 2018 Ezurio
 */

#include <nand.h>

#include <asm/arch/gpio.h>
#include <asm/arch/atmel_mpddrc.h>
#include <asm/arch/clk.h>
#include <asm/arch/at91_sck.h>
#include <asm/arch/at91_common.h>

#include <linux/delay.h>

#include "som60_eeprom.h"

#define LEGACY_BOARD_HW_ID  0x00000000
#define MAX_BOARD_HW_ID     0x00000004

/* RAM IC's used on boards.  Note JSFBAB3YH3BBG_425 and JSFBAB3Y63GBG_425
 * combo packages have the same DDR silicon internally.
 */
typedef enum {
	MT46H16M32LF                             = 0,  // legacy wb50n (5 & 6)
	DDR2_JSFBAB3YH3BBG_425_JSFBAB3Y63GBG_425 = 1,  // som60v2
	DDR2_JSFCBB3YH3BBG_425A                  = 2,  // som60x2v2
	MT29C2G24MAAAAKAMD_5                     = 3,  // som60
	MT29C4G48MAYBBAMR_48                     = 4,  // som60x2
} ram_ic_t;

typedef enum {
	RAM_TYPE_LPDDR1,
	RAM_TYPE_LPDDR2,
} ram_type;

typedef struct {
	const char *name;
	ram_type type;
	unsigned long nand_size;
	unsigned long sdram_size;
	struct atmel_mpddrc_config ddr_config;
} ram_config_t;

static const ram_config_t ram_configs[] = {
#if defined(CONFIG_TARGET_WB50N)
	[MT46H16M32LF] = {
		.name = "W949D2DB",
		.type = RAM_TYPE_LPDDR1,
		.nand_size = SZ_128M,
		.sdram_size = SZ_64M,
		.ddr_config =
			{
			.md = (ATMEL_MPDDRC_MD_DBW_32_BITS |
			       ATMEL_MPDDRC_MD_LPDDR_SDRAM),
			.cr = LPDDR_CR,
			/*
			 * The SDRAM device requires a refresh of all rows at least every 64ms.
			 * ((64ms) / 8192) * 132MHz = 1031 i.e. 0x407
			 */
			.rtr = 0x407,
			.tpr0 = LPDDR_TPR0,
			.tpr1 = LPDDR_TPR1,
			.tpr2 = (4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET),
			.lpr = (ATMEL_MPDDRC_LPR_LPCB_DISABLED       |
				ATMEL_MPDDRC_LPR_CLK_FR              |
				ATMEL_MPDDRC_LPR_PASR_(0)            |
				ATMEL_MPDDRC_LPR_DS_(1)              |
				ATMEL_MPDDRC_LPR_TIMEOUT_0           |
				ATMEL_MPDDRC_LPR_ADPE_FAST           |
				ATMEL_MPDDRC_LPR_UPD_MR_NO_UPDATE)
		},
	},
#else
	[DDR2_JSFBAB3YH3BBG_425_JSFBAB3Y63GBG_425] = {
		.name = "DDR2_JSFBAB3Y[H3B/63G]BG_425",
		.type = RAM_TYPE_LPDDR2,
		.nand_size = SZ_256M,
		.sdram_size = SZ_128M,
		.ddr_config = {
			/* Reference at91bootstrap3/driver/ddramc.c */
			.md = (ATMEL_MPDDRC_MD_DBW_32_BITS |
			       ATMEL_MPDDRC_MD_LPDDR2_SDRAM),

			.cr = LPDDR2_CR,

			/* MPDDRC_LPDDR2_LPR DS: Drive Strength - 3 - DS_48 - 48 ohm typical (reset value is 2, 40 Ohm) */
			.lpddr23_lpr = ATMEL_MPDDRC_LPDDR23_LPR_DS(0x03),
			/* JSFBAB3YH3BBG 90n short calibration (tZQCS) */
			/* at 132 MHz, 12 clocks ~= 90 nsec */
			.tim_cal = ATMEL_MPDDRC_CALR_ZQCS(12),

			.rtr = 0x404,

			/* 0xC852 - Atmel reference count value - see SAMA5D3 29.7.11
			 * Dependent upon expected temperature (T driftrate ) and voltage (V driftrate ) drift rates that the SDRAM is subject to in the application.
			 */
			.cal_mr4 = LPDDR2_CAL_MR4,

			/* tRAS temperature derating += 1.875 ns */
			/* trcd temperature derating += 1.875 ns */
			/* tRAS min 3 tck, 42 ns */
			/* tRAS 44 ns ~= 5.8 clocks */
			.tpr0 = LPDDR2_TPR0,
			.tpr1 = LPDDR2_TPR1,
			.tpr2 = LPDDR2_TPR2,
			.lpr = (ATMEL_MPDDRC_LPR_LPCB_DISABLED       |
				ATMEL_MPDDRC_LPR_CLK_FR              |
				ATMEL_MPDDRC_LPR_PASR_(0)            |
				/* This field is unique to low-power DDR1-SDRAM." */
				ATMEL_MPDDRC_LPR_DS_(1)              |
				ATMEL_MPDDRC_LPR_TIMEOUT_0           |
				ATMEL_MPDDRC_LPR_ADPE_FAST           |
				ATMEL_MPDDRC_LPR_UPD_MR_NO_UPDATE)
		},
	},

	[DDR2_JSFCBB3YH3BBG_425A] = {
		.name = "DDR2_JSFCBB3YH3BBG_425A",
		.type = RAM_TYPE_LPDDR2,
		.nand_size = SZ_512M,
		.sdram_size = SZ_256M,
		.ddr_config = {
			.md = (ATMEL_MPDDRC_MD_DBW_32_BITS |
			       ATMEL_MPDDRC_MD_LPDDR2_SDRAM),
			.cr = LPDDR2_CR_DDR2_JSFCBB3YH3BBG_425A,
			.lpddr23_lpr = ATMEL_MPDDRC_LPDDR23_LPR_DS(0x03),
			.tim_cal = ATMEL_MPDDRC_CALR_ZQCS(12),
			.rtr = 0x202,
			.cal_mr4 = LPDDR2_CAL_MR4,
			.tpr0 = LPDDR2_TPR0,
			.tpr1 = LPDDR2_TPR1,
			.tpr2 = LPDDR2_TPR2,
			.lpr = (ATMEL_MPDDRC_LPR_LPCB_DISABLED       |
				ATMEL_MPDDRC_LPR_CLK_FR              |
				ATMEL_MPDDRC_LPR_PASR_(0)            |
				/* This field is unique to low-power DDR1-SDRAM." */
				ATMEL_MPDDRC_LPR_DS_(1)              |
				ATMEL_MPDDRC_LPR_TIMEOUT_0           |
				ATMEL_MPDDRC_LPR_ADPE_FAST           |
				ATMEL_MPDDRC_LPR_UPD_MR_NO_UPDATE)
		},
	},

	[MT29C2G24MAAAAKAMD_5] = {
		.name = "MT29C2G24MAAAAKAMD_5",
		.type = RAM_TYPE_LPDDR1,
		.nand_size = SZ_256M,
		.sdram_size = SZ_128M,
		.ddr_config =
			{
			.md = (ATMEL_MPDDRC_MD_DBW_32_BITS |
			       ATMEL_MPDDRC_MD_LPDDR_SDRAM),
			.cr = LPDDR_CR,
			/*
			 * The SDRAM device requires a refresh of all rows at least every 64ms.
			 * ((64ms) / 8192) * 132MHz = 1031 i.e. 0x407
			 */
			.rtr = 0x407,
			.tpr0 = LPDDR_TPR0,
			.tpr1 = LPDDR_TPR1,
			.tpr2 = (4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET),
			.lpr = (ATMEL_MPDDRC_LPR_LPCB_DISABLED       |
				ATMEL_MPDDRC_LPR_CLK_FR              |
				ATMEL_MPDDRC_LPR_PASR_(0)            |
				ATMEL_MPDDRC_LPR_DS_(1)              |
				ATMEL_MPDDRC_LPR_TIMEOUT_0           |
				ATMEL_MPDDRC_LPR_ADPE_FAST           |
				ATMEL_MPDDRC_LPR_UPD_MR_NO_UPDATE)
		},
	},

	[MT29C4G48MAYBBAMR_48] = {
		.name = "MT29C4G48MAYBBAMR_48",
		.type = RAM_TYPE_LPDDR1,
		.nand_size = SZ_512M,
		.sdram_size = SZ_256M,
		.ddr_config = {
			.md = (ATMEL_MPDDRC_MD_DBW_32_BITS |
			       ATMEL_MPDDRC_MD_LPDDR_SDRAM),
			.cr = LPDDR_CR_SOM60X2,
			/*
			 * The SDRAM device requires a refresh of all rows at least every 64ms.
			 * ((64ms) / 8192) * 132MHz = 1031 i.e. 0x407
			 */
			.rtr = 0x407,
			.tpr0 = LPDDR_TPR0,
			.tpr1 = LPDDR_TPR1,
			.tpr2 = (4 << ATMEL_MPDDRC_TPR2_TRTP_OFFSET),
			.lpr = (ATMEL_MPDDRC_LPR_LPCB_DISABLED       |
				ATMEL_MPDDRC_LPR_CLK_FR              |
				ATMEL_MPDDRC_LPR_PASR_(0)            |
				ATMEL_MPDDRC_LPR_DS_(1)              |
				ATMEL_MPDDRC_LPR_TIMEOUT_0           |
				ATMEL_MPDDRC_LPR_ADPE_FAST           |
				ATMEL_MPDDRC_LPR_UPD_MR_NO_UPDATE)
		},
	},

#endif
};

int is_micron(void);

static int board_hw_id(void)
{
#if defined(CONFIG_TARGET_WB50N)
	return LEGACY_BOARD_HW_ID;
#elif defined(CONFIG_SPL_SYS_EEPROM_SETUP)
	int hw_id = board_hw_id_nvmem_read();

	if (hw_id > 0 && hw_id <= MAX_BOARD_HW_ID)
		return hw_id;
#endif
	return (is_micron() ? MT29C2G24MAAAAKAMD_5 :
		DDR2_JSFBAB3YH3BBG_425_JSFBAB3Y63GBG_425) +
		(nand_size() == SZ_512M);
}

int board_early_init_f(void)
{
	struct at91_port *at91_port = (struct at91_port *)ATMEL_BASE_PIOE;

	/* pins that could be used as USART3 & USART4 are switched to GPIO here,
	   so allow these USARTs to operate as debug terminal if so configured */
	u32 mask = 0x079fffff ^ (readl(&at91_port->mux.pio3.abcdsr1) & 0x060c0000);

	writel(mask, &at91_port->idr);
	writel(mask, &at91_port->per);

	return 0;
}

void at91_disable_smd_clock(void)
{
	struct at91_pmc *pmc = (struct at91_pmc *)ATMEL_BASE_PMC;

	/*
	 * Set pin DIBP to pull-up and DIBN to pull-down
	 * to save power on VDDIOP0
	 */
	at91_system_clk_enable(AT91_PMC_SMD);
	writel(AT91_PMC_SMDDIV, &pmc->smd);
	at91_periph_clk_enable(ATMEL_ID_SMD);

	writel(0xE, (0x0C + ATMEL_BASE_SMD));

	at91_periph_clk_disable(ATMEL_ID_SMD);
	at91_system_clk_disable(AT91_PMC_SMD);
}

static void at91sama5d3_slowclock_init(void)
{
	/*
	 * On AT91SAMA5D3 CPUs, the slow clock can be based on an
	 * internal imprecise RC oscillator or an external 32 kHz oscillator.
	 * Switch to the latter.
	 */
	static const ulong *reg = (ulong *)ATMEL_BASE_SCKCR;
	unsigned tmp;

	/* Enable the internal 32 kHz RC oscillator for low power by writing a 1 to the RCEN bit. */
	tmp = readl(reg);
	tmp |= AT91SAM9G45_SCKCR_RCEN;
	writel(tmp, reg);

	/* Wait internal 32 kHz RC startup time for clock stabilization (software loop). */
	/* 500 us */
	udelay(500);

	/* Switch from 32768 Hz oscillator to internal RC by writing a 0 to the OSCSEL bit. */
	tmp = readl(reg);
	tmp &= ~AT91SAM9G45_SCKCR_OSCSEL;
	writel(tmp, reg);

	/* Wait 5 slow clock cycles for internal resynchronization. */
	/* 5 slow clock cycles = ~153 us (5 / 32768) */
	udelay(153);

	/* Disable the 32768 Hz oscillator by writing a 0 to the OSC32EN bit. */
	tmp = readl(reg);
	tmp &= ~AT91SAM9G45_SCKCR_OSC32EN;
	writel(tmp, reg);

	/* Wait 5 slow clock cycles for internal resynchronization. */
	/* 5 slow clock cycles = ~153 us (5 / 32768) */
	udelay(153);

	/*
	 * Enable the 32768 Hz oscillator by setting the bit OSC32EN to 1
	 */
	tmp = readl(reg);
	tmp |= AT91SAM9G45_SCKCR_OSC32EN;
	writel(tmp, reg);

	/* Bypass the 32kHz oscillator by using an external clock
	 * Set OSC32BYP=1 and OSC32EN=0 atomically
	 */
	tmp = readl(reg);
	tmp &= ~AT91SAM9G45_SCKCR_OSC32EN;
	tmp |= AT91SAM9G45_SCKCR_OSC32BYP;
	writel(tmp, reg);

	/*
	 * Switching from internal 32kHz RC oscillator to 32768 Hz oscillator
	 * by setting the bit OSCSEL to 1
	 */
	tmp = readl(reg);
	tmp |= AT91SAM9G45_SCKCR_OSCSEL_32;
	writel(tmp, reg);

	/*
	 * Waiting 5 slow clock cycles for internal resynchronization
	 * 5 slow clock cycles = ~153 us (5 / 32768)
	 */
	udelay(153);

	/*
	 * Disable the 32kHz RC oscillator by setting the bit RCEN to 0
	 */
	tmp = readl(reg);
	tmp &= ~AT91SAM9G45_SCKCR_RCEN;
	writel(tmp, reg);
}

void spl_board_init(void)
{
	/* Run after spl_early_init because using timer delay */
	at91sama5d3_slowclock_init();

	/* Disable SMD to resolve power consumption */
	at91_disable_smd_clock();

#if defined(CONFIG_TARGET_WB50N)
	/* Disable WB50n Wi-Fi Radio */
	at91_set_pio_output(AT91_PIO_PORTE, 3, 0);
#endif
	/* Disable WB50n Bluetooth and 60 Radio */
	at91_set_pio_output(AT91_PIO_PORTE, 5, 0);
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

// Reference at91bootstrap3/board/sama5d3x_cmp/sama5dx_cmp.c lpddr2_init()
void mem_init_lpddr2(const struct atmel_mpddrc_config *mpddr_value)
{
	const struct atmel_mpddr *mpddr = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;
	u32 reg;

	/* Reference does not open/close input buffers */

	/* Enable MPDDR clock */
	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	at91_system_clk_enable(AT91_PMC_DDR);

	/*
	 * Initialize the special register for the SAMA5D3X_CMP.
	 * MPDDRC DLL Slave Offset Register: DDR2 configuration
	 */
	reg = ATMEL_MPDDRC_SOR_S0OFF(0x04)
		| ATMEL_MPDDRC_SOR_S1OFF(0x03)
		| ATMEL_MPDDRC_SOR_S2OFF(0x04)
		| ATMEL_MPDDRC_SOR_S3OFF(0x04);
	writel(reg, &mpddr->sor);

	/*
	 * MPDDRC DLL Master Offset Register
	 * write master + clk90 offset
	 */
	reg = ATMEL_MPDDRC_MOR_MOFF(7)
		| ATMEL_MPDDRC_MOR_CLK90OFF(0x1F)
		| ATMEL_MPDDRC_MOR_SELOFF_ENABLED | ATMEL_MPDDRC_MOR_KEY;
	writel(reg, &mpddr->mor);

	/*
	 * MPDDRC I/O Calibration Register
	 * DDR2 RZQ = 48 Ohm
	 * TZQIO = 4
	 */
	reg = readl(&mpddr->io_calibr);
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_RDIV;
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_TZQIO;
	// Deviate from reference, use LPDDR2 48-Ohm value.
	// Reference AT91C_MPDDRC_RDIV_DDR2_RZQ_50, 0x4, translates to 60-Ohm for LP-DDR2
	reg |= ATMEL_MPDDRC_IO_CALIBR_LPDDR2_RZQ_48;
	reg |= ATMEL_MPDDRC_IO_CALIBR_TZQIO_(4);
	writel(reg, &mpddr->io_calibr);

	/* LPDDRAM2 Controller initialize */
	lpddr2_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, mpddr_value);

	/* lpr register is not part of the lpddr2 initialization sequence, but
	 * as with lpddr1, we want to set CLK_FR for power-down mode.
	 */
	writel(mpddr_value->lpr, &mpddr->lpr);
}

void mem_init_lpddr1(const struct atmel_mpddrc_config *mpddr_value)
{
	const struct atmel_mpddr *mpddr = (struct atmel_mpddr *)ATMEL_BASE_MPDDRC;

	u32 reg;

	configure_ddrcfg_input_buffers(true);

	/* Enable MPDDR clock */
	at91_periph_clk_enable(ATMEL_ID_MPDDRC);
	at91_system_clk_enable(AT91_PMC_DDR);

	/* Init the special register for sama5d3x */
	/* MPDDRC DLL Slave Offset Register: DDR2 configuration */
	reg = ATMEL_MPDDRC_SOR_S0OFF_1
		| ATMEL_MPDDRC_SOR_S2OFF_1
		| ATMEL_MPDDRC_SOR_S3OFF_1;
	writel(reg, &mpddr->sor);

	/* MPDDRC DLL Master Offset Register */
	/* write master + clk90 offset */
	reg = ATMEL_MPDDRC_MOR_MOFF_7
		| ATMEL_MPDDRC_MOR_CLK90OFF_31
		| ATMEL_MPDDRC_MOR_SELOFF_ENABLED
		| ATMEL_MPDDRC_MOR_KEY;
	writel(reg, &mpddr->mor);

	/* MPDDRC I/O Calibration Register */
	/* LPDDR1 RZQ = 52 Ohm */
	/* TZQIO = (133 * 10^6) * (20 * 10^-9) + 1 = 3.66 == 4 */
	reg = readl(&mpddr->io_calibr);
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_RDIV;
	reg &= ~ATMEL_MPDDRC_IO_CALIBR_TZQIO;
	reg |= ATMEL_MPDDRC_IO_CALIBR_DDR2_RZQ_52;
	reg |= ATMEL_MPDDRC_IO_CALIBR_TZQIO_(4);
	writel(reg, &mpddr->io_calibr);

	reg = readl(&mpddr->hs);
	reg |= ATMEL_MPDDRC_DDR2_EN_CALIB;
	writel(reg, &mpddr->hs);

	/* LPDDRAM1 Controller initialize */
	lpddr1_init(ATMEL_BASE_MPDDRC, ATMEL_BASE_DDRCS, mpddr_value);

	configure_ddrcfg_input_buffers(false);
}

void at91_mem_init(void)
{
	const ram_config_t* ram_config;

	/* 
	   nand_init provides data used to determine memory size and type 
	   of the module, as such it needs to always execute even when NAND 
	   is not used
	*/
	nand_init();

	ram_config = &ram_configs[board_hw_id()];
	printf("Initializing RAM module %s\n", ram_config->name);
	if (ram_config->type == RAM_TYPE_LPDDR1)
		mem_init_lpddr1(&ram_config->ddr_config);
	else
		mem_init_lpddr2(&ram_config->ddr_config);
}

void at91_pmc_init(void)
{
	at91_plla_init(BOARD_PLLA_SETTINGS);

	at91_pllicpr_init(AT91_PMC_IPLL_PLLA(0x3));

	at91_mck_init(BOARD_PRESCALER_PLLA);
}
