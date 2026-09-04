TARGET = sgemm

CORE ?= ux1000fd

ARCH_EXT ?= v_zba_zbb_zbc_zbs

SRCDIRS = .

INCDIRS = .

STDCLIB = newlib_small

COMMON_FLAGS := -O2 -DCUSTOM_HOST -DREAD_MCYCLE -DNOLIBC_DEFINE_ONLY

include $(NUCLEI_SDK_ROOT)/Build/Makefile.base
