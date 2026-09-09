#include <stdlib.h>

#include <riscv_vector.h>

#include "zvt_encoding.h"

extern void _sgemm_scalar(float *restrict c, float const *restrict a,
                          float const *restrict b, size_t M, size_t K,
                          size_t N);

void _sgemm_vme(float *restrict c, float const *restrict a,
                float const *restrict b, size_t M, size_t K, size_t N) {
  if (!M || !K || !N)
    return;
  if (M != K || K != N) {
    _sgemm_scalar(c, a, b, M, K, N);
    return;
  }

#if !defined(__riscv)
  _sgemm_scalar(c, a, b, M, K, N);
#else
  uintptr_t vtype;
  __asm__ volatile("vsetivli zero, 1, e32, m1, ta, ma\n"
                   "csrr %0, vtype"
                   : "=r"(vtype)
                   :
                   : "memory");
  zvt_msetmtype(ZVT_MTYPE_VALUE(0, 0, ZVT_MTWIDEN_1X), vtype);
  if (zvt_msettm(M) != M || zvt_msettn(N) != N || zvt_msettk(1) != 1) {
    _sgemm_scalar(c, a, b, M, K, N);
    return;
  }

  for (size_t row = 0; row < M; ++row) {
    zvt_vtle32(ZVT_TSS_COL_OF(ZVT_MT0, row), a + row * N);
    zvt_vtle32(ZVT_TSS_ROW_OF(ZVT_MT8, row), c + row * N);
  }

  size_t k = 0;
  for (; k + 4 <= K; k += 4) {
    __asm__ volatile(
        "vsetvli zero, %[n], e32, m1, ta, ma\n"
        ".insn r 0x57, 6, 0x21, v8, %[a_col0], x31\n"
        ".insn r 0x57, 6, 0x21, v9, %[a_col1], x31\n"
        ".insn r 0x57, 6, 0x21, v10, %[a_col2], x31\n"
        ".insn r 0x57, 6, 0x21, v11, %[a_col3], x31\n"
        "vle32.v v16, (%[b0])\n"
        "vle32.v v17, (%[b1])\n"
        "vle32.v v18, (%[b2])\n"
        "vle32.v v19, (%[b3])\n" ZVT_C_ASM_WORD(fmm0) ZVT_C_ASM_WORD(fmm1)
            ZVT_C_ASM_WORD(fmm2) ZVT_C_ASM_WORD(fmm3)
        :
        : [a_col0] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k)),
          [a_col1] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 1)),
          [a_col2] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 2)),
          [a_col3] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 3)), [b0] "r"(b + k * N),
          [b1] "r"(b + (k + 1) * N), [b2] "r"(b + (k + 2) * N),
          [b3] "r"(b + (k + 3) * N), [n] "r"(N),
          ZVT_C_ASM_IMM(fmm0, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 8, 16)),
          ZVT_C_ASM_IMM(fmm1, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 9, 17)),
          ZVT_C_ASM_IMM(fmm2, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 10, 18)),
          ZVT_C_ASM_IMM(fmm3, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 11, 19))
        : "memory");
  }

  for (; k < K; ++k) {
    __asm__ volatile("vsetvli zero, %[n], e32, m1, ta, ma\n"
                     ".insn r 0x57, 6, 0x21, v8, %[a_col], x31\n"
                     "vle32.v v16, (%[b])\n" ZVT_C_ASM_WORD(fmm)
                     :
                     : [a_col] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k)),
                       [b] "r"(b + k * N), [n] "r"(N),
                       ZVT_C_ASM_IMM(fmm, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 8, 16))
                     : "memory");
  }

  for (size_t row = 0; row < M; ++row)
    zvt_vtse32(ZVT_TSS_ROW_OF(ZVT_MT8, row), c + row * N);
#endif
}
