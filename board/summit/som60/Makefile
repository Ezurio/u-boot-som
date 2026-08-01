#
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += som60.o
obj-$(CONFIG_SYS_EEPROM_SETUP) += som60_eeprom.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o som60_nand.o
obj-$(CONFIG_SPL_SYS_MENU) += spl_menu.o
endif
