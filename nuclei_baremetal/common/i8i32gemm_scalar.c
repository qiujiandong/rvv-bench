#include <stdint.h>
#include <stdlib.h>

void _i8i32gemm_scalar(int32_t *restrict c, int8_t const *restrict a,
                      int8_t const *restrict b, size_t M, size_t K, size_t N) {
  for (size_t i = 0; i < M; ++i)
    for (size_t j = 0; j < N; ++j) {
      int32_t sum = 0;
      for (size_t k = 0; k < K; ++k)
        sum += (int32_t)a[i * K + k] * b[k * N + j];
      c[i * N + j] = sum;
    }
}
