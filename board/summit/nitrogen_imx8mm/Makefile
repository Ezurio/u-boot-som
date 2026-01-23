#
# Copyright 2026 Ezurio LLC
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += nitrogen_imx8mm.o ../common/imx-common.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
obj-$(CONFIG_NITROGEN_IMX8MM_2GB) += lpddr4_2gb_timing.o
obj-$(CONFIG_NITROGEN_IMX8MM_4GB) += lpddr4_4gb_timing.o
endif
