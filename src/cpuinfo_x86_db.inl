// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#if defined(FEATURE)
// FEATURE(ENUM, NAME, CPUINFO_FLAG, HWCAP, HWCAP2)
// clang-format off
FEATURE(X86_FPU, fpu, "fpu", 0, 0)
FEATURE(X86_TSC, tsc, "tsc", 0, 0)
FEATURE(X86_CX8, cx8, "cx8", 0, 0)
FEATURE(X86_CLFSH, clfsh, "clfsh", 0, 0)
FEATURE(X86_MMX, mmx, "mmx", 0, 0)
FEATURE(X86_AES, aes, "aes", 0, 0)
FEATURE(X86_ERMS, erms, "erms", 0, 0)
FEATURE(X86_F16C, f16c, "f16c", 0, 0)
FEATURE(X86_FMA4, fma4, "fma4", 0, 0)
FEATURE(X86_FMA3, fma3, "fma3", 0, 0)
FEATURE(X86_VAES, vaes, "vaes", 0, 0)
FEATURE(X86_VPCLMULQDQ, vpclmulqdq, "vpclmulqdq", 0, 0)
FEATURE(X86_BMI1, bmi1, "bmi1", 0, 0)
FEATURE(X86_HLE, hle, "hle", 0, 0)
FEATURE(X86_BMI2, bmi2, "bmi2", 0, 0)
FEATURE(X86_RTM, rtm, "rtm", 0, 0)
FEATURE(X86_RDSEED, rdseed, "rdseed", 0, 0)
FEATURE(X86_CLFLUSHOPT, clflushopt, "clflushopt", 0, 0)
FEATURE(X86_CLWB, clwb, "clwb", 0, 0)
FEATURE(X86_SSE, sse, "sse", 0, 0)
FEATURE(X86_SSE2, sse2, "sse2", 0, 0)
FEATURE(X86_SSE3, sse3, "sse3", 0, 0)
FEATURE(X86_SSSE3, ssse3, "ssse3", 0, 0)
FEATURE(X86_SSE4_1, sse4_1, "sse4_1", 0, 0)
FEATURE(X86_SSE4_2, sse4_2, "sse4_2", 0, 0)
FEATURE(X86_SSE4A, sse4a, "sse4a", 0, 0)
FEATURE(X86_AVX, avx, "avx", 0, 0)
FEATURE(X86_AVX2, avx2, "avx2", 0, 0)
FEATURE(X86_AVX512F, avx512f, "avx512f", 0, 0)
FEATURE(X86_AVX512CD, avx512cd, "avx512cd", 0, 0)
FEATURE(X86_AVX512ER, avx512er, "avx512er", 0, 0)
FEATURE(X86_AVX512PF, avx512pf, "avx512pf", 0, 0)
FEATURE(X86_AVX512BW, avx512bw, "avx512bw", 0, 0)
FEATURE(X86_AVX512DQ, avx512dq, "avx512dq", 0, 0)
FEATURE(X86_AVX512VL, avx512vl, "avx512vl", 0, 0)
FEATURE(X86_AVX512IFMA, avx512ifma, "avx512ifma", 0, 0)
FEATURE(X86_AVX512VBMI, avx512vbmi, "avx512vbmi", 0, 0)
FEATURE(X86_AVX512VBMI2, avx512vbmi2, "avx512vbmi2", 0, 0)
FEATURE(X86_AVX512VNNI, avx512vnni, "avx512vnni", 0, 0)
FEATURE(X86_AVX512BITALG, avx512bitalg, "avx512bitalg", 0, 0)
FEATURE(X86_AVX512VPOPCNTDQ, avx512vpopcntdq, "avx512vpopcntdq", 0, 0)
FEATURE(X86_AVX512_4VNNIW, avx512_4vnniw, "avx512_4vnniw", 0, 0)
FEATURE(X86_AVX512_4VBMI2, avx512_4vbmi2, "avx512_4vbmi2", 0, 0)
FEATURE(X86_AVX512_SECOND_FMA, avx512_second_fma, "avx512_second_fma", 0, 0)
FEATURE(X86_AVX512_4FMAPS, avx512_4fmaps, "avx512_4fmaps", 0, 0)
FEATURE(X86_AVX512_BF16, avx512_bf16, "avx512_bf16", 0, 0)
FEATURE(X86_AVX512_VP2INTERSECT, avx512_vp2intersect, "avx512_vp2intersect", 0, 0)
FEATURE(X86_AMX_BF16, amx_bf16, "amx_bf16", 0, 0)
FEATURE(X86_AMX_TILE, amx_tile, "amx_tile", 0, 0)
FEATURE(X86_AMX_INT8, amx_int8, "amx_int8", 0, 0)
FEATURE(X86_PCLMULQDQ, pclmulqdq, "pclmulqdq", 0, 0)
FEATURE(X86_SMX, smx, "smx", 0, 0)
FEATURE(X86_SGX, sgx, "sgx", 0, 0)
FEATURE(X86_CX16, cx16, "cx16", 0, 0)
FEATURE(X86_SHA, sha, "sha", 0, 0)
FEATURE(X86_POPCNT, popcnt, "popcnt", 0, 0)
FEATURE(X86_MOVBE, movbe, "movbe", 0, 0)
FEATURE(X86_RDRND, rdrnd, "rdrnd", 0, 0)
FEATURE(X86_DCA, dca, "dca", 0, 0)
FEATURE(X86_SS, ss, "ss", 0, 0)
// clang-format on
#else
#error "The FEATURE macro must be defined"
#endif  // FEATURE
