#include <stdio.h>
#include <stdlib.h>

#include <riscv_vector.h>

#include "nuclei_sdk_soc.h"
#include "zvt_encoding.h"

extern void _sgemm_scalar(float *restrict c, float const *restrict a,
                          float const *restrict b, size_t M, size_t K,
                          size_t N);

#define SGEMM_TILE (32)

static void __enable_vme(void) {
  __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MS);
  __RV_CSR_SET(CSR_MSTATUS, MSTATUS_MS_INITIAL);
}

void _sgemm_vme(float *restrict c, float const *restrict a,
                float const *restrict b, size_t M, size_t K, size_t N) {
  if (!M || !K || !N || M % SGEMM_TILE || K % SGEMM_TILE ||
      N % SGEMM_TILE)
    return;

  __enable_vme();

  uintptr_t tn = __riscv_vsetvl_e32m1(SGEMM_TILE);
  uintptr_t vtype = __RV_CSR_READ(CSR_VTYPE);
  if (tn != SGEMM_TILE)
    return;

  for (size_t m0 = 0; m0 < M; m0 += SGEMM_TILE) {
    for (size_t n0 = 0; n0 < N; n0 += SGEMM_TILE) {
      for (size_t k0 = 0; k0 < K; k0 += SGEMM_TILE) {
        zvt_msetmtype(ZVT_MTYPE_VALUE(SGEMM_TILE, 1, ZVT_MTWIDEN_1X),
                      vtype);

        for (size_t row = 0; row < SGEMM_TILE; ++row) {
          zvt_vtle32(ZVT_TSS_COL_OF(ZVT_MT0, row),
                      a + (m0 + row) * K + k0);
          zvt_vtle32(ZVT_TSS_ROW_OF(ZVT_MT8, row),
                      c + (m0 + row) * N + n0);
        }

        __asm__ volatile("vsetvli zero, %[tn], e32, m1, ta, ma\n"
                         :
                         : [tn] "r"(tn)
                         : "memory");

        for (size_t k = 0; k < SGEMM_TILE; k += 4) {
          __asm__ volatile(
              ".insn r 0x57, 6, 0x21, v8, %[a_col0], x31\n"
              ".insn r 0x57, 6, 0x21, v9, %[a_col1], x31\n"
              ".insn r 0x57, 6, 0x21, v10, %[a_col2], x31\n"
              ".insn r 0x57, 6, 0x21, v11, %[a_col3], x31\n"
              "vle32.v v16, (%[b0])\n"
              "vle32.v v17, (%[b1])\n"
              "vle32.v v18, (%[b2])\n"
              "vle32.v v19, (%[b3])\n" ZVT_C_ASM_WORD(fmm0)
                  ZVT_C_ASM_WORD(fmm1) ZVT_C_ASM_WORD(fmm2)
                      ZVT_C_ASM_WORD(fmm3)
              :
              : [a_col0] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k)),
                [a_col1] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 1)),
                [a_col2] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 2)),
                [a_col3] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 3)),
                [b0] "r"(b + (k0 + k) * N + n0),
                [b1] "r"(b + (k0 + k + 1) * N + n0),
                [b2] "r"(b + (k0 + k + 2) * N + n0),
                [b3] "r"(b + (k0 + k + 3) * N + n0),
                ZVT_C_ASM_IMM(fmm0, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 8, 16)),
                ZVT_C_ASM_IMM(fmm1, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 9, 17)),
                ZVT_C_ASM_IMM(fmm2, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 10, 18)),
                ZVT_C_ASM_IMM(fmm3, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 11, 19))
              : "memory");
        }

        for (size_t row = 0; row < SGEMM_TILE; ++row)
          zvt_vtse32(ZVT_TSS_ROW_OF(ZVT_MT8, row),
                      c + (m0 + row) * N + n0);
      }
    }
  }
}
