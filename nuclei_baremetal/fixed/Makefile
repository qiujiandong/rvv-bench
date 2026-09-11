VLM_BASE ?=
FPGA_TEST ?= 0

TARGET = fixed_test

CORE ?= ux1000fd

ARCH_EXT ?= v_zba_zbb_zbc_zbs

ifeq ($(FPGA_TEST),0)
# Reduce codesize by default
CODESIZE ?= 0
# Run with RTL simulation by default
SIMULATION ?= 0
else
CODESIZE ?= 1
SIMULATION ?= 1
endif

SRCDIRS = . ../common

INCDIRS = . ../common

STDCLIB = newlib_small

COMMON_FLAGS := -g -O2 -gdwarf-4 -gstrict-dwarf -funroll-loops

LINKER_SCRIPT = ../common/link.ld

ifneq ($(VLM_BASE),)
COMMON_FLAGS += -DVLM_BASE=$(VLM_BASE)
endif

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
