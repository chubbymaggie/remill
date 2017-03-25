/* Copyright 2017 Peter Goodman (peter@trailofbits.com), all rights reserved. */

#ifndef REMILL_ARCH_X86_SEMANTICS_SSE_H_
#define REMILL_ARCH_X86_SEMANTICS_SSE_H_

namespace {

enum FloatCompareOperator {
  kEqOrderedQuiet,
  kLtOrderedSignal,
  kLeOrderedSignal,
  kUnorderedQuiet,
  kNeUnorderedQuiet,
  kNltUnorderedSignal,
  kNleUnorderedSignal,
  kOrderedQuiet,
  kEqUnorderedQuiet,
  kNgeUnorderedSignal,
  kNgtUnorderedSignal,
  kFalseOrderedQuiet,
  kNeOrderedQuiet,
  kGeOrderedSignal,
  kGtOrderedSignal,
  kTrueUnorderedQuiet,
  kEqOrderedSignal,
  kLtOrderedQuiet,
  kLeOrderedQuiet,
  kUnorderedSignal,
  kNeUnorderedSignal,
  kNltUnorderedQuiet,
  kNleUnorderedQuiet,
  kOrderedSignal,
  kEqUnorderedSignal,
  kNgeUnorderedQuiet,
  kNgtUnorderedQuiet,
  kFalseOrderedSignal,
  kNeOrderedSignal,
  kGeOrderedQuiet,
  kGtOrderedQuiet,
  kTrueUnorderedSignal,
};

template <typename T>
ALWAYS_INLINE static bool CompareFloats(FloatCompareOperator op, T v1, T v2) {
  auto is_unordered = __builtin_isunordered(v1, v2);
  auto is_ordered = !is_unordered;
  switch (op) {
    case kEqOrderedQuiet:
      return !__builtin_islessgreater(v1, v2) && is_ordered;
    case kLtOrderedSignal:
      return v1 < v2 && is_ordered;
    case kLeOrderedSignal:
      return v1 <= v2 && is_ordered;
    case kUnorderedQuiet:
      return is_unordered;
    case kNeUnorderedQuiet:
      return __builtin_islessgreater(v1, v2) || is_unordered;
    case kNltUnorderedSignal:
      return !(v1 < v2) || is_unordered;
    case kNleUnorderedSignal:
      return !(v1 <= v2) || is_unordered;
    case kOrderedQuiet:
      return is_ordered;
    case kEqUnorderedQuiet:
      return !__builtin_islessgreater(v1, v2);
    case kNgeUnorderedSignal:
      return !(v1 >= v2) || is_unordered;
    case kNgtUnorderedSignal:
      return !(v1 > v2) || is_unordered;
    case kFalseOrderedQuiet:
      return false;
    case kNeOrderedQuiet:
      return __builtin_islessgreater(v1, v2);
    case kGeOrderedSignal:
      return v1 >= v2 && is_ordered;
    case kGtOrderedSignal:
      return v1 > v2 && is_ordered;
    case kTrueUnorderedQuiet:
      return true;
    case kEqOrderedSignal:
      return v1 == v2 && is_ordered;
    case kLtOrderedQuiet:
      return __builtin_isless(v1, v2);
    case kLeOrderedQuiet:
      return __builtin_islessequal(v1, v2);
    case kUnorderedSignal:
      return is_unordered;
    case kNeUnorderedSignal:
      return v1 != v2 || is_unordered;
    case kNltUnorderedQuiet:
      return !__builtin_isless(v1, v2);
    case kNleUnorderedQuiet:
      return !__builtin_islessequal(v1, v2);
    case kOrderedSignal:
      return is_ordered;
    case kEqUnorderedSignal:
      return v1 == v2 || is_unordered;
    case kNgeUnorderedQuiet:
      return !__builtin_isgreaterequal(v1, v2);
    case kNgtUnorderedQuiet:
      return !__builtin_isgreater(v1, v2);
    case kFalseOrderedSignal:
      return false;
    case kNeOrderedSignal:
      return !(v1 == v2) && is_ordered;
    case kGeOrderedQuiet:
      return __builtin_isgreaterequal(v1, v2);
    case kGtOrderedQuiet:
      return __builtin_isgreater(v1, v2);
    case kTrueUnorderedSignal:
      return true;
  }
}

#ifndef issignaling

union nan32_t {
  float32_t f;
  struct {
    uint32_t mantissa:22;
    uint32_t is_quiet_nan:1;
    uint32_t exponent:8;
    uint32_t is_negative:1;
  } __attribute__((packed));
} __attribute__((packed));

static_assert(sizeof(float32_t) == sizeof(nan32_t),
              "Invalid packing of `nan32_t`.");

union nan64_t {
  float64_t d;
  struct {
    uint64_t mantissa_low:32;
    uint64_t mantissa_high:19;
    uint64_t is_quiet_nan:1;
    uint64_t exponent:11;
    uint64_t is_negative:1;
  } __attribute__((packed));
} __attribute__((packed));

static_assert(sizeof(float64_t) == sizeof(nan64_t),
              "Invalid packing of `nan64_t`.");

ALWAYS_INLINE bool issignaling(float32_t x) {
  if (!std::isnan(x)) {
    return false;
  }

  nan32_t x_nan = {x};
  return !x_nan.is_quiet_nan;
}

ALWAYS_INLINE bool issignaling(float64_t x) {
  if (!std::isnan(x)) {
    return false;
  }

  nan64_t x_nan = {x};
  return !x_nan.is_quiet_nan;
}

#endif

template <typename S1, typename S2>
DEF_SEM(COMISS, S1 src1, S2 src2) {
  auto left = FExtractV32(FReadV32(src1), 0);
  auto right = FExtractV32(FReadV32(src2), 0);

  if (__builtin_isunordered(left, right)) {
    if (issignaling(left + right)) {
      StopFailure();
    }

    Write(FLAG_ZF, true);
    Write(FLAG_PF, true);
    Write(FLAG_CF, true);

  } else if (FCmpGt(left, right)) {
    Write(FLAG_ZF, false);
    Write(FLAG_PF, false);
    Write(FLAG_CF, false);
  } else if (FCmpLt(left, right)) {
    Write(FLAG_ZF, false);
    Write(FLAG_PF, false);
    Write(FLAG_CF, true);

  } else if (FCmpEq(left, right)) {
    Write(FLAG_ZF, true);
    Write(FLAG_PF, false);
    Write(FLAG_CF, false);
  }

  Write(FLAG_OF, false);
  Write(FLAG_SF, false);
  Write(FLAG_AF, false);
  return memory;
}

template <typename S1, typename S2>
DEF_SEM(COMISD, S1 src1, S2 src2) {
  auto left = FExtractV64(FReadV64(src1), 0);
  auto right = FExtractV64(FReadV64(src2), 0);

  if (__builtin_isunordered(left, right)) {
    if (issignaling(left + right)) {
      StopFailure();
    }

    Write(FLAG_ZF, true);
    Write(FLAG_PF, true);
    Write(FLAG_CF, true);

  } else if (FCmpGt(left, right)) {
    Write(FLAG_ZF, false);
    Write(FLAG_PF, false);
    Write(FLAG_CF, false);
  } else if (FCmpLt(left, right)) {
    Write(FLAG_ZF, false);
    Write(FLAG_PF, false);
    Write(FLAG_CF, true);

  } else if (FCmpEq(left, right)) {
    Write(FLAG_ZF, true);
    Write(FLAG_PF, false);
    Write(FLAG_CF, false);

  }
  Write(FLAG_OF, false);
  Write(FLAG_SF, false);
  Write(FLAG_AF, false);
  return memory;
}

}  // namespace

DEF_ISEL(COMISD_XMMsd_MEMsd) = COMISD<V128, MV64>;
DEF_ISEL(COMISD_XMMsd_XMMsd) = COMISD<V128, V128>;
DEF_ISEL(COMISS_XMMss_MEMss) = COMISS<V128, MV32>;
DEF_ISEL(COMISS_XMMss_XMMss) = COMISS<V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VCOMISD_XMMq_MEMq) = COMISD<V128, MV64>;
DEF_ISEL(VCOMISD_XMMq_XMMq) = COMISD<V128, V128>;
DEF_ISEL(VCOMISS_XMMd_MEMd) = COMISS<V128, MV32>;
DEF_ISEL(VCOMISS_XMMd_XMMd) = COMISS<V128, V128>;
#endif  // HAS_FEATURE_AVX

DEF_ISEL(UCOMISD_XMMsd_MEMsd) = COMISD<V128, MV64>;
DEF_ISEL(UCOMISD_XMMsd_XMMsd) = COMISD<V128, V128>;
DEF_ISEL(UCOMISS_XMMss_MEMss) = COMISS<V128, MV32>;
DEF_ISEL(UCOMISS_XMMss_XMMss) = COMISS<V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VUCOMISD_XMMdq_MEMq) = COMISD<V128, MV64>;
DEF_ISEL(VUCOMISD_XMMdq_XMMq) = COMISD<V128, V128>;
DEF_ISEL(VUCOMISS_XMMdq_MEMd) = COMISS<V128, MV32>;
DEF_ISEL(VUCOMISS_XMMdq_XMMd) = COMISS<V128, V128>;
#endif  // HAS_FEATURE_AVX


/*

4290 VCOMISD VCOMISD_XMMf64_XMMf64_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
4291 VCOMISD VCOMISD_XMMf64_XMMf64_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
4292 VCOMISD VCOMISD_XMMf64_MEMf64_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: DISP8_SCALAR MXCSR SIMD_SCALAR
4293 VCOMISS VCOMISS_XMMf32_XMMf32_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
4294 VCOMISS VCOMISS_XMMf32_XMMf32_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
4295 VCOMISS VCOMISS_XMMf32_MEMf32_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: DISP8_SCALAR MXCSR SIMD_SCALAR
5396 VUCOMISS VUCOMISS_XMMf32_XMMf32_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
5397 VUCOMISS VUCOMISS_XMMf32_XMMf32_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
5398 VUCOMISS VUCOMISS_XMMf32_MEMf32_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: DISP8_SCALAR MXCSR SIMD_SCALAR
5430 VUCOMISD VUCOMISD_XMMf64_XMMf64_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
5431 VUCOMISD VUCOMISD_XMMf64_XMMf64_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MXCSR SIMD_SCALAR
5432 VUCOMISD VUCOMISD_XMMf64_MEMf64_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: DISP8_SCALAR MXCSR SIMD_SCALAR

 */

namespace {

template <typename D, typename S1>
DEF_SEM(PSHUFD, D dst, S1 src1, I8 src2) {
  auto dst_vec = UClearV32(UReadV32(src1));
  auto src_vec = UReadV128(src1);
  auto num_groups = NumVectorElems(src_vec);

  _Pragma("unroll")
  for (std::size_t i = 0, k = 0; i < num_groups; ++i) {
    auto group = UExtractV128(src_vec, i);
    auto order = Read(src2);

    _Pragma("unroll")
    for (std::size_t j = 0; j < 4; ++j, ++k) {
      auto sel = UAnd(order, 0x3_u8);
      auto shift = UMul(sel, 32_u8);
      order = UShr(order, 2_u8);
      auto sel_val = UShr(group, UInt128(shift));
      dst_vec = UInsertV32(dst_vec, k, TruncTo<uint32_t>(sel_val));
    }
  }
  UWriteV32(dst, dst_vec);
  return memory;
}

}  // namespace

DEF_ISEL(PSHUFD_XMMdq_MEMdq_IMMb) = PSHUFD<V128W, MV128>;
DEF_ISEL(PSHUFD_XMMdq_XMMdq_IMMb) = PSHUFD<V128W, V128>;
IF_AVX(DEF_ISEL(VPSHUFD_XMMdq_MEMdq_IMMb) = PSHUFD<V128W, MV128>;)
IF_AVX(DEF_ISEL(VPSHUFD_XMMdq_XMMdq_IMMb) = PSHUFD<V128W, V128>;)
IF_AVX(DEF_ISEL(VPSHUFD_YMMqq_MEMqq_IMMb) = PSHUFD<VV256W, MV256>;)
IF_AVX(DEF_ISEL(VPSHUFD_YMMqq_YMMqq_IMMb) = PSHUFD<VV256W, V256>;)

/*
4319 VPSHUFD VPSHUFD_ZMMu32_MASKmskw_ZMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
4320 VPSHUFD VPSHUFD_ZMMu32_MASKmskw_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX
4321 VPSHUFD VPSHUFD_XMMu32_MASKmskw_XMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
4322 VPSHUFD VPSHUFD_XMMu32_MASKmskw_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX
4323 VPSHUFD VPSHUFD_YMMu32_MASKmskw_YMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
4324 VPSHUFD VPSHUFD_YMMu32_MASKmskw_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX
 */

namespace {

#define MAKE_PCMP(suffix, size, op) \
    template <typename D, typename S1, typename S2> \
    DEF_SEM(PCMP ## suffix, D dst, S1 src1, S2 src2) { \
      auto src1_vec = SReadV ## size(src1); \
      auto src2_vec = SReadV ## size(src2); \
      auto dst_vec = SClearV ## size(SReadV ## size(dst)); \
      auto num_elems = NumVectorElems(src1_vec); \
      _Pragma("unroll") \
      for (std::size_t i = 0; i < num_elems; ++i) { \
        auto src1_elem = SExtractV ## size(src1_vec, i); \
        auto src2_elem = SExtractV ## size(src2_vec, i); \
        auto res = Select<int ## size ## _t>( \
            op(src1_elem, src2_elem), -1_s ## size, 0_s ## size); \
        dst_vec = SInsertV ## size(dst_vec, i, res); \
      } \
      SWriteV ## size(dst, dst_vec); \
      return memory; \
    }

MAKE_PCMP(GTQ, 64, SCmpGt)
MAKE_PCMP(GTW, 16, SCmpGt)
MAKE_PCMP(GTB, 8, SCmpGt)
MAKE_PCMP(GTD, 32, SCmpGt)

MAKE_PCMP(EQQ, 64, SCmpEq)
MAKE_PCMP(EQW, 16, SCmpEq)
MAKE_PCMP(EQB, 8, SCmpEq)
MAKE_PCMP(EQD, 32, SCmpEq)
}  // namespace

DEF_ISEL(PCMPGTB_MMXq_MMXq) = PCMPGTB<V64W, V64, V64>;
DEF_ISEL(PCMPGTB_MMXq_MEMq) = PCMPGTB<V64W, V64, MV64>;
DEF_ISEL(PCMPGTW_MMXq_MMXq) = PCMPGTW<V64W, V64, V64>;
DEF_ISEL(PCMPGTW_MMXq_MEMq) = PCMPGTW<V64W, V64, MV64>;
DEF_ISEL(PCMPGTD_MMXq_MMXq) = PCMPGTD<V64W, V64, V64>;
DEF_ISEL(PCMPGTD_MMXq_MEMq) = PCMPGTD<V64W, V64, MV64>;

DEF_ISEL(PCMPGTQ_XMMdq_MEMdq) = PCMPGTQ<V128W, V128, MV128>;
DEF_ISEL(PCMPGTQ_XMMdq_XMMdq) = PCMPGTQ<V128W, V128, V128>;
DEF_ISEL(PCMPGTW_XMMdq_MEMdq) = PCMPGTW<V128W, V128, MV128>;
DEF_ISEL(PCMPGTW_XMMdq_XMMdq) = PCMPGTW<V128W, V128, V128>;
DEF_ISEL(PCMPGTB_XMMdq_MEMdq) = PCMPGTB<V128W, V128, MV128>;
DEF_ISEL(PCMPGTB_XMMdq_XMMdq) = PCMPGTB<V128W, V128, V128>;
DEF_ISEL(PCMPGTD_XMMdq_MEMdq) = PCMPGTD<V128W, V128, MV128>;
DEF_ISEL(PCMPGTD_XMMdq_XMMdq) = PCMPGTD<V128W, V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VPCMPGTQ_XMMdq_XMMdq_MEMdq) = PCMPGTQ<VV128W, V128, MV128>;
DEF_ISEL(VPCMPGTQ_XMMdq_XMMdq_XMMdq) = PCMPGTQ<VV128W, V128, V128>;
DEF_ISEL(VPCMPGTQ_YMMqq_YMMqq_MEMqq) = PCMPGTQ<VV256W, V256, MV256>;
DEF_ISEL(VPCMPGTQ_YMMqq_YMMqq_YMMqq) = PCMPGTQ<VV256W, V256, V256>;
DEF_ISEL(VPCMPGTW_XMMdq_XMMdq_MEMdq) = PCMPGTW<VV128W, V128, MV128>;
DEF_ISEL(VPCMPGTW_XMMdq_XMMdq_XMMdq) = PCMPGTW<VV128W, V128, V128>;
DEF_ISEL(VPCMPGTW_YMMqq_YMMqq_MEMqq) = PCMPGTW<VV256W, V256, MV256>;
DEF_ISEL(VPCMPGTW_YMMqq_YMMqq_YMMqq) = PCMPGTW<VV256W, V256, V256>;
DEF_ISEL(VPCMPGTB_XMMdq_XMMdq_MEMdq) = PCMPGTB<VV128W, V128, MV128>;
DEF_ISEL(VPCMPGTB_XMMdq_XMMdq_XMMdq) = PCMPGTB<VV128W, V128, V128>;
DEF_ISEL(VPCMPGTB_YMMqq_YMMqq_MEMqq) = PCMPGTB<VV256W, V256, MV256>;
DEF_ISEL(VPCMPGTB_YMMqq_YMMqq_YMMqq) = PCMPGTB<VV256W, V256, V256>;
DEF_ISEL(VPCMPGTD_XMMdq_XMMdq_MEMdq) = PCMPGTD<VV128W, V128, MV128>;
DEF_ISEL(VPCMPGTD_XMMdq_XMMdq_XMMdq) = PCMPGTD<VV128W, V128, V128>;
DEF_ISEL(VPCMPGTD_YMMqq_YMMqq_MEMqq) = PCMPGTD<VV256W, V256, MV256>;
DEF_ISEL(VPCMPGTD_YMMqq_YMMqq_YMMqq) = PCMPGTD<VV256W, V256, V256>;
#endif  // HAS_FEATURE_AVX

DEF_ISEL(PCMPEQB_MMXq_MMXq) = PCMPEQB<V64W, V64, V64>;
DEF_ISEL(PCMPEQB_MMXq_MEMq) = PCMPEQB<V64W, V64, MV64>;
DEF_ISEL(PCMPEQW_MMXq_MMXq) = PCMPEQW<V64W, V64, V64>;
DEF_ISEL(PCMPEQW_MMXq_MEMq) = PCMPEQW<V64W, V64, MV64>;
DEF_ISEL(PCMPEQD_MMXq_MMXq) = PCMPEQD<V64W, V64, V64>;
DEF_ISEL(PCMPEQD_MMXq_MEMq) = PCMPEQD<V64W, V64, MV64>;

DEF_ISEL(PCMPEQQ_XMMdq_MEMdq) = PCMPEQQ<V128W, V128, MV128>;
DEF_ISEL(PCMPEQQ_XMMdq_XMMdq) = PCMPEQQ<V128W, V128, V128>;
DEF_ISEL(PCMPEQW_XMMdq_MEMdq) = PCMPEQW<V128W, V128, MV128>;
DEF_ISEL(PCMPEQW_XMMdq_XMMdq) = PCMPEQW<V128W, V128, V128>;
DEF_ISEL(PCMPEQB_XMMdq_MEMdq) = PCMPEQB<V128W, V128, MV128>;
DEF_ISEL(PCMPEQB_XMMdq_XMMdq) = PCMPEQB<V128W, V128, V128>;
DEF_ISEL(PCMPEQD_XMMdq_MEMdq) = PCMPEQD<V128W, V128, MV128>;
DEF_ISEL(PCMPEQD_XMMdq_XMMdq) = PCMPEQD<V128W, V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VPCMPEQQ_XMMdq_XMMdq_MEMdq) = PCMPEQQ<VV128W, V128, MV128>;
DEF_ISEL(VPCMPEQQ_XMMdq_XMMdq_XMMdq) = PCMPEQQ<VV128W, V128, V128>;
DEF_ISEL(VPCMPEQQ_YMMqq_YMMqq_MEMqq) = PCMPEQQ<VV256W, V256, MV256>;
DEF_ISEL(VPCMPEQQ_YMMqq_YMMqq_YMMqq) = PCMPEQQ<VV256W, V256, V256>;
DEF_ISEL(VPCMPEQW_XMMdq_XMMdq_MEMdq) = PCMPEQW<VV128W, V128, MV128>;
DEF_ISEL(VPCMPEQW_XMMdq_XMMdq_XMMdq) = PCMPEQW<VV128W, V128, V128>;
DEF_ISEL(VPCMPEQW_YMMqq_YMMqq_MEMqq) = PCMPEQW<VV256W, V256, MV256>;
DEF_ISEL(VPCMPEQW_YMMqq_YMMqq_YMMqq) = PCMPEQW<VV256W, V256, V256>;
DEF_ISEL(VPCMPEQB_XMMdq_XMMdq_MEMdq) = PCMPEQB<VV128W, V128, MV128>;
DEF_ISEL(VPCMPEQB_XMMdq_XMMdq_XMMdq) = PCMPEQB<VV128W, V128, V128>;
DEF_ISEL(VPCMPEQB_YMMqq_YMMqq_MEMqq) = PCMPEQB<VV256W, V256, MV256>;
DEF_ISEL(VPCMPEQB_YMMqq_YMMqq_YMMqq) = PCMPEQB<VV256W, V256, V256>;
DEF_ISEL(VPCMPEQD_XMMdq_XMMdq_MEMdq) = PCMPEQD<VV128W, V128, MV128>;
DEF_ISEL(VPCMPEQD_XMMdq_XMMdq_XMMdq) = PCMPEQD<VV128W, V128, V128>;
DEF_ISEL(VPCMPEQD_YMMqq_YMMqq_MEMqq) = PCMPEQD<VV256W, V256, MV256>;
DEF_ISEL(VPCMPEQD_YMMqq_YMMqq_YMMqq) = PCMPEQD<VV256W, V256, V256>;
#endif  // HAS_FEATURE_AVX

namespace {

template <typename D, typename S1, typename S2>
DEF_SEM(CMPSS, D dst, S1 src1, S2 src2, I8 src3) {
  auto src1_vec = FReadV32(src1);
  auto src2_vec = FReadV32(src2);
  auto dst_vec = UClearV32(UReadV32(dst));
  auto op = Read(src3);
  if (op >= 32) {
    StopFailure();
  }
  auto v1 = FExtractV32(src1_vec, 0);
  auto v2 = FExtractV32(src2_vec, 0);
  bool cond = CompareFloats<float32_t>(
      static_cast<FloatCompareOperator>(op), v1, v2);

  dst_vec = UInsertV32(dst_vec, 0, Select<uint32_t>(cond, ~0_u32, 0_u32));

  UWriteV32(dst, dst_vec);
  return memory;
}

template <typename D, typename S1, typename S2>
DEF_SEM(CMPSD, D dst, S1 src1, S2 src2, I8 src3) {
  auto src1_vec = FReadV64(src1);
  auto src2_vec = FReadV64(src2);
  auto dst_vec = UClearV64(UReadV64(dst));
  auto op = Read(src3);
  if (op >= 32) {
    StopFailure();
  }
  auto v1 = FExtractV64(src1_vec, 0);
  auto v2 = FExtractV64(src2_vec, 0);
  bool cond = CompareFloats<float64_t>(
      static_cast<FloatCompareOperator>(op), v1, v2);

  dst_vec = UInsertV64(dst_vec, 0, Select<uint64_t>(cond, ~0_u64, 0_u64));

  UWriteV64(dst, dst_vec);
  return memory;
}


}  // namespace

DEF_ISEL(CMPSS_XMMss_MEMss_IMMb) = CMPSS<V128W, V128, MV32>;
DEF_ISEL(CMPSS_XMMss_XMMss_IMMb) = CMPSS<V128W, V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VCMPSS_XMMdq_XMMdq_MEMd_IMMb) = CMPSS<VV128W, V128, MV32>;
DEF_ISEL(VCMPSS_XMMdq_XMMdq_XMMd_IMMb) = CMPSS<VV128W, V128, V128>;
#endif  // HAS_FEATURE_AVX

DEF_ISEL(CMPSD_XMM_XMMsd_MEMsd_IMMb) = CMPSD<V128W, V128, MV64>;
DEF_ISEL(CMPSD_XMM_XMMsd_XMMsd_IMMb) = CMPSD<V128W, V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VCMPSD_XMMdq_XMMdq_MEMq_IMMb) = CMPSD<VV128W, V128, MV64>;
DEF_ISEL(VCMPSD_XMMdq_XMMdq_XMMq_IMMb) = CMPSD<VV128W, V128, V128>;
#endif  // HAS_FEATURE_AVX

namespace {
template <typename D, typename S1, typename S2>
DEF_SEM(CMPPS, D dst, S1 src1, S2 src2, I8 src3) {
  auto src1_vec = FReadV32(src1);
  auto src2_vec = FReadV32(src2);
  auto dst_vec = UClearV32(UReadV32(dst));
  auto op = Read(src3);
  if (op >= 32) {
    StopFailure();
  }

  auto vec_count = NumVectorElems(src2_vec);
  _Pragma("unroll")
  for (std::size_t i = 0; i < vec_count; i++) {
    auto v1 = FExtractV32(src1_vec, i);
    auto v2 = FExtractV32(src2_vec, i);

    bool cond = CompareFloats<float32_t>(
        static_cast<FloatCompareOperator>(op), v1, v2);

    auto res = Select<uint32_t>(cond, ~0_u32, 0_u32);
    dst_vec = UInsertV32(dst_vec, i, res);
  }
  UWriteV32(dst, dst_vec);
  return memory;
}

template <typename D, typename S1, typename S2>
DEF_SEM(CMPPD, D dst, S1 src1, S2 src2, I8 src3) {
  auto src1_vec = FReadV64(src1);
  auto src2_vec = FReadV64(src2);
  auto dst_vec = UClearV64(UReadV64(dst));
  auto op = Read(src3);
  if (op >= 32) {
    StopFailure();
  }

  auto vec_count = NumVectorElems(src2_vec);
  _Pragma("unroll")
  for (std::size_t i = 0; i < vec_count; i++) {
    auto v1 = FExtractV64(src1_vec, i);
    auto v2 = FExtractV64(src2_vec, i);

    bool cond = CompareFloats<float64_t>(
        static_cast<FloatCompareOperator>(op), v1, v2);

    auto res = Select<uint64_t>(cond, ~0_u64, 0_u64);
    dst_vec = UInsertV64(dst_vec, i, res);
  }
  UWriteV64(dst, dst_vec);
  return memory;
}

}  // namespace

DEF_ISEL(CMPPS_XMMps_MEMps_IMMb) = CMPPS<V128W, V128, MV128>;
DEF_ISEL(CMPPS_XMMps_XMMps_IMMb) = CMPPS<V128W, V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VCMPPS_XMMdq_XMMdq_MEMdq_IMMb) = CMPPS<VV128W, V128, MV128>;
DEF_ISEL(VCMPPS_XMMdq_XMMdq_XMMdq_IMMb) = CMPPS<VV128W, V128, V128>;
DEF_ISEL(VCMPPS_YMMqq_YMMqq_MEMqq_IMMb) = CMPPS<VV256W, V256, MV256>;
DEF_ISEL(VCMPPS_YMMqq_YMMqq_YMMqq_IMMb) = CMPPS<VV256W, V256, V256>;
#endif  // HAS_FEATURE_AVX

DEF_ISEL(CMPPD_XMMpd_MEMpd_IMMb) = CMPPD<V128W, V128, MV128>;
DEF_ISEL(CMPPD_XMMpd_XMMpd_IMMb) = CMPPD<V128W, V128, V128>;

#if HAS_FEATURE_AVX
DEF_ISEL(VCMPPD_XMMdq_XMMdq_MEMdq_IMMb) = CMPPD<VV128W, V128, MV128>;
DEF_ISEL(VCMPPD_XMMdq_XMMdq_XMMdq_IMMb) = CMPPD<VV128W, V128, V128>;
DEF_ISEL(VCMPPD_YMMqq_YMMqq_MEMqq_IMMb) = CMPPD<VV256W, V256, MV256>;
DEF_ISEL(VCMPPD_YMMqq_YMMqq_YMMqq_IMMb) = CMPPD<VV256W, V256, V256>;
#endif  // HAS_FEATURE_AVX

namespace {

enum InputFormat : uint8_t {
  kUInt8 = 0,
  kUInt16 = 1,
  kInt8 = 2,
  kInt16 = 3
};

enum AggregationOperation : uint8_t {
  kEqualAny = 0,
  kRanges = 1,
  kEqualEach = 2,
  kEqualOrdered = 3
};

enum Polarity : uint8_t {
  kPositive = 0,
  kNegative = 1,
  kMaskedPositive = 2,
  kMaskedNegative = 3
};

enum OutputSelection : uint8_t {
  kLeastSignificantIndex = 0,
  kMostSignificantIndex = 1
};

union StringCompareControl {
  uint8_t flat;
  struct {
    uint8_t input_format:2;
    uint8_t agg_operation:2;
    uint8_t polarity:2;
    uint8_t output_selection:1;
    uint8_t should_be_0:1;
  } __attribute__((packed));
} __attribute__((packed));

static_assert(1 == sizeof(StringCompareControl),
              "Invalid packing of `StringCompareControl`.");

template <size_t x, size_t y>
class BitMatrix : std::bitset<x * y> {
 public:
  ALWAYS_INLINE bool Test(size_t i, size_t j) const {
    return this->operator[]((x * i) + j);
  }

  ALWAYS_INLINE void Set(size_t i, size_t j, bool val) {
    this->operator[]((x * i) + j) = val;
  }

 private:
  bool rows[x][y];
};

// src1 is a char set, src2 is a string. We set a bit of `int_res_1` to `1`
// when a char in `src2` belongs to the char set `src1`.
template <size_t num_elems>
ALWAYS_INLINE static uint16_t AggregateEqualAny(
    const BitMatrix<num_elems, num_elems> &bool_res,
    const size_t src1_len, const size_t src2_len) {

  uint16_t int_res_1 = 0;
  uint16_t bit = 1;
  for (size_t j = 0; j < src2_len; ++j, bit <<= 1) {

    _Pragma("unroll")
    for (size_t i = 0; i < src1_len; ++i) {
      if (bool_res.Test(i, j)) {
        int_res_1 |= bit;
        break;  // src2_j is in src1, at position src1_i.
      }
    }
  }
  return int_res_1;
}

// `src2` is a string, and `src1` is kind of like a the ranges of regular
// expression character classes.
template <size_t num_elems>
ALWAYS_INLINE static uint16_t AggregateRanges(
    const BitMatrix<num_elems, num_elems> &bool_res,
    const size_t src1_len, const size_t src2_len) {

  uint16_t int_res_1 = 0;
  uint16_t bit = 1;

  for (size_t j = 0; j < src2_len; ++j, bit <<= 1)  {

    _Pragma("unroll")
    for (size_t i = 0; i < (src1_len - 1); i += 2) {
      const auto geq_lower_bound = bool_res.Test(i, j);
      const auto leq_upper_bound = bool_res.Test(i + 1, j);
      if (geq_lower_bound && leq_upper_bound) {
        int_res_1 |= bit;  // src2_j is in the range [src1_i, src1_i+1]
        break;
      }
    }
  }
  return int_res_1;
}

template <size_t num_elems>
ALWAYS_INLINE static uint16_t AggregateEqualEach(
    const BitMatrix<num_elems, num_elems> &bool_res,
    const size_t src1_len, const size_t src2_len) {

  uint16_t int_res_1 = 0;
  uint16_t bit = 1;

  _Pragma("unroll")
  for (size_t i = 0; i < num_elems; ++i, bit <<= 1) {
    const bool in_str1 = i < src1_len;
    const bool in_str2 = i < src2_len;
    if (in_str1 && in_str2) {
      if (bool_res.Test(i, i)) {
        int_res_1 |= bit;
      }
    } else if (!in_str1 && !in_str2) {
      int_res_1 |= bit;
    }
  }
  return int_res_1;
}

// This is really `strstr`, i.e. searching for `src1` in `src2`.
template <size_t num_elems>
ALWAYS_INLINE static uint16_t AggregateEqualOrdered(
    const BitMatrix<num_elems, num_elems> &bool_res,
    const size_t src1_len, const size_t src2_len) {

  if (src1_len > src2_len) {
    return 0;
  }

  uint16_t int_res_1 = (0xFFFF_u16 >> (16 - num_elems));
  uint16_t bit = 1;

  for (size_t j = 0; j < num_elems; ++j, bit <<= 1) {

    _Pragma("unroll")
    for (size_t i = 0, k = j; i < (num_elems - j) && k < num_elems; ++i, ++k) {
      auto needle_valid = i < src1_len;
      auto haystack_valid = k < src2_len;

      if (!needle_valid) {
        break;
      } else if (!haystack_valid || !bool_res.Test(i, k)) {
        int_res_1 ^= bit;
        break;
      }
    }
  }

  return int_res_1;
}

template <typename V, size_t num_elems>
DEF_SEM(DoPCMPISTRI, const V &src1, const V &src2,
        StringCompareControl control) {
  BitMatrix<num_elems, num_elems> bool_res;
  size_t src1_len = num_elems;
  size_t src2_len = num_elems;

  const auto agg_operation = static_cast<AggregationOperation>(
      control.agg_operation);

  const auto polarity = static_cast<Polarity>(control.polarity);
  const auto output_selection = static_cast<OutputSelection>(
      control.output_selection);

  _Pragma("unroll")
  for (size_t i = 0; i < num_elems; ++i) {
    if (!src1.elems[i]) {
      src1_len = std::min<size_t>(src1_len, i);
    }
    if (!src2.elems[i]) {
      src2_len = std::min<size_t>(src2_len, i);
    }
  }

  for (size_t n = 0; n < num_elems; ++n) {
    const auto reg = src1.elems[n];

    _Pragma("unroll")
    for (size_t m = 0; m < num_elems; ++m) {
      const auto reg_mem = src2.elems[m];

      switch (agg_operation) {
        case kEqualAny:
        case kEqualEach:
        case kEqualOrdered:
          bool_res.Set(n, m, reg == reg_mem);
          break;

        // Checking is `src2[m]` is in the range of `src1[n]` and `src1[n+1]`.
        case kRanges:
          if (n & 1U) {  // Odd.
            bool_res.Set(n, m, reg_mem <= reg);  // `z` and `Z` in `azAZ`.
          } else {  // Even.
            bool_res.Set(n, m, reg_mem >= reg);  // `a` and `A` in `azAZ`.
          }
          break;
      }
    }
  }

  uint16_t int_res_1 = 0;

  switch (agg_operation) {
    case kEqualAny:
      int_res_1 = AggregateEqualAny<num_elems>(bool_res, src1_len, src2_len);
      break;
    case kRanges:
      int_res_1 = AggregateRanges<num_elems>(bool_res, src1_len, src2_len);
      break;
    case kEqualEach:
      int_res_1 = AggregateEqualEach<num_elems>(bool_res, src1_len, src2_len);
      break;
    case kEqualOrdered:
      int_res_1 = AggregateEqualOrdered<num_elems>(
          bool_res, src1_len, src2_len);
      break;
  }

  uint16_t int_res_2 = 0;
  switch (polarity) {
    case kPositive:
      int_res_2 = int_res_1;
      break;
    case kNegative:
      int_res_2 = (0xFFFF_u16 >> (16 - num_elems)) ^ int_res_1;
      break;
    case kMaskedPositive:
      int_res_2 = int_res_1;
      break;
    case kMaskedNegative:
      int_res_2 = int_res_1;
      _Pragma("unroll")
      for (size_t i = 0; i < num_elems; ++i) {
        auto mask = static_cast<uint16_t>(1_u16 << i);
        if (i < src2_len) {
          int_res_2 ^= mask;
        }
      }
      break;
  }

  uint16_t index = num_elems;
  switch (output_selection) {
    case kLeastSignificantIndex:
      if (auto lsb_index = __builtin_ffs(int_res_2)) {
        index = static_cast<uint16_t>(lsb_index - 1);
      }
      break;
    case kMostSignificantIndex:
      if (int_res_2) {
        uint16_t count = CountLeadingZeros(int_res_2) - (16_u16 - num_elems);
        index = num_elems - count - 1;
      }
      break;
  }

  Write(REG_XCX, static_cast<addr_t>(index));
  Write(FLAG_CF, int_res_2 != 0_u16);
  Write(FLAG_ZF, src2_len < num_elems);
  Write(FLAG_SF, src1_len < num_elems);
  Write(FLAG_OF, 0_u16 != (int_res_2 & 1_u16));
  Write(FLAG_AF, false);
  Write(FLAG_PF, false);
  return memory;
}

template <typename S2>
DEF_SEM(PCMPISTRI, V128 src1, S2 src2, I8 src3) {
  const StringCompareControl control = {.flat = Read(src3)};
  switch (static_cast<InputFormat>(control.input_format)) {
    case kUInt8:
      return DoPCMPISTRI<uint8v16_t, 16>(memory, state, UReadV8(src1),
                                  UReadV8(src2), control);
    case kUInt16:
      return DoPCMPISTRI<uint16v8_t, 8>(memory, state, UReadV16(src1),
                                 UReadV16(src2), control);
    case kInt8:
      return DoPCMPISTRI<int8v16_t, 16>(memory, state, SReadV8(src1),
                                 SReadV8(src2), control);
    case kInt16:
      return DoPCMPISTRI<int16v8_t, 8>(memory, state, SReadV16(src1),
                                SReadV16(src2), control);
  }
  return memory;
}

}  // namespace

DEF_ISEL(PCMPISTRI_XMMdq_XMMdq_IMMb) = PCMPISTRI<V128>;
DEF_ISEL(PCMPISTRI_XMMdq_MEMdq_IMMb) = PCMPISTRI<MV128>;
IF_AVX(DEF_ISEL(VPCMPISTRI_XMMdq_XMMdq_IMMb) = PCMPISTRI<V128>;)
IF_AVX(DEF_ISEL(VPCMPISTRI_XMMdq_MEMdq_IMMb) = PCMPISTRI<MV128>;)

namespace {

template <typename D, typename S>
DEF_SEM(PSRLDQ, D dst, S src1, I8 src2) {
  auto vec = UReadV8(src1);
  auto new_vec = UClearV8(UReadV8(dst));
  auto shift = std::min<size_t>(Read(src2), 16);
  _Pragma("unroll")
  for (size_t i = shift, j = 0; i < 16; ++i, ++j) {
    new_vec = UInsertV8(new_vec, j, UExtractV8(vec, i));
  }
  UWriteV8(dst, new_vec);
  return memory;
}

#if 1 || HAS_FEATURE_AVX

template <typename D, typename S>
DEF_SEM(VPSRLDQ, D dst, S src1, I8 src2) {
  auto vec = UReadV8(src1);
  auto new_vec = UClearV8(UReadV8(dst));
  auto shift = std::min<size_t>(Read(src2), 16);
  _Pragma("unroll")
  for (size_t i = shift, j = 0; i < 16; ++i, ++j) {
    new_vec = UInsertV8(new_vec, j, UExtractV8(vec, i));
  }

  _Pragma("unroll")
  for (size_t i = shift, j = 0; i < 16; ++i, ++j) {
    new_vec = UInsertV8(new_vec, j + 16, UExtractV8(vec, i + 16));
  }

  UWriteV8(dst, new_vec);
  return memory;
}

#endif  // HAS_FEATURE_AVX

}  // namespace

DEF_ISEL(PSRLDQ_XMMdq_IMMb) = PSRLDQ<V128W, V128>;
IF_AVX(DEF_ISEL(VPSRLDQ_XMMdq_XMMdq_IMMb) = PSRLDQ<VV128W, V128>;)
IF_AVX(DEF_ISEL(VPSRLDQ_YMMqq_YMMqq_IMMb) = VPSRLDQ<VV256W, V256>;)

/*

1291 PSRLQ PSRLQ_MMXq_IMMb MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1292 PSRLQ PSRLQ_XMMdq_IMMb SSE SSE2 SSE2 ATTRIBUTES:
1293 PSRLQ PSRLQ_MMXq_MEMq MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1294 PSRLQ PSRLQ_MMXq_MMXq MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1295 PSRLQ PSRLQ_XMMdq_MEMdq SSE SSE2 SSE2 ATTRIBUTES: REQUIRES_ALIGNMENT SIMD_PACKED_ALIGNMENT
1296 PSRLQ PSRLQ_XMMdq_XMMdq SSE SSE2 SSE2 ATTRIBUTES: REQUIRES_ALIGNMENT SIMD_PACKED_ALIGNMENT
1297 PSRLW PSRLW_MMXq_IMMb MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1298 PSRLW PSRLW_XMMdq_IMMb SSE SSE2 SSE2 ATTRIBUTES:
1299 PSRLW PSRLW_MMXq_MEMq MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1300 PSRLW PSRLW_MMXq_MMXq MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1301 PSRLW PSRLW_XMMdq_MEMdq SSE SSE2 SSE2 ATTRIBUTES: REQUIRES_ALIGNMENT SIMD_PACKED_ALIGNMENT
1302 PSRLW PSRLW_XMMdq_XMMdq SSE SSE2 SSE2 ATTRIBUTES: REQUIRES_ALIGNMENT SIMD_PACKED_ALIGNMENT
1310 PSRLD PSRLD_MMXq_IMMb MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1311 PSRLD PSRLD_XMMdq_IMMb SSE SSE2 SSE2 ATTRIBUTES:
1312 PSRLD PSRLD_MMXq_MEMq MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1313 PSRLD PSRLD_MMXq_MMXq MMX MMX PENTIUMMMX ATTRIBUTES: NOTSX
1314 PSRLD PSRLD_XMMdq_MEMdq SSE SSE2 SSE2 ATTRIBUTES: REQUIRES_ALIGNMENT SIMD_PACKED_ALIGNMENT
1315 PSRLD PSRLD_XMMdq_XMMdq SSE SSE2 SSE2 ATTRIBUTES: REQUIRES_ALIGNMENT SIMD_PACKED_ALIGNMENT
2266 VPSRLVQ VPSRLVQ_XMMdq_XMMdq_MEMdq AVX2 AVX2 AVX2 ATTRIBUTES:
2267 VPSRLVQ VPSRLVQ_XMMdq_XMMdq_XMMdq AVX2 AVX2 AVX2 ATTRIBUTES:
2268 VPSRLVQ VPSRLVQ_YMMqq_YMMqq_MEMqq AVX2 AVX2 AVX2 ATTRIBUTES:
2269 VPSRLVQ VPSRLVQ_YMMqq_YMMqq_YMMqq AVX2 AVX2 AVX2 ATTRIBUTES:
2289 VPSRLVD VPSRLVD_XMMdq_XMMdq_MEMdq AVX2 AVX2 AVX2 ATTRIBUTES:
2290 VPSRLVD VPSRLVD_XMMdq_XMMdq_XMMdq AVX2 AVX2 AVX2 ATTRIBUTES:
2291 VPSRLVD VPSRLVD_YMMqq_YMMqq_MEMqq AVX2 AVX2 AVX2 ATTRIBUTES:
2292 VPSRLVD VPSRLVD_YMMqq_YMMqq_YMMqq AVX2 AVX2 AVX2 ATTRIBUTES:
3161 VPSRLW VPSRLW_XMMdq_XMMdq_MEMdq AVX AVX AVX ATTRIBUTES:
3162 VPSRLW VPSRLW_XMMdq_XMMdq_XMMdq AVX AVX AVX ATTRIBUTES:
3163 VPSRLW VPSRLW_XMMdq_XMMdq_IMMb AVX AVX AVX ATTRIBUTES:
3164 VPSRLW VPSRLW_YMMqq_YMMqq_MEMdq AVX2 AVX2 AVX2 ATTRIBUTES:
3165 VPSRLW VPSRLW_YMMqq_YMMqq_XMMq AVX2 AVX2 AVX2 ATTRIBUTES:
3166 VPSRLW VPSRLW_YMMqq_YMMqq_IMMb AVX2 AVX2 AVX2 ATTRIBUTES:
3167 VPSRLQ VPSRLQ_XMMdq_XMMdq_MEMdq AVX AVX AVX ATTRIBUTES:
3168 VPSRLQ VPSRLQ_XMMdq_XMMdq_XMMdq AVX AVX AVX ATTRIBUTES:
3169 VPSRLQ VPSRLQ_XMMdq_XMMdq_IMMb AVX AVX AVX ATTRIBUTES:
3170 VPSRLQ VPSRLQ_YMMqq_YMMqq_MEMdq AVX2 AVX2 AVX2 ATTRIBUTES:
3171 VPSRLQ VPSRLQ_YMMqq_YMMqq_XMMq AVX2 AVX2 AVX2 ATTRIBUTES:
3172 VPSRLQ VPSRLQ_YMMqq_YMMqq_IMMb AVX2 AVX2 AVX2 ATTRIBUTES:
3195 VPSRLD VPSRLD_XMMdq_XMMdq_MEMdq AVX AVX AVX ATTRIBUTES:
3196 VPSRLD VPSRLD_XMMdq_XMMdq_XMMdq AVX AVX AVX ATTRIBUTES:
3197 VPSRLD VPSRLD_XMMdq_XMMdq_IMMb AVX AVX AVX ATTRIBUTES:
3198 VPSRLD VPSRLD_YMMqq_YMMqq_MEMdq AVX2 AVX2 AVX2 ATTRIBUTES:
3199 VPSRLD VPSRLD_YMMqq_YMMqq_XMMq AVX2 AVX2 AVX2 ATTRIBUTES:
3200 VPSRLD VPSRLD_YMMqq_YMMqq_IMMb AVX2 AVX2 AVX2 ATTRIBUTES:
3992 VPSRLVQ VPSRLVQ_ZMMu64_MASKmskw_ZMMu64_ZMMu64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
3993 VPSRLVQ VPSRLVQ_ZMMu64_MASKmskw_ZMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3994 VPSRLVQ VPSRLVQ_XMMu64_MASKmskw_XMMu64_XMMu64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
3995 VPSRLVQ VPSRLVQ_XMMu64_MASKmskw_XMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3996 VPSRLVQ VPSRLVQ_YMMu64_MASKmskw_YMMu64_YMMu64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
3997 VPSRLVQ VPSRLVQ_YMMu64_MASKmskw_YMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4004 VPSRLVW VPSRLVW_XMMu16_MASKmskw_XMMu16_XMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
4005 VPSRLVW VPSRLVW_XMMu16_MASKmskw_XMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4006 VPSRLVW VPSRLVW_YMMu16_MASKmskw_YMMu16_YMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
4007 VPSRLVW VPSRLVW_YMMu16_MASKmskw_YMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4008 VPSRLVW VPSRLVW_ZMMu16_MASKmskw_ZMMu16_ZMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
4009 VPSRLVW VPSRLVW_ZMMu16_MASKmskw_ZMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4063 VPSRLVD VPSRLVD_ZMMu32_MASKmskw_ZMMu32_ZMMu32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
4064 VPSRLVD VPSRLVD_ZMMu32_MASKmskw_ZMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4065 VPSRLVD VPSRLVD_XMMu32_MASKmskw_XMMu32_XMMu32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
4066 VPSRLVD VPSRLVD_XMMu32_MASKmskw_XMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4067 VPSRLVD VPSRLVD_YMMu32_MASKmskw_YMMu32_YMMu32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
4068 VPSRLVD VPSRLVD_YMMu32_MASKmskw_YMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4093 VPSRLDQ VPSRLDQ_XMMu8_XMMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES:
4094 VPSRLDQ VPSRLDQ_XMMu8_MEMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM
4095 VPSRLDQ VPSRLDQ_YMMu8_YMMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES:
4096 VPSRLDQ VPSRLDQ_YMMu8_MEMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM
4097 VPSRLDQ VPSRLDQ_ZMMu8_ZMMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES:
4098 VPSRLDQ VPSRLDQ_ZMMu8_MEMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM
5561 VPSRLW VPSRLW_XMMu16_MASKmskw_XMMu16_XMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
5562 VPSRLW VPSRLW_XMMu16_MASKmskw_XMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5563 VPSRLW VPSRLW_XMMu16_MASKmskw_XMMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
5564 VPSRLW VPSRLW_XMMu16_MASKmskw_MEMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5565 VPSRLW VPSRLW_YMMu16_MASKmskw_YMMu16_XMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
5566 VPSRLW VPSRLW_YMMu16_MASKmskw_YMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5567 VPSRLW VPSRLW_YMMu16_MASKmskw_YMMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
5568 VPSRLW VPSRLW_YMMu16_MASKmskw_MEMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5569 VPSRLW VPSRLW_ZMMu16_MASKmskw_ZMMu16_XMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
5570 VPSRLW VPSRLW_ZMMu16_MASKmskw_ZMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5571 VPSRLW VPSRLW_ZMMu16_MASKmskw_ZMMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
5572 VPSRLW VPSRLW_ZMMu16_MASKmskw_MEMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5573 VPSRLQ VPSRLQ_ZMMu64_MASKmskw_ZMMu64_XMMu64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5574 VPSRLQ VPSRLQ_ZMMu64_MASKmskw_ZMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5575 VPSRLQ VPSRLQ_ZMMu64_MASKmskw_ZMMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5576 VPSRLQ VPSRLQ_ZMMu64_MASKmskw_MEMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5577 VPSRLQ VPSRLQ_XMMu64_MASKmskw_XMMu64_XMMu64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5578 VPSRLQ VPSRLQ_XMMu64_MASKmskw_XMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5579 VPSRLQ VPSRLQ_XMMu64_MASKmskw_XMMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5580 VPSRLQ VPSRLQ_XMMu64_MASKmskw_MEMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5581 VPSRLQ VPSRLQ_YMMu64_MASKmskw_YMMu64_XMMu64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5582 VPSRLQ VPSRLQ_YMMu64_MASKmskw_YMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5583 VPSRLQ VPSRLQ_YMMu64_MASKmskw_YMMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5584 VPSRLQ VPSRLQ_YMMu64_MASKmskw_MEMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5615 VPSRLD VPSRLD_ZMMu32_MASKmskw_ZMMu32_XMMu32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5616 VPSRLD VPSRLD_ZMMu32_MASKmskw_ZMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5617 VPSRLD VPSRLD_ZMMu32_MASKmskw_ZMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5618 VPSRLD VPSRLD_ZMMu32_MASKmskw_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5619 VPSRLD VPSRLD_XMMu32_MASKmskw_XMMu32_XMMu32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5620 VPSRLD VPSRLD_XMMu32_MASKmskw_XMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5621 VPSRLD VPSRLD_XMMu32_MASKmskw_XMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5622 VPSRLD VPSRLD_XMMu32_MASKmskw_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5623 VPSRLD VPSRLD_YMMu32_MASKmskw_YMMu32_XMMu32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5624 VPSRLD VPSRLD_YMMu32_MASKmskw_YMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: DISP8_MEM128 MASKOP_EVEX
5625 VPSRLD VPSRLD_YMMu32_MASKmskw_YMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5626 VPSRLD VPSRLD_YMMu32_MASKmskw_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION

288 PCMPISTRM PCMPISTRM_XMMdq_MEMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
289 PCMPISTRM PCMPISTRM_XMMdq_XMMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1044 PCMPESTRI PCMPESTRI_XMMdq_MEMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1045 PCMPESTRI PCMPESTRI_XMMdq_XMMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1046 PCMPESTRI PCMPESTRI_XMMdq_MEMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1047 PCMPESTRI PCMPESTRI_XMMdq_XMMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1056 PCMPESTRM PCMPESTRM_XMMdq_MEMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1057 PCMPESTRM PCMPESTRM_XMMdq_XMMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1058 PCMPESTRM PCMPESTRM_XMMdq_MEMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED
1059 PCMPESTRM PCMPESTRM_XMMdq_XMMdq_IMMb SSE SSE4 SSE42 ATTRIBUTES: UNALIGNED

2974 VPCMPESTRI VPCMPESTRI_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2975 VPCMPESTRI VPCMPESTRI_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2976 VPCMPESTRI VPCMPESTRI_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2977 VPCMPESTRI VPCMPESTRI_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2978 VPCMPESTRI VPCMPESTRI_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2979 VPCMPESTRI VPCMPESTRI_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2980 VPCMPESTRM VPCMPESTRM_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2981 VPCMPESTRM VPCMPESTRM_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2982 VPCMPESTRM VPCMPESTRM_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2983 VPCMPESTRM VPCMPESTRM_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2984 VPCMPESTRM VPCMPESTRM_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
2985 VPCMPESTRM VPCMPESTRM_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:
3103 VPCMPISTRM VPCMPISTRM_XMMdq_MEMdq_IMMb STTNI AVX AVX ATTRIBUTES:
3104 VPCMPISTRM VPCMPISTRM_XMMdq_XMMdq_IMMb STTNI AVX AVX ATTRIBUTES:

3925 VPCMPEQQ VPCMPEQQ_MASKmskw_MASKmskw_ZMMu64_ZMMu64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
3926 VPCMPEQQ VPCMPEQQ_MASKmskw_MASKmskw_ZMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3927 VPCMPEQQ VPCMPEQQ_MASKmskw_MASKmskw_XMMu64_XMMu64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
3928 VPCMPEQQ VPCMPEQQ_MASKmskw_MASKmskw_XMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3929 VPCMPEQQ VPCMPEQQ_MASKmskw_MASKmskw_YMMu64_YMMu64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
3930 VPCMPEQQ VPCMPEQQ_MASKmskw_MASKmskw_YMMu64_MEMu64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3943 VPCMPEQD VPCMPEQD_MASKmskw_MASKmskw_ZMMu32_ZMMu32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
3944 VPCMPEQD VPCMPEQD_MASKmskw_MASKmskw_ZMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3945 VPCMPEQD VPCMPEQD_MASKmskw_MASKmskw_XMMu32_XMMu32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
3946 VPCMPEQD VPCMPEQD_MASKmskw_MASKmskw_XMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3947 VPCMPEQD VPCMPEQD_MASKmskw_MASKmskw_YMMu32_YMMu32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
3948 VPCMPEQD VPCMPEQD_MASKmskw_MASKmskw_YMMu32_MEMu32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3956 VPCMPEQB VPCMPEQB_MASKmskw_MASKmskw_XMMu8_XMMu8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
3957 VPCMPEQB VPCMPEQB_MASKmskw_MASKmskw_XMMu8_MEMu8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3958 VPCMPEQB VPCMPEQB_MASKmskw_MASKmskw_YMMu8_YMMu8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
3959 VPCMPEQB VPCMPEQB_MASKmskw_MASKmskw_YMMu8_MEMu8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
3960 VPCMPEQB VPCMPEQB_MASKmskw_MASKmskw_ZMMu8_ZMMu8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
3961 VPCMPEQB VPCMPEQB_MASKmskw_MASKmskw_ZMMu8_MEMu8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4362 VPCMPGTB VPCMPGTB_MASKmskw_MASKmskw_XMMu8_XMMu8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
4363 VPCMPGTB VPCMPGTB_MASKmskw_MASKmskw_XMMu8_MEMu8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4364 VPCMPGTB VPCMPGTB_MASKmskw_MASKmskw_YMMu8_YMMu8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
4365 VPCMPGTB VPCMPGTB_MASKmskw_MASKmskw_YMMu8_MEMu8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4366 VPCMPGTB VPCMPGTB_MASKmskw_MASKmskw_ZMMu8_ZMMu8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
4367 VPCMPGTB VPCMPGTB_MASKmskw_MASKmskw_ZMMu8_MEMu8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4388 VCMPSS VCMPSS_MASKmskw_MASKmskw_XMMf32_XMMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MASKOP_EVEX MXCSR SIMD_SCALAR
4389 VCMPSS VCMPSS_MASKmskw_MASKmskw_XMMf32_XMMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MASKOP_EVEX MXCSR SIMD_SCALAR
4390 VCMPSS VCMPSS_MASKmskw_MASKmskw_XMMf32_MEMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: DISP8_SCALAR MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR SIMD_SCALAR
4407 VCMPSD VCMPSD_MASKmskw_MASKmskw_XMMf64_XMMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MASKOP_EVEX MXCSR SIMD_SCALAR
4408 VCMPSD VCMPSD_MASKmskw_MASKmskw_XMMf64_XMMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: MASKOP_EVEX MXCSR SIMD_SCALAR
4409 VCMPSD VCMPSD_MASKmskw_MASKmskw_XMMf64_MEMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_SCALAR ATTRIBUTES: DISP8_SCALAR MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR SIMD_SCALAR
4808 VPCMPUW VPCMPUW_MASKmskw_MASKmskw_XMMu16_XMMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
4809 VPCMPUW VPCMPUW_MASKmskw_MASKmskw_XMMu16_MEMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4810 VPCMPUW VPCMPUW_MASKmskw_MASKmskw_YMMu16_YMMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
4811 VPCMPUW VPCMPUW_MASKmskw_MASKmskw_YMMu16_MEMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
4812 VPCMPUW VPCMPUW_MASKmskw_MASKmskw_ZMMu16_ZMMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
4813 VPCMPUW VPCMPUW_MASKmskw_MASKmskw_ZMMu16_MEMu16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5041 VCMPPS VCMPPS_MASKmskw_MASKmskw_ZMMf32_ZMMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX MXCSR
5042 VCMPPS VCMPPS_MASKmskw_MASKmskw_ZMMf32_ZMMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX MXCSR
5043 VCMPPS VCMPPS_MASKmskw_MASKmskw_ZMMf32_MEMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR
5044 VCMPPS VCMPPS_MASKmskw_MASKmskw_XMMf32_XMMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX MXCSR
5045 VCMPPS VCMPPS_MASKmskw_MASKmskw_XMMf32_MEMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR
5046 VCMPPS VCMPPS_MASKmskw_MASKmskw_YMMf32_YMMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX MXCSR
5047 VCMPPS VCMPPS_MASKmskw_MASKmskw_YMMf32_MEMf32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR
5048 VCMPPD VCMPPD_MASKmskw_MASKmskw_ZMMf64_ZMMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX MXCSR
5049 VCMPPD VCMPPD_MASKmskw_MASKmskw_ZMMf64_ZMMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX MXCSR
5050 VCMPPD VCMPPD_MASKmskw_MASKmskw_ZMMf64_MEMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR
5051 VCMPPD VCMPPD_MASKmskw_MASKmskw_XMMf64_XMMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX MXCSR
5052 VCMPPD VCMPPD_MASKmskw_MASKmskw_XMMf64_MEMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR
5053 VCMPPD VCMPPD_MASKmskw_MASKmskw_YMMf64_YMMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX MXCSR
5054 VCMPPD VCMPPD_MASKmskw_MASKmskw_YMMf64_MEMf64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION MXCSR
5111 VPCMPW VPCMPW_MASKmskw_MASKmskw_XMMi16_XMMi16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
5112 VPCMPW VPCMPW_MASKmskw_MASKmskw_XMMi16_MEMi16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5113 VPCMPW VPCMPW_MASKmskw_MASKmskw_YMMi16_YMMi16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
5114 VPCMPW VPCMPW_MASKmskw_MASKmskw_YMMi16_MEMi16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5115 VPCMPW VPCMPW_MASKmskw_MASKmskw_ZMMi16_ZMMi16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
5116 VPCMPW VPCMPW_MASKmskw_MASKmskw_ZMMi16_MEMi16_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5117 VPCMPQ VPCMPQ_MASKmskw_MASKmskw_ZMMi64_ZMMi64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5118 VPCMPQ VPCMPQ_MASKmskw_MASKmskw_ZMMi64_MEMi64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5119 VPCMPQ VPCMPQ_MASKmskw_MASKmskw_XMMi64_XMMi64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5120 VPCMPQ VPCMPQ_MASKmskw_MASKmskw_XMMi64_MEMi64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5121 VPCMPQ VPCMPQ_MASKmskw_MASKmskw_YMMi64_YMMi64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5122 VPCMPQ VPCMPQ_MASKmskw_MASKmskw_YMMi64_MEMi64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5135 VPCMPD VPCMPD_MASKmskw_MASKmskw_ZMMi32_ZMMi32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5136 VPCMPD VPCMPD_MASKmskw_MASKmskw_ZMMi32_MEMi32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5137 VPCMPD VPCMPD_MASKmskw_MASKmskw_XMMi32_XMMi32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5138 VPCMPD VPCMPD_MASKmskw_MASKmskw_XMMi32_MEMi32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5139 VPCMPD VPCMPD_MASKmskw_MASKmskw_YMMi32_YMMi32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5140 VPCMPD VPCMPD_MASKmskw_MASKmskw_YMMi32_MEMi32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5141 VPCMPB VPCMPB_MASKmskw_MASKmskw_XMMi8_XMMi8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
5142 VPCMPB VPCMPB_MASKmskw_MASKmskw_XMMi8_MEMi8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5143 VPCMPB VPCMPB_MASKmskw_MASKmskw_YMMi8_YMMi8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
5144 VPCMPB VPCMPB_MASKmskw_MASKmskw_YMMi8_MEMi8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5145 VPCMPB VPCMPB_MASKmskw_MASKmskw_ZMMi8_ZMMi8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
5146 VPCMPB VPCMPB_MASKmskw_MASKmskw_ZMMi8_MEMi8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5222 VPCMPUQ VPCMPUQ_MASKmskw_MASKmskw_ZMMu64_ZMMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5223 VPCMPUQ VPCMPUQ_MASKmskw_MASKmskw_ZMMu64_MEMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5224 VPCMPUQ VPCMPUQ_MASKmskw_MASKmskw_XMMu64_XMMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5225 VPCMPUQ VPCMPUQ_MASKmskw_MASKmskw_XMMu64_MEMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5226 VPCMPUQ VPCMPUQ_MASKmskw_MASKmskw_YMMu64_YMMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5227 VPCMPUQ VPCMPUQ_MASKmskw_MASKmskw_YMMu64_MEMu64_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5243 VPCMPUB VPCMPUB_MASKmskw_MASKmskw_XMMu8_XMMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
5244 VPCMPUB VPCMPUB_MASKmskw_MASKmskw_XMMu8_MEMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5245 VPCMPUB VPCMPUB_MASKmskw_MASKmskw_YMMu8_YMMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
5246 VPCMPUB VPCMPUB_MASKmskw_MASKmskw_YMMu8_MEMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5247 VPCMPUB VPCMPUB_MASKmskw_MASKmskw_ZMMu8_ZMMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
5248 VPCMPUB VPCMPUB_MASKmskw_MASKmskw_ZMMu8_MEMu8_IMM8_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5258 VPCMPUD VPCMPUD_MASKmskw_MASKmskw_ZMMu32_ZMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
5259 VPCMPUD VPCMPUD_MASKmskw_MASKmskw_ZMMu32_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5260 VPCMPUD VPCMPUD_MASKmskw_MASKmskw_XMMu32_XMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
5261 VPCMPUD VPCMPUD_MASKmskw_MASKmskw_XMMu32_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5262 VPCMPUD VPCMPUD_MASKmskw_MASKmskw_YMMu32_YMMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
5263 VPCMPUD VPCMPUD_MASKmskw_MASKmskw_YMMu32_MEMu32_IMM8_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5677 VPCMPEQW VPCMPEQW_MASKmskw_MASKmskw_XMMu16_XMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
5678 VPCMPEQW VPCMPEQW_MASKmskw_MASKmskw_XMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5679 VPCMPEQW VPCMPEQW_MASKmskw_MASKmskw_YMMu16_YMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
5680 VPCMPEQW VPCMPEQW_MASKmskw_MASKmskw_YMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
5681 VPCMPEQW VPCMPEQW_MASKmskw_MASKmskw_ZMMu16_ZMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
5682 VPCMPEQW VPCMPEQW_MASKmskw_MASKmskw_ZMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6222 VPCMPGTD VPCMPGTD_MASKmskw_MASKmskw_ZMMi32_ZMMi32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
6223 VPCMPGTD VPCMPGTD_MASKmskw_MASKmskw_ZMMi32_MEMi32_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6224 VPCMPGTD VPCMPGTD_MASKmskw_MASKmskw_XMMi32_XMMi32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
6225 VPCMPGTD VPCMPGTD_MASKmskw_MASKmskw_XMMi32_MEMi32_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6226 VPCMPGTD VPCMPGTD_MASKmskw_MASKmskw_YMMi32_YMMi32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
6227 VPCMPGTD VPCMPGTD_MASKmskw_MASKmskw_YMMi32_MEMi32_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6237 VPCMPGTQ VPCMPGTQ_MASKmskw_MASKmskw_ZMMi64_ZMMi64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: MASKOP_EVEX
6238 VPCMPGTQ VPCMPGTQ_MASKmskw_MASKmskw_ZMMi64_MEMi64_AVX512 AVX512 AVX512EVEX AVX512F_512 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6239 VPCMPGTQ VPCMPGTQ_MASKmskw_MASKmskw_XMMi64_XMMi64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: MASKOP_EVEX
6240 VPCMPGTQ VPCMPGTQ_MASKmskw_MASKmskw_XMMi64_MEMi64_AVX512 AVX512 AVX512EVEX AVX512F_128 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6241 VPCMPGTQ VPCMPGTQ_MASKmskw_MASKmskw_YMMi64_YMMi64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: MASKOP_EVEX
6242 VPCMPGTQ VPCMPGTQ_MASKmskw_MASKmskw_YMMi64_MEMi64_AVX512 AVX512 AVX512EVEX AVX512F_256 ATTRIBUTES: BROADCAST_ENABLED DISP8_FULL MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6253 VPCMPGTW VPCMPGTW_MASKmskw_MASKmskw_XMMu16_XMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: MASKOP_EVEX
6254 VPCMPGTW VPCMPGTW_MASKmskw_MASKmskw_XMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_128 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6255 VPCMPGTW VPCMPGTW_MASKmskw_MASKmskw_YMMu16_YMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: MASKOP_EVEX
6256 VPCMPGTW VPCMPGTW_MASKmskw_MASKmskw_YMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_256 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
6257 VPCMPGTW VPCMPGTW_MASKmskw_MASKmskw_ZMMu16_ZMMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: MASKOP_EVEX
6258 VPCMPGTW VPCMPGTW_MASKmskw_MASKmskw_ZMMu16_MEMu16_AVX512 AVX512 AVX512EVEX AVX512BW_512 ATTRIBUTES: DISP8_FULLMEM MASKOP_EVEX MEMORY_FAULT_SUPPRESSION
 */

#endif  // REMILL_ARCH_X86_SEMANTICS_SSE_H_
