#include <string.h>
#include <stdlib.h>

#include "nmsis_bench.h"

#ifndef MAT_SIZE
#define MAT_SIZE (32)
#endif

typedef unsigned long ux;

typedef void I8I32gemm(int32_t *restrict c, int8_t const *restrict a,
                   int8_t const *restrict b, size_t M, size_t K, size_t N);

extern I8I32gemm _i8i32gemm_scalar;
extern I8I32gemm _i8i32gemm_rvv;

BENCH_DECLARE_VAR();

static int32_t c[MAT_SIZE * MAT_SIZE] __attribute__((section(".vlm_data"))) = {0};
static int8_t a[MAT_SIZE * MAT_SIZE] __attribute__((section(".vlm_data"))) = {0};
static int8_t b[MAT_SIZE * MAT_SIZE] __attribute__((section(".vlm_data"))) = {0};

static ux uhash(ux x) {
  /* splitmix64 finalizer */
  x ^= x >> 30;
  x *= 0xbf58476d1ce4e5b9U;
  x ^= x >> 27;
  x *= 0x94d049bb133111ebU;
  x ^= x >> 31;
  return x;
}

static ux checksum(size_t n) {
  ux sum = 0;
  for (size_t i = 0; i < n * n; ++i) {
    uint32_t bits;
    memcpy(&bits, c + i, sizeof bits);
    sum = uhash(sum) + bits;
  }
  return sum;
}

static ux measure(I8I32gemm *i8i32gemm, size_t n) {
  memset(c, 0, n * n * sizeof *c);
  BENCH_START(i8i32gemm);
  i8i32gemm(c, a, b, n, n, n);
  BENCH_SAMPLE(i8i32gemm);
  return BENCH_GET_USECYC();
}

__attribute__((noinline)) void finish_test() {
  while (1)
    ;
}

int main(void) {
  for (size_t i = 0; i < MAT_SIZE * MAT_SIZE; ++i) {
    a[i] = (int8_t)(rand() % 256 - 128);
    b[i] = (int8_t)(rand() % 256 - 128);
  }

  static struct {
    char const *name;
    I8I32gemm *func;
  } impls[] = {
      {"scalar", _i8i32gemm_scalar},
      {"rvv", _i8i32gemm_rvv},
  };

  printf("measurements:\n");
  for (size_t i = 0; i < sizeof impls / sizeof *impls; ++i) {
    ux cycles = measure(impls[i].func, MAT_SIZE);
    ux sum = checksum(MAT_SIZE);
    printf("%s: %lu cycles, checksum=%lu\n", impls[i].name, cycles, sum);
  }
#ifndef CFG_SIMULATION
  finish_test();
#endif
  return 0;
}
