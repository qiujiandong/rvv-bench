#include <string.h>

#include "nmsis_bench.h"
#include "tolerance.h"

#define MAX_MEM (1024 * 1024)
#define TOLERANCE (1e-5f)
#ifndef MAT_SIZE
#define MAT_SIZE (32)
#endif

typedef unsigned long ux;

typedef void Sgemm(float *restrict c, float const *restrict a,
                   float const *restrict b, size_t M, size_t K, size_t N);

extern Sgemm _sgemm_scalar;
extern Sgemm _sgemm_rvv;
extern Sgemm _sgemm_vme;
extern Sgemm _sgemm_vme_asm;

BENCH_DECLARE_VAR();

static float c[MAT_SIZE * MAT_SIZE] __attribute__((section(".vlm_data"))) = {0};
static float reference[MAT_SIZE * MAT_SIZE]
    __attribute__((section(".vlm_data"))) = {0};
static float a[MAT_SIZE * MAT_SIZE] __attribute__((section(".vlm_data"))) = {0};
static float b[MAT_SIZE * MAT_SIZE] __attribute__((section(".vlm_data"))) = {0};

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

static ux measure(Sgemm *sgemm, size_t n) {
  memset(c, 0, n * n * sizeof *c);
  BENCH_START(sgemm);
  sgemm(c, a, b, n, n, n);
  BENCH_SAMPLE(sgemm);
  return BENCH_GET_USECYC();
}

__attribute__((noinline)) void  finish_test() {
  while (1)
    ;
}

int main(void) {
  for (size_t i = 0; i < MAT_SIZE * MAT_SIZE; ++i) {
    a[i] = (float)(i % 97) / 97.0f;
    b[i] = (float)(i % 89) / 89.0f;
  }

  static struct {
    char const *name;
    Sgemm *func;
  } impls[] = {
      // {"scalar", _sgemm_scalar},
      {"rvv", _sgemm_rvv},
      {"vme", _sgemm_vme},
      {"vme asm", _sgemm_vme_asm},
  };

  _sgemm_rvv(reference, a, b, MAT_SIZE, MAT_SIZE, MAT_SIZE);
  printf("measurements:\n");
  int failed = 0;
  for (size_t i = 0; i < sizeof impls / sizeof *impls; ++i) {
    ux cycles = measure(impls[i].func, MAT_SIZE);
    ux sum = checksum(MAT_SIZE);
    float max_error;
    int pass = compare_f32_tolerance(c, reference, MAT_SIZE * MAT_SIZE,
                                     TOLERANCE, &max_error);
    failed |= !pass;
    printf("%s: %lu cycles, checksum=%lu, %s (max_error=%g)\n",
           impls[i].name, cycles, sum, pass ? "PASS" : "FAIL", max_error);
  }
#ifndef CFG_SIMULATION
  finish_test();
#endif
  return failed;
}
