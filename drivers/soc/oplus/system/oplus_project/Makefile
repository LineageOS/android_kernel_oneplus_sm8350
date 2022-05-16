# SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2018-2020 Oplus. All rights reserved.

ifeq ($(CONFIG_KASAN),y)
subdir-ccflags-y += -Wno-format
endif

ifeq ($(CONFIG_OPLUS_FEATURE_OPROJECT), y)
obj-$(CONFIG_OPLUS_FEATURE_OPROJECT) 	+= qcom/oplus_project.o
else
obj-$(CONFIG_OPLUS_FEATURE_OPROJECT)	+= gki/oplus_project.o
endif
