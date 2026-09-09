#include <stdlib.h>

void _sgemm_scalar(float *restrict c, float const *restrict a,
                   float const *restrict b, size_t M, size_t K, size_t N) {
  for (size_t i = 0; i < M; ++i) {
    float const *pa = a + i * K;
    float *pc = c + i * N;
    size_t j = 0;
    for (; j + 3 < N; j += 4) {
      float sum0 = pc[j];
      float sum1 = pc[j + 1];
      float sum2 = pc[j + 2];
      float sum3 = pc[j + 3];
      for (size_t k = 0; k < K; ++k) {
        float av = pa[k];
        float const *pb = b + k * N + j;
        sum0 += av * pb[0];
        sum1 += av * pb[1];
        sum2 += av * pb[2];
        sum3 += av * pb[3];
      }
      pc[j] = sum0;
      pc[j + 1] = sum1;
      pc[j + 2] = sum2;
      pc[j + 3] = sum3;
    }
    for (; j < N; ++j) {
      float sum = pc[j];
      for (size_t k = 0; k < K; ++k)
        sum += pa[k] * b[k * N + j];
      pc[j] = sum;
    }
  }
}
