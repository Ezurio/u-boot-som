#
# Copyright 2026 Ezurio LLC
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += nitrogen_imx91.o ../common/imx-common.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o

ifdef CONFIG_NITROGEN_IMX91_OSM_S
obj-y += lpddr4_osm_s_1gb_timing.o lpddr4_osm_s_2gb_timing.o
else
obj-y += lpddr4_smarc_1gb_timing.o lpddr4_smarc_2gb_timing.o
endif
endif
