FPGA_TEST ?= 0

TARGET = sgemm

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

SRCDIRS = . common

INCDIRS = . common

STDCLIB = newlib_small

AUTOVEC ?= 0

COMMON_FLAGS := -g -O3 -DCUSTOM_HOST -DREAD_MCYCLE -DNOLIBC_DEFINE_ONLY -gdwarf-4 -gstrict-dwarf -funroll-loops

LINKER_SCRIPT = common/link.ld

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
