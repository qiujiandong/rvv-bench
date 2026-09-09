#include <stdlib.h>

#include <riscv_vector.h>

void _sgemm_rvv(float *restrict c, float const *restrict a,
                float const *restrict b, size_t M, size_t K, size_t N) {
  size_t ii, jj, kk;
  size_t vl;
  float *pc, *po;
  const float *pa, *pin, *pb;
  size_t rows;
  vfloat32m4_t vb0m4, vc0m4, vc1m4, vc2m4, vc3m4;
  vfloat32m8_t va0m8, vc0m8, vc1m8;

  po = c;
  pin = a;
  for (jj = M / 4; jj > 0; jj--) {
    pc = po;
    pb = b;
    for (ii = N; ii > 0; ii -= vl) {
      vl = __riscv_vsetvl_e32m4(ii);
      pa = pin;
      vc0m4 = __riscv_vfmv_v_f_f32m4(0.0, vl);
      vc1m4 = __riscv_vmv_v_v_f32m4(vc0m4, vl);
      vc2m4 = __riscv_vmv_v_v_f32m4(vc0m4, vl);
      vc3m4 = __riscv_vmv_v_v_f32m4(vc0m4, vl);
      for (kk = 0; kk < K; kk++) {
        vb0m4 = __riscv_vle32_v_f32m4(pb + kk * N, vl);
        vc0m4 = __riscv_vfmacc_vf_f32m4(vc0m4, *pa, vb0m4, vl);
        vc1m4 = __riscv_vfmacc_vf_f32m4(vc1m4, *(pa + K), vb0m4, vl);
        vc2m4 = __riscv_vfmacc_vf_f32m4(vc2m4, *(pa + 2 * K), vb0m4, vl);
        vc3m4 = __riscv_vfmacc_vf_f32m4(vc3m4, *(pa + 3 * K), vb0m4, vl);
        pa++;
      }
      __riscv_vse32_v_f32m4(pc, vc0m4, vl);
      __riscv_vse32_v_f32m4(pc + N, vc1m4, vl);
      __riscv_vse32_v_f32m4(pc + 2 * N, vc2m4, vl);
      __riscv_vse32_v_f32m4(pc + 3 * N, vc3m4, vl);
      pc += vl;
      pb += vl;
    }
    pin += 4 * K;
    po += 4 * N;
  }

  rows = M & 0x3;
  for (jj = rows / 2; jj > 0; jj--) {
    pc = po;
    pb = b;
    for (ii = N; ii > 0; ii -= vl) {
      vl = __riscv_vsetvl_e32m8(ii);
      pa = pin;
      vc0m8 = __riscv_vfmv_v_f_f32m8(0.0, vl);
      vc1m8 = __riscv_vmv_v_v_f32m8(vc0m8, vl);
      for (kk = 0; kk < K; kk++) {
        va0m8 = __riscv_vle32_v_f32m8(pb + kk * N, vl);
        vc0m8 = __riscv_vfmacc_vf_f32m8(vc0m8, *pa, va0m8, vl);
        vc1m8 = __riscv_vfmacc_vf_f32m8(vc1m8, *(pa + K), va0m8, vl);
        pa++;
      }
      __riscv_vse32_v_f32m8(pc, vc0m8, vl);
      __riscv_vse32_v_f32m8(pc + N, vc1m8, vl);
      pc += vl;
      pb += vl;
    }
    pin += 2 * K;
    po += 2 * N;
  }

  rows = M & 0x1;
  for (jj = rows; jj > 0; jj--) {
    pc = po;
    pb = b;
    for (ii = N; ii > 0; ii -= vl) {
      vl = __riscv_vsetvl_e32m8(ii);
      pa = pin;
      vc0m8 = __riscv_vfmv_v_f_f32m8(0.0, vl);
      for (kk = 0; kk < K; kk++) {
        va0m8 = __riscv_vle32_v_f32m8(pb + kk * N, vl);
        vc0m8 = __riscv_vfmacc_vf_f32m8(vc0m8, *pa++, va0m8, vl);
      }
      __riscv_vse32_v_f32m8(pc, vc0m8, vl);
      pc += vl;
      pb += vl;
    }
    pin += K;
    po += N;
  }
}
