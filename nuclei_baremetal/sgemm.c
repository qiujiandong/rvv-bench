#include <riscv_vector.h>
#include <string.h>

#include "bench.h"

extern void _sgemm_scalar(float *restrict c, float const *restrict a,
                          float const *restrict b, size_t M, size_t K,
                          size_t N);

extern void _sgemm_rvv(float *restrict c, float const *restrict a,
                       float const *restrict b, size_t M, size_t K, size_t N);

extern void _sgemm_vme(float *restrict c, float const *restrict a,
                       float const *restrict b, size_t M, size_t K, size_t N);

void sgemm_scalar(float *restrict c, float const *restrict a,
                  float const *restrict b, size_t n) {
  const size_t M = n;
  const size_t N = n;
  const size_t K = n;
  _sgemm_scalar(c, a, b, M, K, N);
}

void sgemm_rvv(float *restrict c, float const *restrict a,
               float const *restrict b, size_t n) {
  const size_t M = n;
  const size_t N = n;
  const size_t K = n;
  _sgemm_rvv(c, a, b, M, K, N);
}

void sgemm_vme(float *restrict c, float const *restrict a,
               float const *restrict b, size_t n) {
  const size_t M = n;
  const size_t N = n;
  const size_t K = n;
  _sgemm_vme(c, a, b, M, K, N);
}

typedef void Func(float *restrict c, float const *restrict a,
                  float const *restrict b, size_t n);

#define IMPLS(f) f(scalar) f(rvv) f(vme)
#define DECLARE(f) extern Func sgemm_##f;

#define IMPLS(f) f(scalar) f(rvv) f(vme)
IMPLS(DECLARE)

#define EXTRACT(f) {#f, &sgemm_##f, 0},
Impl impls[] = {IMPLS(EXTRACT)};

float *pc, *pa, *pb;
BENCH_BEG(base) {
  memset(pc, 0, MAX_MAT * MAX_MAT * sizeof *pc);
  TIME f(pc, pa, pb, n);
}
BENCH_END
Bench benches[] = {BENCH(impls, MAX_MAT, "sgemm", bench_base)};

void init(void) {
  pc = (float *)mem;
  pa = (float *)(mem + MAX_MEM / 4);
  pb = (float *)(mem + MAX_MEM / 2);
  for (int i = 0; i < MAX_MAT * MAX_MAT; ++i) {
    pa[i] = bench_urandf();
    pb[i] = bench_urandf();
  }
  benches->impls[2].skipCheck = 1;
}

ux checksum(size_t n) {
  ux sum = 0;
  for (size_t i = 0; i < n * n; ++i) {
    uint32_t bits;
    memcpy(&bits, pc + i, sizeof bits);
    sum = uhash(sum) + bits;
  }
  return sum;
}

BENCH_MAIN(benches)
