
WARN=-Wall -Wextra -Wno-unused-function -Wno-unused-parameter

# freestanding using any recent clang build
# CC=clang
# CFLAGS=--target=riscv64 -march=rv64gcv_zba_zbb_zbs -O3 ${WARN} -nostdlib -fno-builtin -ffreestanding
#CFLAGS=--target=riscv32 -march=rv32gc_zve32f_zba_zbb_zbs -O3 ${WARN} -nostdlib -fno-builtin -ffreestanding

# full cross compilation toolchain
#CC=riscv64-linux-gnu-gcc
#CFLAGS=-march=rv64gcv -O3 ${WARN}

# native build
#CC=cc
#CFLAGS=-march=rv64gcv -O3 ${WARN}

# basic CFLAGS
CFLAGS := -march=rv64gcv_zba_zbb_zbs -mcmodel=medany -mabi=lp64d -O3 --static ${WARN}

SDK := K230
# SDK := Nuclei

ifeq (${SDK}, K230)
# K230 SDK v1.6
ifndef K230_SDK_ROOT
$(error K230_SDK_ROOT is not set)
endif
TOOL_PREFIX = riscv64-unknown-linux-musl-
LD_SCRIPT := ${K230_SDK_ROOT}/src/big/mpp/userapps/sample/linker_scripts/riscv64/link.lds
LDFLAGS := -L${K230_SDK_ROOT}/src/big/rt-smart/userapps/sdk/rt-thread/lib/risc-v/rv64 \
		   -Wl,--start-group -lrtthread -Wl,--end-group \
		   -Wl,--whole-archive -lrtthread -Wl,--no-whole-archive
CFLAGS += -T${LD_SCRIPT} ${LDFLAGS}
endif

ifeq (${SDK}, Nuclei)
# Nuclei gcc version: riscv64-unknown-linux-gnu-gcc (g598f284ab) 13.1.1 20230713
TOOL_PREFIX = riscv64-unknown-linux-gnu-
endif

# common part
CC := ${TOOL_PREFIX}gcc # make sure the compiler path is added to your $PATH variable
CHECK = $(shell which ${CC} > /dev/null 2>&1 && echo found || echo not found)
ifneq (${CHECK}, found)
$(error Toolchain not found: ${CC})
endif
