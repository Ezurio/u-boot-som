# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright 2025 Ezurio
#

obj-y	+= carbon-am67.o

ifeq ($(CONFIG_K3_DDRSS),y)
obj-y	+= ../common/k3-common-r5.o lpddr4_patches.o
else
obj-y	+= ../common/k3-common.o
endif
