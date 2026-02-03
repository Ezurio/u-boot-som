#
# Copyright 2025 Ezurio
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += nitrogen_imx8mp.o
obj-y += ../common/imx-common.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_NITROGEN_IMX8MP_2GB_R1) += lpddr4_2gb_r1_timing.o
obj-$(CONFIG_NITROGEN_IMX8MP_2GB_R2) += lpddr4_2gb_r2_timing.o
obj-$(CONFIG_NITROGEN_IMX8MP_4GB) += lpddr4_4gb_timing.o
obj-$(CONFIG_NITROGEN_IMX8MP_8GB) += lpddr4_8gb_timing.o
endif
