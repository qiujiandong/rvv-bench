#include <stdint.h>
#include <stdio.h>

#include <riscv_vector.h>

#include "nuclei_sdk_soc.h"
#include "zvt_encoding.h"

#define DIM 4

static const float matmul_a[DIM * DIM] = {
    1, 2, 3, 4,  -1, 0, 1, 2,
    2, -2, 1, 0, 3, 1, -1, 2,
};
static const float matmul_b[DIM * DIM] = {
    1, 2, 3, 4,  2, -1, 0, 1,
    1, 0, 2, -2, 0, 1, -1, 3,
};
static const float matmul_expected[DIM * DIM] = {
    1, 6, 4, 8,  0, 5, 1, 15,
    6, 4, 12, 8, 8, 8, 10, 24,
};
static const float tile_zero[DIM * DIM];
static const float transpose_input[DIM * DIM] = {
    1, 2, 3, 4, 5, 6, 7, 8,
    9, 10, 11, 12, 13, 14, 15, 16,
};
static const float transpose_expected[DIM * DIM] = {
    1, 5, 9, 13, 2, 6, 10, 14,
    3, 7, 11, 15, 4, 8, 12, 16,
};

static int check(const char *name, const float *actual, const float *expected)
{
    int failed = 0;

    for (unsigned i = 0; i < DIM * DIM; ++i) {
        if (actual[i] != expected[i]) {
            printf("%s FAIL [%u]: expected %.1f, got %.1f\n",
                   name, i, expected[i], actual[i]);
            failed = 1;
        }
    }
    if (failed)
        return 1;
    printf("%s PASS\n", name);
    return 0;
}

static void __enable_vme(void)
{
    __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MS);
    __RV_CSR_SET(CSR_MSTATUS, MSTATUS_MS_INITIAL);
}

static void __disable_vme(void)
{
    __RV_CSR_CLEAR(CSR_MSTATUS, MSTATUS_MS);
}

static int configure_f32_tile(void)
{
    uintptr_t vtype;


    // `msetmtypei` is not supported
    // ZVT_C_EMIT(ZVT_ENC_MSETMTYPEI(
    //     ZVT_VSEW_32, ZVT_MTYPEI_LO5(ZVT_MTWIDEN_1X)));

    __asm__ volatile ("vsetivli zero, 4, e32, m1, ta, ma\n"
                      "csrr %0, vtype"
                      : "=r"(vtype) : : "memory");
    zvt_msetmtype(ZVT_MTYPE_VALUE(0, 0, ZVT_MTWIDEN_1X), vtype);

    if (zvt_msettm(DIM) != DIM || zvt_msettn(DIM) != DIM ||
        zvt_msettk(1) != 1) {
        printf("Zvt tile smaller than %ux%u\n", DIM, DIM);
        return 1;
    }

    __enable_vme();

    return 0;
}

static void matmul(float *result)
{
    // `vtzero` is not supported
    // ZVT_C_EMIT(ZVT_ENC_VTZERO(ZVT_MT0));

    printf("loading tile zero\n");
    for (unsigned row = 0; row < DIM; ++row)
        zvt_vtle32(ZVT_TSS_ROW_OF(ZVT_MT0, row), tile_zero + row * DIM);

    printf("vtfmm.tvv\n");
    for (unsigned k = 0; k < DIM; ++k) {
        __asm__ volatile (
            "vle32.v v8, (%0)\n"
            "vle32.v v16, (%1)\n"
            ZVT_C_ASM_WORD(fmm)
            :
            : "r"(matmul_a + k * DIM), "r"(matmul_b + k * DIM),
              ZVT_C_ASM_IMM(fmm, ZVT_ENC_VTFMM_TVV(ZVT_MT0, 8, 16))
            : "memory");
    }

    printf("storing result\n");
    for (unsigned row = 0; row < DIM; ++row)
        zvt_vtse32(ZVT_TSS_ROW_OF(ZVT_MT0, row), result + row * DIM);
}

static void transpose(float *result)
{
    printf("load input\n");
    for (unsigned row = 0; row < DIM; ++row)
        zvt_vtle32(ZVT_TSS_ROW_OF(ZVT_MT0, row),
                    transpose_input + row * DIM);

    printf("tile transpose\n");
    for (unsigned col = 0; col < DIM; ++col)
        zvt_vtse32(ZVT_TSS_COL_OF(ZVT_MT0, col), result + col * DIM);
}

int main(void)
{
    float result[DIM * DIM];
    int failed;

    if (configure_f32_tile())
        return 1;

    matmul(result);
    failed = check("vtfmm.tvv", result, matmul_expected);
    transpose(result);
    failed |= check("tile transpose", result, transpose_expected);

    // `vtdiscard` is not supported
    // ZVT_C_EMIT(ZVT_ENC_VTDISCARD);

    return failed;
}
