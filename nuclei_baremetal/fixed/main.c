#include <string.h>

#include "nmsis_bench.h"

#define MAX_MEM (1024 * 1024)
#define MAT_SIZE (32)
#define VLM_BASE (0x60000000UL)
typedef unsigned long ux;

typedef void Sgemm(float *restrict c, float const *restrict a,
                   float const *restrict b, size_t M, size_t K, size_t N);

extern Sgemm _sgemm_scalar;
extern Sgemm _sgemm_rvv;
extern Sgemm _sgemm_vme;

BENCH_DECLARE_VAR();

static float *const c = (float *)VLM_BASE;
static float *const a = (float *)(VLM_BASE + MAX_MEM / 4);
static float *const b = (float *)(VLM_BASE + MAX_MEM / 2);

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
  sgemm(c, a, b, n, n, n);
  memset(c, 0, n * n * sizeof *c);
  BENCH_START(sgemm);
  sgemm(c, a, b, n, n, n);
  BENCH_SAMPLE(sgemm);
  return BENCH_GET_USECYC();
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
      {"scalar", _sgemm_scalar},
      {"rvv", _sgemm_rvv},
      {"vme", _sgemm_vme},
  };

  printf("measurements:\n");
  for (size_t i = 0; i < sizeof impls / sizeof *impls; ++i) {
    ux cycles = measure(impls[i].func, MAT_SIZE);
    ux sum = checksum(MAT_SIZE);
    printf("%s: %lu cycles, checksum=%lu\n", impls[i].name, cycles, sum);
  }
  return 0;
}
