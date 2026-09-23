#ifndef ZVT_ENCODING_H
#define ZVT_ENCODING_H

#define MSTATUS_MS 0x60000000
#define MSTATUS_MS_INITIAL 0x20000000
#define MSTATUS_MS_CLEAN 0x40000000
#define MSTATUS_MS_DIRTY 0x60000000
#define CSR_MTYPE 0xC23

/*
 * zvt_encoding.h - raw encodings for RISC-V Zvt Vector Matrix Extensions v0.4
 *
 * Source: "Chapter 12. Matrix Extensions", Zvt family Version 0.4,
 * pages 564-579 of the supplied RISC-V Instruction Set Manual excerpt.
 *
 * Purpose:
 *   - use Zvt instructions before the assembler knows their mnemonics;
 *   - share one encoding definition between hand-written .S and C/C++ tests;
 *   - make later replacement by official mnemonics straightforward.
 *
 * IMPORTANT:
 *   1. This header follows the supplied v0.4 draft exactly. Draft encodings can change.
 *   2. The hardware/ISS must implement the same draft; otherwise execution traps.
 *   3. Raw .4byte instructions are invisible to the compiler's vector/tile register
 *      allocator. Keep RVV loads/stores and raw Zvt arithmetic in the SAME inline-asm
 *      block, or use a .S microkernel, so the compiler cannot move code across them.
 *   4. Register-number arguments below are architectural numbers (x0..x31 => 0..31,
 *      v0..v31 => 0..31). Matrix tile arguments use the architectural tile specifier
 *      (mt0 => 0, mt4 => 4, etc.), not the encoded rd field value.
 */

/* ---------- common bit helpers ---------- */

#ifdef __ASSEMBLER__
#define ZVT_U32(x)                (x)
#define ZVT_MASK(v, bits)         ((v) & ((1 << (bits)) - 1))
#define ZVT_FIELD(v, shift, bits) (ZVT_MASK((v), (bits)) << (shift))
#else
#define ZVT_U32(x)                ((uint32_t)(x))
#define ZVT_MASK(v, bits)         (ZVT_U32(v) & ((UINT32_C(1) << (bits)) - UINT32_C(1)))
#define ZVT_FIELD(v, shift, bits) (ZVT_MASK((v), (bits)) << (shift))
#endif

#define ZVT_STR_1(x) #x
#define ZVT_STR(x)   ZVT_STR_1(x)

/* Architectural register/tile numbers, for readability in raw encoders. */
#define ZVT_X0   0
#define ZVT_RA   1
#define ZVT_SP   2
#define ZVT_GP   3
#define ZVT_TP   4
#define ZVT_T0   5
#define ZVT_T1   6
#define ZVT_T2   7
#define ZVT_S0   8
#define ZVT_FP   8
#define ZVT_S1   9
#define ZVT_A0  10
#define ZVT_A1  11
#define ZVT_A2  12
#define ZVT_A3  13
#define ZVT_A4  14
#define ZVT_A5  15
#define ZVT_A6  16
#define ZVT_A7  17
#define ZVT_S2  18
#define ZVT_S3  19
#define ZVT_S4  20
#define ZVT_S5  21
#define ZVT_S6  22
#define ZVT_S7  23
#define ZVT_S8  24
#define ZVT_S9  25
#define ZVT_S10 26
#define ZVT_S11 27
#define ZVT_T3  28
#define ZVT_T4  29
#define ZVT_T5  30
#define ZVT_T6  31

#define ZVT_MT0   0
#define ZVT_MT1   1
#define ZVT_MT2   2
#define ZVT_MT3   3
#define ZVT_MT4   4
#define ZVT_MT5   5
#define ZVT_MT6   6
#define ZVT_MT7   7
#define ZVT_MT8   8
#define ZVT_MT9   9
#define ZVT_MT10 10
#define ZVT_MT11 11
#define ZVT_MT12 12
#define ZVT_MT13 13
#define ZVT_MT14 14
#define ZVT_MT15 15

/* ---------- mtype CSR ---------- */

#define ZVT_MTYPE_CSR 0xC23

/* mtwiden field encoding. mtwiden=0 means matrix unit not configured. */
#define ZVT_MTWIDEN_OFF 0
#define ZVT_MTWIDEN_1X  1
#define ZVT_MTWIDEN_2X  2
#define ZVT_MTWIDEN_4X  3

/* Build the writable low fields used by msetmtype/msetmtypei.
 * tm[13:0] lives at mtype[23:10], tk[2:0] at [7:5], mtwiden[1:0] at [1:0].
 */
#define ZVT_MTYPE_VALUE(tm, tk, mtwiden) \
    (ZVT_FIELD((tm), 10, 14) | ZVT_FIELD((tk), 5, 3) | ZVT_FIELD((mtwiden), 0, 2))

/* vsew immediate values used by msetmtypei. */
#define ZVT_VSEW_8   0
#define ZVT_VSEW_16  1
#define ZVT_VSEW_32  2
#define ZVT_VSEW_64  3

#define ZVT_ALTFMT_NORMAL 0
#define ZVT_ALTFMT_ALT    1

/* ---------- Tile Subset Specifier (TSS), spec section 12.1.1.5 ---------- */

#define ZVT_TSS_ROW 0
#define ZVT_TSS_COL 1

#define ZVT_TSS(tile, pattern, index) \
    (ZVT_FIELD((tile), 27, 4) | ZVT_FIELD((pattern), 24, 3) | ZVT_FIELD((index), 0, 24))

#define ZVT_TSS_ROW_OF(tile, row) ZVT_TSS((tile), ZVT_TSS_ROW, (row))
#define ZVT_TSS_COL_OF(tile, col) ZVT_TSS((tile), ZVT_TSS_COL, (col))

/* ---------- generic instruction encoders ---------- */

#define ZVT_ENC_R(funct7, rs2, rs1, funct3, rd, opcode) \
    (ZVT_FIELD((funct7), 25, 7) | ZVT_FIELD((rs2), 20, 5) | \
     ZVT_FIELD((rs1), 15, 5) | ZVT_FIELD((funct3), 12, 3) | \
     ZVT_FIELD((rd), 7, 5) | ZVT_FIELD((opcode), 0, 7))

/* Generic Zvt arithmetic encoding. mtd is the architectural tile specifier.
 * Arithmetic instructions encode t[3:1] in rd[4:2], rd[1]=0 and use rd[0]
 * to select the normal/alternate arithmetic form. This matches the v0.4
 * encoding diagram on page 578.
 */
#define ZVT_ARITH_RD(mtd, altbit) \
    (ZVT_FIELD(((ZVT_U32(mtd) >> 1)), 2, 3) | ZVT_FIELD((altbit), 0, 1))

#define ZVT_ENC_ARITH(funct3, altbit, mtd, vs2, vs1) \
    ZVT_ENC_R(0x79, (vs2), (vs1), (funct3), ZVT_ARITH_RD((mtd), (altbit)), 0x77)

/* ---------- configuration instructions, spec section 12.1.1.4 ---------- */

/* msetmtype x0, rs1=mtype, rs2=vtype */
#define ZVT_ENC_MSETMTYPE(rs1_mtype, rs2_vtype) \
    ZVT_ENC_R(0x42, (rs2_vtype), (rs1_mtype), 7, 0, 0x57)

/* msettn/msettm/msettk share funct7=0x44. Bits [24:20] are a fixed sub-op,
 * not a source-register dependency: 0=tn, 1=tm, 2=tk.
 */
#define ZVT_ENC_MSETTN(rd, rs1) ZVT_ENC_R(0x44, 0, (rs1), 7, (rd), 0x57)
#define ZVT_ENC_MSETTM(rd, rs1) ZVT_ENC_R(0x44, 1, (rs1), 7, (rd), 0x57)
#define ZVT_ENC_MSETTK(rd, rs1) ZVT_ENC_R(0x44, 2, (rs1), 7, (rd), 0x57)

/* msetmtypei: [31:29]=100, [28]=altfmt, [27:25]=100, [24:23]=vsew,
 * [22:20]=100, [19:17]=000, [16:15]=mtwiden, funct3=111, rd=x0,
 * opcode=0x57.
 */
#define ZVT_ENC_MSETMTYPEI(vsew, altfmt, mtwiden) \
    (ZVT_FIELD(4, 29, 3) | ZVT_FIELD((altfmt), 28, 1) | \
     ZVT_FIELD(4, 25, 3) | ZVT_FIELD((vsew), 23, 2) | \
     ZVT_FIELD(4, 20, 3) | ZVT_FIELD((mtwiden), 15, 2) | \
     ZVT_FIELD(7, 12, 3) | ZVT_FIELD(0, 7, 5) | 0x57)

/* ---------- tile subset memory transfers, spec section 12.1.1.6 ---------- */

/* EEW code eee in instruction bits [31:29]. */
#define ZVT_EEW_CODE_8   0
#define ZVT_EEW_CODE_16  1
#define ZVT_EEW_CODE_32  2
#define ZVT_EEW_CODE_64  3

/* bits[31:25] = eee:1:00:1 => (eee << 4) | 0x9 */
#define ZVT_TILE_MEM_FUNCT7(eew_code) ((ZVT_MASK((eew_code), 3) << 4) | 0x9)

#define ZVT_ENC_VTLE(eew_code, rs2_tss, rs1_addr) \
    ZVT_ENC_R(ZVT_TILE_MEM_FUNCT7(eew_code), (rs2_tss), (rs1_addr), 7, 0, 0x07)
#define ZVT_ENC_VTSE(eew_code, rs2_tss, rs1_addr) \
    ZVT_ENC_R(ZVT_TILE_MEM_FUNCT7(eew_code), (rs2_tss), (rs1_addr), 7, 0, 0x27)

#define ZVT_ENC_VTLE8(rs2_tss, rs1_addr)  ZVT_ENC_VTLE(ZVT_EEW_CODE_8,  (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTLE16(rs2_tss, rs1_addr) ZVT_ENC_VTLE(ZVT_EEW_CODE_16, (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTLE32(rs2_tss, rs1_addr) ZVT_ENC_VTLE(ZVT_EEW_CODE_32, (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTLE64(rs2_tss, rs1_addr) ZVT_ENC_VTLE(ZVT_EEW_CODE_64, (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTSE8(rs2_tss, rs1_addr)  ZVT_ENC_VTSE(ZVT_EEW_CODE_8,  (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTSE16(rs2_tss, rs1_addr) ZVT_ENC_VTSE(ZVT_EEW_CODE_16, (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTSE32(rs2_tss, rs1_addr) ZVT_ENC_VTSE(ZVT_EEW_CODE_32, (rs2_tss), (rs1_addr))
#define ZVT_ENC_VTSE64(rs2_tss, rs1_addr) ZVT_ENC_VTSE(ZVT_EEW_CODE_64, (rs2_tss), (rs1_addr))

/* ---------- tile <-> vector moves, spec section 12.1.1.7 ---------- */

#define ZVT_ENC_VTMV_V_T(vd, rs1_tss) \
    ZVT_ENC_R(0x21, 31, (rs1_tss), 6, (vd), 0x57)

#define ZVT_ENC_VTMV_T_V(rs1_tss, vs2) \
    ZVT_ENC_R(0x2f, (vs2), (rs1_tss), 6, 0, 0x57)

/* ---------- matrix arithmetic, spec section 12.1.1.8 ---------- */

/* FP: funct3=001; normal/alt selected by encoded rd[0]. */
#define ZVT_ENC_VTFMM_TVV(mtd, vs2, vs1) \
    ZVT_ENC_ARITH(1, 0, (mtd), (vs2), (vs1))
#define ZVT_ENC_VTFMM_ALT_TVV(mtd, vs2, vs1) \
    ZVT_ENC_ARITH(1, 1, (mtd), (vs2), (vs1))

/* Integer: funct3=000; vtmmu has unsigned vs2, vtmms signed vs2.
 * vs1 signedness is selected by altfmt in the configured vtype.
 */
#define ZVT_ENC_VTMMU_TVV(mtd, vs2, vs1) \
    ZVT_ENC_ARITH(0, 0, (mtd), (vs2), (vs1))
#define ZVT_ENC_VTMMS_TVV(mtd, vs2, vs1) \
    ZVT_ENC_ARITH(0, 1, (mtd), (vs2), (vs1))

/* ---------- tile zero, spec section 12.1.1.9 ---------- */

/* rd[4:1] = tile[3:0], rd[0]=0 */
#define ZVT_VTZERO_RD(mtd) ZVT_FIELD((mtd), 1, 4)
#define ZVT_ENC_VTZERO(mtd) \
    ZVT_ENC_R(0x21, 30, 0, 6, ZVT_VTZERO_RD(mtd), 0x57)

/* ---------- context discard, spec section 12.1.1.10 ---------- */

#define ZVT_ENC_VTDISCARD \
    ZVT_ENC_R(0x21, 28, 0, 6, 0, 0x57)

/* ---------- emission helpers ----------
 *
 * .S usage (preprocessed assembly):
 *     ZVT_S_VTFMM_TVV(ZVT_MT0, 8, 16)
 *
 * C/C++ simple usage:
 *     ZVT_C_EMIT(ZVT_ENC_VTZERO(ZVT_MT0));
 *
 * C/C++ combined inline-asm usage (recommended for vector/tile arithmetic):
 *     __asm__ volatile (
 *         "vle8.v v8, (%0)\n"
 *         "vle8.v v16, (%1)\n"
 *         ZVT_C_ASM_WORD(matmul)
 *         :
 *         : "r"(a), "r"(b),
 *           ZVT_C_ASM_IMM(matmul, ZVT_ENC_VTFMM_TVV(ZVT_MT0, 8, 16))
 *         : "memory");
 *
 * The named-immediate pair lets the compiler evaluate the uint32_t encoding, while
 * the assembler only sees the resulting integer through %[name].
 */

#ifdef __ASSEMBLER__

#define ZVT_S_WORD(enc)                    .4byte enc
#define ZVT_S_MSETMTYPE(rs1, rs2)          ZVT_S_WORD(ZVT_ENC_MSETMTYPE((rs1), (rs2)))
#define ZVT_S_MSETMTYPEI(vsew, alt, widen) ZVT_S_WORD(ZVT_ENC_MSETMTYPEI((vsew), (alt), (widen)))
#define ZVT_S_MSETTN(rd, rs1)               ZVT_S_WORD(ZVT_ENC_MSETTN((rd), (rs1)))
#define ZVT_S_MSETTM(rd, rs1)               ZVT_S_WORD(ZVT_ENC_MSETTM((rd), (rs1)))
#define ZVT_S_MSETTK(rd, rs1)               ZVT_S_WORD(ZVT_ENC_MSETTK((rd), (rs1)))
#define ZVT_S_VTLE8(tss, addr)              ZVT_S_WORD(ZVT_ENC_VTLE8((tss), (addr)))
#define ZVT_S_VTLE16(tss, addr)             ZVT_S_WORD(ZVT_ENC_VTLE16((tss), (addr)))
#define ZVT_S_VTLE32(tss, addr)             ZVT_S_WORD(ZVT_ENC_VTLE32((tss), (addr)))
#define ZVT_S_VTLE64(tss, addr)             ZVT_S_WORD(ZVT_ENC_VTLE64((tss), (addr)))
#define ZVT_S_VTSE8(tss, addr)              ZVT_S_WORD(ZVT_ENC_VTSE8((tss), (addr)))
#define ZVT_S_VTSE16(tss, addr)             ZVT_S_WORD(ZVT_ENC_VTSE16((tss), (addr)))
#define ZVT_S_VTSE32(tss, addr)             ZVT_S_WORD(ZVT_ENC_VTSE32((tss), (addr)))
#define ZVT_S_VTSE64(tss, addr)             ZVT_S_WORD(ZVT_ENC_VTSE64((tss), (addr)))
#define ZVT_S_VTMV_V_T(vd, tss)             ZVT_S_WORD(ZVT_ENC_VTMV_V_T((vd), (tss)))
#define ZVT_S_VTMV_T_V(tss, vs2)            ZVT_S_WORD(ZVT_ENC_VTMV_T_V((tss), (vs2)))
#define ZVT_S_VTFMM_TVV(mt, vs2, vs1)       ZVT_S_WORD(ZVT_ENC_VTFMM_TVV((mt), (vs2), (vs1)))
#define ZVT_S_VTFMM_ALT_TVV(mt, vs2, vs1)   ZVT_S_WORD(ZVT_ENC_VTFMM_ALT_TVV((mt), (vs2), (vs1)))
#define ZVT_S_VTMMU_TVV(mt, vs2, vs1)       ZVT_S_WORD(ZVT_ENC_VTMMU_TVV((mt), (vs2), (vs1)))
#define ZVT_S_VTMMS_TVV(mt, vs2, vs1)       ZVT_S_WORD(ZVT_ENC_VTMMS_TVV((mt), (vs2), (vs1)))
#define ZVT_S_VTZERO(mt)                     ZVT_S_WORD(ZVT_ENC_VTZERO((mt)))
#define ZVT_S_VTDISCARD                      ZVT_S_WORD(ZVT_ENC_VTDISCARD)

#else /* C/C++ */

#define ZVT_C_ASM_WORD(name)                 ".4byte %[" #name "]\n"
#define ZVT_C_ASM_IMM(name, enc)             [name] "i" ((uint32_t)(enc))
#define ZVT_C_EMIT(enc) \
    __asm__ volatile (".4byte %0" : : "i" ((uint32_t)(enc)) : "memory")

#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__

#include <stdint.h>

#if !defined(__riscv)
/* Encoders remain usable for host-side tests/tools; execution helpers do not. */
#else

/* Compiler-register-aware helpers for instructions whose variable operands are
 * all scalar GPRs. They use the generic .insn R-format so GCC/Clang can allocate
 * the GPR operands safely even if the assembler has no Zvt mnemonic table.
 *
 * Matrix arithmetic and tile/vector moves intentionally do NOT have function-like
 * C wrappers here because the compiler has no knowledge of the new tile state and
 * may not be able to allocate the required vector register groups correctly.
 * Use ZVT_ASM_* inside a single inline-asm block, or a .S microkernel.
 */

static inline void zvt_msetmtype(uintptr_t mtype_value, uintptr_t vtype_value)
{
    __asm__ volatile (".insn r 0x57, 7, 0x42, x0, %0, %1"
                      : : "r"(mtype_value), "r"(vtype_value) : "memory");
}

static inline uintptr_t zvt_msettn(uintptr_t requested_tn)
{
    uintptr_t actual;
    __asm__ volatile (".insn r 0x57, 7, 0x44, %0, %1, x0"
                      : "=r"(actual) : "r"(requested_tn) : "memory");
    return actual;
}

static inline uintptr_t zvt_msettm(uintptr_t requested_tm)
{
    uintptr_t actual;
    /* rs2 field encodes sub-op 1; it is not a semantic x1 dependency. */
    __asm__ volatile (".insn r 0x57, 7, 0x44, %0, %1, x1"
                      : "=r"(actual) : "r"(requested_tm) : "memory");
    return actual;
}

static inline uintptr_t zvt_msettk(uintptr_t requested_tk)
{
    uintptr_t actual;
    /* rs2 field encodes sub-op 2; it is not a semantic x2 dependency. */
    __asm__ volatile (".insn r 0x57, 7, 0x44, %0, %1, x2"
                      : "=r"(actual) : "r"(requested_tk) : "memory");
    return actual;
}

/* Direct tile-subset <-> memory helpers. rs2 contains the runtime TSS value. */
#define ZVT_DEFINE_TILE_LOAD_FN(bits, f7) \
static inline void zvt_vtle##bits(uintptr_t tss, const void *addr) \
{ \
    __asm__ volatile (".insn r 0x07, 7, " ZVT_STR(f7) ", x0, %0, %1" \
                      : : "r"(addr), "r"(tss) : "memory"); \
}

#define ZVT_DEFINE_TILE_STORE_FN(bits, f7) \
static inline void zvt_vtse##bits(uintptr_t tss, void *addr) \
{ \
    __asm__ volatile (".insn r 0x27, 7, " ZVT_STR(f7) ", x0, %0, %1" \
                      : : "r"(addr), "r"(tss) : "memory"); \
}

ZVT_DEFINE_TILE_LOAD_FN(8,  0x09)
ZVT_DEFINE_TILE_LOAD_FN(16, 0x19)
ZVT_DEFINE_TILE_LOAD_FN(32, 0x29)
ZVT_DEFINE_TILE_LOAD_FN(64, 0x39)
ZVT_DEFINE_TILE_STORE_FN(8,  0x09)
ZVT_DEFINE_TILE_STORE_FN(16, 0x19)
ZVT_DEFINE_TILE_STORE_FN(32, 0x29)
ZVT_DEFINE_TILE_STORE_FN(64, 0x39)

#undef ZVT_DEFINE_TILE_LOAD_FN
#undef ZVT_DEFINE_TILE_STORE_FN

#endif /* __riscv */
#endif /* !__ASSEMBLER__ */

#endif /* ZVT_ENCODING_H */
