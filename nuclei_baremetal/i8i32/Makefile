MAT_SIZE ?=
FPGA_TEST ?= 0

TARGET = i8i32gemm

CORE ?= ux1000fd

ARCH_EXT ?= v_zba_zbb_zbc_zbs

ifeq ($(FPGA_TEST),1)
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

AUTOVEC ?= 0

COMMON_FLAGS := -g -O3 -gdwarf-4 -gstrict-dwarf -funroll-loops

LINKER_SCRIPT = ../common/link.ld

ifneq ($(MAT_SIZE),)
COMMON_FLAGS += -DMAT_SIZE=$(MAT_SIZE)
endif

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
