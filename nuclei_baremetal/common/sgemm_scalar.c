#include <stdlib.h>

void _sgemm_scalar(float *restrict c, float const *restrict a,
                   float const *restrict b, size_t M, size_t K, size_t N) {
  for (size_t i = 0; i < M; ++i)
    for (size_t j = 0; j < N; ++j)
      for (size_t k = 0; k < K; ++k)
        c[i * N + j] += a[i * K + k] * b[k * N + j];
}
