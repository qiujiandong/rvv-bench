WARN=-Wall -Wextra -Wno-unused-function -Wno-unused-parameter

# append -DUSE_PERF_EVENT to CFLAGS, if the cycle csr isn't exposed
# try -DUSE_PERF_EVENT_SLOW if the abvoe doesn't work

# freestanding using any recent clang build
CC=clang
CFLAGS=--target=riscv64 -march=rv64gcv_zfh_zvfh_zba_zbb_zbs -O3 ${WARN} -nostdlib -fno-builtin -ffreestanding -DUSE_PERF_EVENT
#CFLAGS=--target=riscv32 -march=rv32gc_zve32f_zba_zbb_zbs -O3 ${WARN} -nostdlib -fno-builtin -ffreestanding


# full cross compilation toolchain
#CC=riscv64-linux-gnu-gcc
#CFLAGS=-march=rv64gcv -O3 ${WARN}

# native build
#CC=cc
#CFLAGS=-march=rv64gcv -O3 ${WARN}


