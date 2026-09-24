#include <stddef.h>
#include <stdint.h>

#include "nuclei_sdk_soc.h"
#include "zvt_encoding.h"

#define SGEMM_TILE (32)

static void __enable_vme(void) {
  __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MS);
  __RV_CSR_SET(CSR_MSTATUS, MSTATUS_MS_INITIAL);
}

static inline uintptr_t asm_msettn(uintptr_t requested) {
  uintptr_t actual;
  __asm__ volatile("msettn %0, %1"
                   : "=r"(actual)
                   : "r"(requested)
                   : "memory");
  return actual;
}

static inline void asm_vtle32(uintptr_t tss, const void *addr) {
  __asm__ volatile("vtle32 %0, (%1)" : : "r"(tss), "r"(addr) : "memory");
}

static inline void asm_vtse32(uintptr_t tss, void *addr) {
  __asm__ volatile("vtse32 %0, (%1)" : : "r"(tss), "r"(addr) : "memory");
}

void _sgemm_vme_asm(float *restrict c, float const *restrict a,
                    float const *restrict b, size_t M, size_t K, size_t N) {
  if (!M || !K || !N || M % SGEMM_TILE || K % SGEMM_TILE ||
      N % SGEMM_TILE)
    return;

  __enable_vme();

  uintptr_t tn;
  __asm__ volatile("vsetvli %0, %1, e32, m1, ta, ma"
                   : "=r"(tn)
                   : "r"(SGEMM_TILE)
                   : "memory");
  uintptr_t vtype = __RV_CSR_READ(CSR_VTYPE);
  if (tn != SGEMM_TILE)
    return;

  for (size_t m0 = 0; m0 < M; m0 += SGEMM_TILE) {
    __asm__ volatile("msetmtype %0, %1"
                     :
                     : "r"(ZVT_MTYPE_VALUE(SGEMM_TILE, 1, ZVT_MTWIDEN_1X)),
                       "r"(vtype)
                     : "memory");
    if (asm_msettn(SGEMM_TILE) != SGEMM_TILE)
      return;

    for (size_t row = 0; row < SGEMM_TILE; ++row) {
      asm_vtle32(ZVT_TSS_COL_OF(ZVT_MT0, row), a + (m0 + row) * K);
      asm_vtle32(ZVT_TSS_ROW_OF(ZVT_MT4, row), c + (m0 + row) * N);
    }

    for (size_t n0 = 0; n0 < N; n0 += SGEMM_TILE) {
      const unsigned c_tile = n0 / SGEMM_TILE & 1 ? ZVT_MT12 : ZVT_MT4;

      for (size_t k0 = 0; k0 < K; k0 += SGEMM_TILE) {
        const unsigned a_tile = k0 / SGEMM_TILE & 1 ? ZVT_MT8 : ZVT_MT0;

        for (size_t k = 0; k < SGEMM_TILE; k += 4) {
          if (c_tile == ZVT_MT4) {
            __asm__ volatile(
                "vtmv.v.t v8, %[a_col0]\n"
                "vtmv.v.t v9, %[a_col1]\n"
                "vtmv.v.t v10, %[a_col2]\n"
                "vtmv.v.t v11, %[a_col3]\n"
                "vle32.v v16, (%[b0])\n"
                "vle32.v v17, (%[b1])\n"
                "vle32.v v18, (%[b2])\n"
                "vle32.v v19, (%[b3])\n"
                "vtfmm.tvv mt4, v8, v16\n"
                "vtfmm.tvv mt4, v9, v17\n"
                "vtfmm.tvv mt4, v10, v18\n"
                "vtfmm.tvv mt4, v11, v19\n"
                :
                : [a_col0] "r"(ZVT_TSS_ROW_OF(a_tile, k)),
                  [a_col1] "r"(ZVT_TSS_ROW_OF(a_tile, k + 1)),
                  [a_col2] "r"(ZVT_TSS_ROW_OF(a_tile, k + 2)),
                  [a_col3] "r"(ZVT_TSS_ROW_OF(a_tile, k + 3)),
                  [b0] "r"(b + (k0 + k) * N + n0),
                  [b1] "r"(b + (k0 + k + 1) * N + n0),
                  [b2] "r"(b + (k0 + k + 2) * N + n0),
                  [b3] "r"(b + (k0 + k + 3) * N + n0)
                : "memory");
          } else {
            __asm__ volatile(
                "vtmv.v.t v8, %[a_col0]\n"
                "vtmv.v.t v9, %[a_col1]\n"
                "vtmv.v.t v10, %[a_col2]\n"
                "vtmv.v.t v11, %[a_col3]\n"
                "vle32.v v16, (%[b0])\n"
                "vle32.v v17, (%[b1])\n"
                "vle32.v v18, (%[b2])\n"
                "vle32.v v19, (%[b3])\n"
                "vtfmm.tvv mt12, v8, v16\n"
                "vtfmm.tvv mt12, v9, v17\n"
                "vtfmm.tvv mt12, v10, v18\n"
                "vtfmm.tvv mt12, v11, v19\n"
                :
                : [a_col0] "r"(ZVT_TSS_ROW_OF(a_tile, k)),
                  [a_col1] "r"(ZVT_TSS_ROW_OF(a_tile, k + 1)),
                  [a_col2] "r"(ZVT_TSS_ROW_OF(a_tile, k + 2)),
                  [a_col3] "r"(ZVT_TSS_ROW_OF(a_tile, k + 3)),
                  [b0] "r"(b + (k0 + k) * N + n0),
                  [b1] "r"(b + (k0 + k + 1) * N + n0),
                  [b2] "r"(b + (k0 + k + 2) * N + n0),
                  [b3] "r"(b + (k0 + k + 3) * N + n0)
                : "memory");
          }
          if (!k0 && n0) {
            uintptr_t c_tile_tss = (uintptr_t)(c_tile ^ ZVT_MT8) << 27;
            uintptr_t c_row;
            __asm__ volatile(
                "or %[c_row], %[c_tile_tss], %[k]\n"
                "vtse32 %[c_row], (%[c_addr0])\n"
                "addi %[c_row], %[c_row], 1\n"
                "vtse32 %[c_row], (%[c_addr1])\n"
                "addi %[c_row], %[c_row], 1\n"
                "vtse32 %[c_row], (%[c_addr2])\n"
                "addi %[c_row], %[c_row], 1\n"
                "vtse32 %[c_row], (%[c_addr3])\n"
                : [c_row] "=&r"(c_row)
                : [c_tile_tss] "r"(c_tile_tss), [k] "r"(k),
                  [c_addr0] "r"(c + (m0 + k) * N + n0 - SGEMM_TILE),
                  [c_addr1] "r"(c + (m0 + k + 1) * N + n0 - SGEMM_TILE),
                  [c_addr2] "r"(c + (m0 + k + 2) * N + n0 - SGEMM_TILE),
                  [c_addr3] "r"(c + (m0 + k + 3) * N + n0 - SGEMM_TILE)
                : "memory");
          }

          if (k0 + SGEMM_TILE < K) {
            uintptr_t a_tile_tss = (uintptr_t)(a_tile ^ ZVT_MT8) << 27;
            uintptr_t a_col0, a_col1, a_col2, a_col3;
            __asm__ volatile(
                "or %[a_col0], %[a_tile_tss], %[k]\n"
                "addi %[a_col1], %[a_col0], 1\n"
                "addi %[a_col2], %[a_col0], 2\n"
                "addi %[a_col3], %[a_col0], 3\n"
                "bseti %[a_col0], %[a_col0], 24\n"
                "bseti %[a_col1], %[a_col1], 24\n"
                "bseti %[a_col2], %[a_col2], 24\n"
                "bseti %[a_col3], %[a_col3], 24\n"
                "vtle32 %[a_col0], (%[a_addr0])\n"
                "vtle32 %[a_col1], (%[a_addr1])\n"
                "vtle32 %[a_col2], (%[a_addr2])\n"
                "vtle32 %[a_col3], (%[a_addr3])\n"
                : [a_col0] "=&r"(a_col0), [a_col1] "=&r"(a_col1),
                  [a_col2] "=&r"(a_col2), [a_col3] "=&r"(a_col3)
                : [a_tile_tss] "r"(a_tile_tss), [k] "r"(k),
                  [a_addr0] "r"(a + (m0 + k) * K + k0 + SGEMM_TILE),
                  [a_addr1] "r"(a + (m0 + k + 1) * K + k0 + SGEMM_TILE),
                  [a_addr2] "r"(a + (m0 + k + 2) * K + k0 + SGEMM_TILE),
                  [a_addr3] "r"(a + (m0 + k + 3) * K + k0 + SGEMM_TILE)
                : "memory");
          } else if (n0 + SGEMM_TILE < N) {
            uintptr_t c_tile_tss = (uintptr_t)(c_tile ^ ZVT_MT8) << 27;
            uintptr_t a_col, c_row;
            __asm__ volatile(
                "bseti %[a_col], %[k], 24\n"
                "or %[c_row], %[c_tile_tss], %[k]\n"
                "vtle32 %[a_col], (%[a_addr0])\n"
                "vtle32 %[c_row], (%[c_addr0])\n"
                "addi %[a_col], %[a_col], 1\n"
                "addi %[c_row], %[c_row], 1\n"
                "vtle32 %[a_col], (%[a_addr1])\n"
                "vtle32 %[c_row], (%[c_addr1])\n"
                "addi %[a_col], %[a_col], 1\n"
                "addi %[c_row], %[c_row], 1\n"
                "vtle32 %[a_col], (%[a_addr2])\n"
                "vtle32 %[c_row], (%[c_addr2])\n"
                "addi %[a_col], %[a_col], 1\n"
                "addi %[c_row], %[c_row], 1\n"
                "vtle32 %[a_col], (%[a_addr3])\n"
                "vtle32 %[c_row], (%[c_addr3])\n"
                : [a_col] "=&r"(a_col), [c_row] "=&r"(c_row)
                : [c_tile_tss] "r"(c_tile_tss), [k] "r"(k),
                  [a_addr0] "r"(a + (m0 + k) * K),
                  [c_addr0] "r"(c + (m0 + k) * N + n0 + SGEMM_TILE),
                  [a_addr1] "r"(a + (m0 + k + 1) * K),
                  [c_addr1] "r"(c + (m0 + k + 1) * N + n0 + SGEMM_TILE),
                  [a_addr2] "r"(a + (m0 + k + 2) * K),
                  [c_addr2] "r"(c + (m0 + k + 2) * N + n0 + SGEMM_TILE),
                  [a_addr3] "r"(a + (m0 + k + 3) * K),
                  [c_addr3] "r"(c + (m0 + k + 3) * N + n0 + SGEMM_TILE)
                : "memory");
          }
        }
      }
      if (n0 + SGEMM_TILE == N)
        for (size_t row = 0; row < SGEMM_TILE; ++row)
          asm_vtse32(ZVT_TSS_ROW_OF(c_tile, row),
                      c + (m0 + row) * N + n0);
    }
  }
}
