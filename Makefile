
TARGET_PLATFORM ?= PC

ifeq ($(TARGET_PLATFORM),3DS)
include Makefile.3ds
else
include Makefile.pc
endif

