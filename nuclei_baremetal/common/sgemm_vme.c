#include <stdio.h>
#include <stdlib.h>

#include <riscv_vector.h>

#include "nuclei_sdk_soc.h"
#include "zvt_encoding.h"

extern void _sgemm_scalar(float *restrict c, float const *restrict a,
                          float const *restrict b, size_t M, size_t K,
                          size_t N);

static void __enable_vme(void)
{
    __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MS);
    __RV_CSR_SET(CSR_MSTATUS, MSTATUS_MS_INITIAL);
}

void _sgemm_vme(float *restrict c, float const *restrict a,
                float const *restrict b, size_t M, size_t K, size_t N) {
  if (!M || !K || !N)
    return;
  if (M != K || K != N) {
    return;
  }

  __enable_vme();

  uintptr_t vtype;
  __asm__ volatile("vsetivli zero, 1, e32, m1, ta, ma\n"
                   "csrr %0, vtype"
                   : "=r"(vtype)
                   :
                   : "memory");
  zvt_msetmtype(ZVT_MTYPE_VALUE(0, 0, ZVT_MTWIDEN_1X), vtype);
  if (zvt_msettm(M) != M || zvt_msettn(N) != N || zvt_msettk(1) != 1) {
    printf("Wrong tile size\n");
    return;
  }

  for (size_t row = 0; row < M; ++row) {
    zvt_vtle32(ZVT_TSS_COL_OF(ZVT_MT0, row), a + row * N);
    zvt_vtle32(ZVT_TSS_ROW_OF(ZVT_MT4, row), b + row * N);
    zvt_vtle32(ZVT_TSS_ROW_OF(ZVT_MT8, row), c + row * N);
  }

  __asm__ volatile("vsetvli zero, %0, e32, m1, ta, ma" : : "r"(N) : "memory");

  size_t k = 0;
  for (; k + 4 <= K; k += 4) {
    __asm__ volatile(".insn r 0x57, 6, 0x21, v8, %[a_col0], x31\n"
                     ".insn r 0x57, 6, 0x21, v16, %[b_row0], x31\n"
                     ZVT_C_ASM_WORD(fmm0)
                     ".insn r 0x57, 6, 0x21, v9, %[a_col1], x31\n"
                     ".insn r 0x57, 6, 0x21, v17, %[b_row1], x31\n"
                     ZVT_C_ASM_WORD(fmm1)
                     ".insn r 0x57, 6, 0x21, v10, %[a_col2], x31\n"
                     ".insn r 0x57, 6, 0x21, v18, %[b_row2], x31\n"
                     ZVT_C_ASM_WORD(fmm2)
                     ".insn r 0x57, 6, 0x21, v11, %[a_col3], x31\n"
                     ".insn r 0x57, 6, 0x21, v19, %[b_row3], x31\n"
                     ZVT_C_ASM_WORD(fmm3)
                     :
                     : [a_col0] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k)),
                       [a_col1] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 1)),
                       [a_col2] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 2)),
                       [a_col3] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k + 3)),
                       [b_row0] "r"(ZVT_TSS_ROW_OF(ZVT_MT4, k)),
                       [b_row1] "r"(ZVT_TSS_ROW_OF(ZVT_MT4, k + 1)),
                       [b_row2] "r"(ZVT_TSS_ROW_OF(ZVT_MT4, k + 2)),
                       [b_row3] "r"(ZVT_TSS_ROW_OF(ZVT_MT4, k + 3)),
                       ZVT_C_ASM_IMM(fmm0, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 8, 16)),
                       ZVT_C_ASM_IMM(fmm1, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 9, 17)),
                       ZVT_C_ASM_IMM(fmm2, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 10, 18)),
                       ZVT_C_ASM_IMM(fmm3, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 11, 19))
                     : "memory");
  }

  for (; k < K; ++k) {
    __asm__ volatile(
        ".insn r 0x57, 6, 0x21, v8, %[a_col], x31\n"
        ".insn r 0x57, 6, 0x21, v16, %[b_row], x31\n"
        ZVT_C_ASM_WORD(fmm)
        :
        : [a_col] "r"(ZVT_TSS_ROW_OF(ZVT_MT0, k)),
          [b_row] "r"(ZVT_TSS_ROW_OF(ZVT_MT4, k)),
          ZVT_C_ASM_IMM(fmm, ZVT_ENC_VTFMM_TVV(ZVT_MT8, 8, 16))
        : "memory");
  }

  for (size_t row = 0; row < M; ++row)
    zvt_vtse32(ZVT_TSS_ROW_OF(ZVT_MT8, row), c + row * N);
}
