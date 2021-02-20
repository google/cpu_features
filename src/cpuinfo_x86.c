// Copyright 2017 Google LLC
// Copyright 2020 Intel Corporation
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

#include "cpuinfo_x86.h"

#include <stdbool.h>
#include <string.h>

#include "internal/bit_utils.h"
#include "internal/cpuid_x86.h"

#if !defined(CPU_FEATURES_ARCH_X86)
#error "Cannot compile cpuinfo_x86 on a non x86 platform."
#endif

// Generation of feature's getters/setters functions and kGetters, kSetters,
// kCpuInfoFlags global tables.
#define DEFINE_TABLE_FEATURES                                                  \
  FEATURE(X86_FPU, fpu, "fpu", 0, 0)                                           \
  FEATURE(X86_TSC, tsc, "tsc", 0, 0)                                           \
  FEATURE(X86_CX8, cx8, "cx8", 0, 0)                                           \
  FEATURE(X86_CLFSH, clfsh, "clfsh", 0, 0)                                     \
  FEATURE(X86_MMX, mmx, "mmx", 0, 0)                                           \
  FEATURE(X86_AES, aes, "aes", 0, 0)                                           \
  FEATURE(X86_ERMS, erms, "erms", 0, 0)                                        \
  FEATURE(X86_F16C, f16c, "f16c", 0, 0)                                        \
  FEATURE(X86_FMA4, fma4, "fma4", 0, 0)                                        \
  FEATURE(X86_FMA3, fma3, "fma3", 0, 0)                                        \
  FEATURE(X86_VAES, vaes, "vaes", 0, 0)                                        \
  FEATURE(X86_VPCLMULQDQ, vpclmulqdq, "vpclmulqdq", 0, 0)                      \
  FEATURE(X86_BMI1, bmi1, "bmi1", 0, 0)                                        \
  FEATURE(X86_HLE, hle, "hle", 0, 0)                                           \
  FEATURE(X86_BMI2, bmi2, "bmi2", 0, 0)                                        \
  FEATURE(X86_RTM, rtm, "rtm", 0, 0)                                           \
  FEATURE(X86_RDSEED, rdseed, "rdseed", 0, 0)                                  \
  FEATURE(X86_CLFLUSHOPT, clflushopt, "clflushopt", 0, 0)                      \
  FEATURE(X86_CLWB, clwb, "clwb", 0, 0)                                        \
  FEATURE(X86_SSE, sse, "sse", 0, 0)                                           \
  FEATURE(X86_SSE2, sse2, "sse2", 0, 0)                                        \
  FEATURE(X86_SSE3, sse3, "sse3", 0, 0)                                        \
  FEATURE(X86_SSSE3, ssse3, "ssse3", 0, 0)                                     \
  FEATURE(X86_SSE4_1, sse4_1, "sse4_1", 0, 0)                                  \
  FEATURE(X86_SSE4_2, sse4_2, "sse4_2", 0, 0)                                  \
  FEATURE(X86_SSE4A, sse4a, "sse4a", 0, 0)                                     \
  FEATURE(X86_AVX, avx, "avx", 0, 0)                                           \
  FEATURE(X86_AVX2, avx2, "avx2", 0, 0)                                        \
  FEATURE(X86_AVX512F, avx512f, "avx512f", 0, 0)                               \
  FEATURE(X86_AVX512CD, avx512cd, "avx512cd", 0, 0)                            \
  FEATURE(X86_AVX512ER, avx512er, "avx512er", 0, 0)                            \
  FEATURE(X86_AVX512PF, avx512pf, "avx512pf", 0, 0)                            \
  FEATURE(X86_AVX512BW, avx512bw, "avx512bw", 0, 0)                            \
  FEATURE(X86_AVX512DQ, avx512dq, "avx512dq", 0, 0)                            \
  FEATURE(X86_AVX512VL, avx512vl, "avx512vl", 0, 0)                            \
  FEATURE(X86_AVX512IFMA, avx512ifma, "avx512ifma", 0, 0)                      \
  FEATURE(X86_AVX512VBMI, avx512vbmi, "avx512vbmi", 0, 0)                      \
  FEATURE(X86_AVX512VBMI2, avx512vbmi2, "avx512vbmi2", 0, 0)                   \
  FEATURE(X86_AVX512VNNI, avx512vnni, "avx512vnni", 0, 0)                      \
  FEATURE(X86_AVX512BITALG, avx512bitalg, "avx512bitalg", 0, 0)                \
  FEATURE(X86_AVX512VPOPCNTDQ, avx512vpopcntdq, "avx512vpopcntdq", 0, 0)       \
  FEATURE(X86_AVX512_4VNNIW, avx512_4vnniw, "avx512_4vnniw", 0, 0)             \
  FEATURE(X86_AVX512_4VBMI2, avx512_4vbmi2, "avx512_4vbmi2", 0, 0)             \
  FEATURE(X86_AVX512_SECOND_FMA, avx512_second_fma, "avx512_second_fma", 0, 0) \
  FEATURE(X86_AVX512_4FMAPS, avx512_4fmaps, "avx512_4fmaps", 0, 0)             \
  FEATURE(X86_AVX512_BF16, avx512_bf16, "avx512_bf16", 0, 0)                   \
  FEATURE(X86_AVX512_VP2INTERSECT, avx512_vp2intersect, "avx512_vp2intersect", \
          0, 0)                                                                \
  FEATURE(X86_AMX_BF16, amx_bf16, "amx_bf16", 0, 0)                            \
  FEATURE(X86_AMX_TILE, amx_tile, "amx_tile", 0, 0)                            \
  FEATURE(X86_AMX_INT8, amx_int8, "amx_int8", 0, 0)                            \
  FEATURE(X86_PCLMULQDQ, pclmulqdq, "pclmulqdq", 0, 0)                         \
  FEATURE(X86_SMX, smx, "smx", 0, 0)                                           \
  FEATURE(X86_SGX, sgx, "sgx", 0, 0)                                           \
  FEATURE(X86_CX16, cx16, "cx16", 0, 0)                                        \
  FEATURE(X86_SHA, sha, "sha", 0, 0)                                           \
  FEATURE(X86_POPCNT, popcnt, "popcnt", 0, 0)                                  \
  FEATURE(X86_MOVBE, movbe, "movbe", 0, 0)                                     \
  FEATURE(X86_RDRND, rdrnd, "rdrnd", 0, 0)                                     \
  FEATURE(X86_DCA, dca, "dca", 0, 0)                                           \
  FEATURE(X86_SS, ss, "ss", 0, 0)
#define DEFINE_TABLE_FEATURE_TYPE X86Features
#define DEFINE_TABLE_DONT_GENERATE_HWCAPS
#include "define_tables.h"

// The following includes are necessary to provide SSE detections on pre-AVX
// microarchitectures.
#if defined(CPU_FEATURES_OS_WINDOWS)
#include <windows.h>  // IsProcessorFeaturePresent
#elif defined(CPU_FEATURES_OS_LINUX_OR_ANDROID)
#include "internal/filesystem.h"         // Needed to parse /proc/cpuinfo
#include "internal/stack_line_reader.h"  // Needed to parse /proc/cpuinfo
#include "internal/string_view.h"        // Needed to parse /proc/cpuinfo
#elif defined(CPU_FEATURES_OS_DARWIN)
#if !defined(HAVE_SYSCTLBYNAME)
#error "Darwin needs support for sysctlbyname"
#endif
#include <sys/sysctl.h>
#else
#error "Unsupported OS"
#endif  // CPU_FEATURES_OS

////////////////////////////////////////////////////////////////////////////////
// Definitions for CpuId and GetXCR0Eax.
////////////////////////////////////////////////////////////////////////////////

#if defined(CPU_FEATURES_MOCK_CPUID_X86)
// Implementation will be provided by test/cpuinfo_x86_test.cc.
#elif defined(CPU_FEATURES_COMPILER_CLANG) || defined(CPU_FEATURES_COMPILER_GCC)

#include <cpuid.h>

Leaf GetCpuidLeaf(uint32_t leaf_id, int ecx) {
  Leaf leaf;
  __cpuid_count(leaf_id, ecx, leaf.eax, leaf.ebx, leaf.ecx, leaf.edx);
  return leaf;
}

uint32_t GetXCR0Eax(void) {
  uint32_t eax, edx;
  /* named form of xgetbv not supported on OSX, so must use byte form, see:
     https://github.com/asmjit/asmjit/issues/78
   */
  __asm(".byte 0x0F, 0x01, 0xd0" : "=a"(eax), "=d"(edx) : "c"(0));
  return eax;
}

#elif defined(CPU_FEATURES_COMPILER_MSC)

#include <immintrin.h>
#include <intrin.h>  // For __cpuidex()

Leaf GetCpuidLeaf(uint32_t leaf_id, int ecx) {
  Leaf leaf;
  int data[4];
  __cpuidex(data, leaf_id, ecx);
  leaf.eax = data[0];
  leaf.ebx = data[1];
  leaf.ecx = data[2];
  leaf.edx = data[3];
  return leaf;
}

uint32_t GetXCR0Eax(void) { return (uint32_t)_xgetbv(0); }

#else
#error "Unsupported compiler, x86 cpuid requires either GCC, Clang or MSVC."
#endif

static Leaf CpuId(uint32_t leaf_id) { return GetCpuidLeaf(leaf_id, 0); }

static const Leaf kEmptyLeaf;

static Leaf SafeCpuIdEx(uint32_t max_cpuid_leaf, uint32_t leaf_id, int ecx) {
  if (leaf_id <= max_cpuid_leaf) {
    return GetCpuidLeaf(leaf_id, ecx);
  } else {
    return kEmptyLeaf;
  }
}

static Leaf SafeCpuId(uint32_t max_cpuid_leaf, uint32_t leaf_id) {
  return SafeCpuIdEx(max_cpuid_leaf, leaf_id, 0);
}

#define MASK_XMM 0x2
#define MASK_YMM 0x4
#define MASK_MASKREG 0x20
#define MASK_ZMM0_15 0x40
#define MASK_ZMM16_31 0x80
#define MASK_XTILECFG 0x20000
#define MASK_XTILEDATA 0x40000

static bool HasMask(uint32_t value, uint32_t mask) {
  return (value & mask) == mask;
}

// Checks that operating system saves and restores xmm registers during context
// switches.
static bool HasXmmOsXSave(uint32_t xcr0_eax) {
  return HasMask(xcr0_eax, MASK_XMM);
}

// Checks that operating system saves and restores ymm registers during context
// switches.
static bool HasYmmOsXSave(uint32_t xcr0_eax) {
  return HasMask(xcr0_eax, MASK_XMM | MASK_YMM);
}

// Checks that operating system saves and restores zmm registers during context
// switches.
static bool HasZmmOsXSave(uint32_t xcr0_eax) {
  return HasMask(xcr0_eax, MASK_XMM | MASK_YMM | MASK_MASKREG | MASK_ZMM0_15 |
                               MASK_ZMM16_31);
}

// Checks that operating system saves and restores AMX/TMUL state during context
// switches.
static bool HasTmmOsXSave(uint32_t xcr0_eax) {
  return HasMask(xcr0_eax, MASK_XMM | MASK_YMM | MASK_MASKREG | MASK_ZMM0_15 |
                               MASK_ZMM16_31 | MASK_XTILECFG | MASK_XTILEDATA);
}

static bool HasSecondFMA(uint32_t model) {
  // Skylake server
  if (model == 0x55) {
    char proc_name[49] = {0};
    FillX86BrandString(proc_name);
    // detect Xeon
    if (proc_name[9] == 'X') {
      // detect Silver or Bronze
      if (proc_name[17] == 'S' || proc_name[17] == 'B') return false;
      // detect Gold 5_20 and below, except for Gold 53__
      if (proc_name[17] == 'G' && proc_name[22] == '5')
        return ((proc_name[23] == '3') ||
                (proc_name[24] == '2' && proc_name[25] == '2'));
      // detect Xeon W 210x
      if (proc_name[17] == 'W' && proc_name[21] == '0') return false;
      // detect Xeon D 2xxx
      if (proc_name[17] == 'D' && proc_name[19] == '2' && proc_name[20] == '1')
        return false;
    }
    return true;
  }
  // Cannon Lake client
  if (model == 0x66) return false;
  // Ice Lake client
  if (model == 0x7d || model == 0x7e) return false;
  // This is the right default...
  return true;
}

static void SetVendor(const Leaf leaf, char* const vendor) {
  *(uint32_t*)(vendor) = leaf.ebx;
  *(uint32_t*)(vendor + 4) = leaf.edx;
  *(uint32_t*)(vendor + 8) = leaf.ecx;
  vendor[12] = '\0';
}

static int IsVendor(const Leaf leaf, const char* const name) {
  const uint32_t ebx = *(const uint32_t*)(name);
  const uint32_t edx = *(const uint32_t*)(name + 4);
  const uint32_t ecx = *(const uint32_t*)(name + 8);
  return leaf.ebx == ebx && leaf.ecx == ecx && leaf.edx == edx;
}

static const CacheLevelInfo kEmptyCacheLevelInfo;

static CacheLevelInfo GetCacheLevelInfo(const uint32_t reg) {
  const int UNDEF = -1;
  const int KiB = 1024;
  const int MiB = 1024 * KiB;
  switch (reg) {
    case 0x01:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 32,
                              .partitioning = 0};
    case 0x02:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * MiB,
                              .ways = 0xFF,
                              .line_size = UNDEF,
                              .tlb_entries = 2,
                              .partitioning = 0};
    case 0x03:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 64,
                              .partitioning = 0};
    case 0x04:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 8,
                              .partitioning = 0};
    case 0x05:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 32,
                              .partitioning = 0};
    case 0x06:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 8 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x08:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 16 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x09:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 32 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x0A:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 8 * KiB,
                              .ways = 2,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x0B:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 4,
                              .partitioning = 0};
    case 0x0C:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 16 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x0D:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 16 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x0E:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 24 * KiB,
                              .ways = 6,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x1D:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 128 * KiB,
                              .ways = 2,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x21:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 256 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x22:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x23:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x24:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x25:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x29:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 4 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x2C:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 32 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x30:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 32 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x40:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = UNDEF,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x41:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 128 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x42:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 256 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x43:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x44:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x45:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x46:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 4 * MiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x47:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 8 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x48:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 3 * MiB,
                              .ways = 12,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x49:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 4 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case (0x49 | (1 << 8)):
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 4 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x4A:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 6 * MiB,
                              .ways = 12,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x4B:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 8 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x4C:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 12 * MiB,
                              .ways = 12,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x4D:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 16 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x4E:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 6 * MiB,
                              .ways = 24,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x4F:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 32,
                              .partitioning = 0};
    case 0x50:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 64,
                              .partitioning = 0};
    case 0x51:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 128,
                              .partitioning = 0};
    case 0x52:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 256,
                              .partitioning = 0};
    case 0x55:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 2 * MiB,
                              .ways = 0xFF,
                              .line_size = UNDEF,
                              .tlb_entries = 7,
                              .partitioning = 0};
    case 0x56:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 16,
                              .partitioning = 0};
    case 0x57:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 16,
                              .partitioning = 0};
    case 0x59:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 0xFF,
                              .line_size = UNDEF,
                              .tlb_entries = 16,
                              .partitioning = 0};
    case 0x5A:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 2 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 32,
                              .partitioning = 0};
    case 0x5B:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 64,
                              .partitioning = 0};
    case 0x5C:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 128,
                              .partitioning = 0};
    case 0x5D:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = 256,
                              .partitioning = 0};
    case 0x60:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 16 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x61:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 0xFF,
                              .line_size = UNDEF,
                              .tlb_entries = 48,
                              .partitioning = 0};
    case 0x63:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 2 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 4,
                              .partitioning = 0};
    case 0x66:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 8 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x67:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 16 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x68:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 32 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x70:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 12 * KiB,
                              .ways = 8,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x71:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 16 * KiB,
                              .ways = 8,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x72:
      return (CacheLevelInfo){.level = 1,
                              .cache_type = CPU_FEATURE_CACHE_INSTRUCTION,
                              .cache_size = 32 * KiB,
                              .ways = 8,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x76:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 2 * MiB,
                              .ways = 0xFF,
                              .line_size = UNDEF,
                              .tlb_entries = 8,
                              .partitioning = 0};
    case 0x78:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x79:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 128 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x7A:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 256 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x7B:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x7C:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 2};
    case 0x7D:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x7F:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 2,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x80:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x82:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 256 * KiB,
                              .ways = 8,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x83:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 8,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x84:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 8,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x85:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 8,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x86:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 4,
                              .line_size = 32,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0x87:
      return (CacheLevelInfo){.level = 2,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xA0:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_DTLB,
                              .cache_size = 4 * KiB,
                              .ways = 0xFF,
                              .line_size = UNDEF,
                              .tlb_entries = 32,
                              .partitioning = 0};
    case 0xB0:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 128,
                              .partitioning = 0};
    case 0xB1:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 2 * MiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 8,
                              .partitioning = 0};
    case 0xB2:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 64,
                              .partitioning = 0};
    case 0xB3:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 128,
                              .partitioning = 0};
    case 0xB4:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 256,
                              .partitioning = 0};
    case 0xB5:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 8,
                              .line_size = UNDEF,
                              .tlb_entries = 64,
                              .partitioning = 0};
    case 0xB6:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 8,
                              .line_size = UNDEF,
                              .tlb_entries = 128,
                              .partitioning = 0};
    case 0xBA:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 64,
                              .partitioning = 0};
    case 0xC0:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_TLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 8,
                              .partitioning = 0};
    case 0xC1:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_STLB,
                              .cache_size = 4 * KiB,
                              .ways = 8,
                              .line_size = UNDEF,
                              .tlb_entries = 1024,
                              .partitioning = 0};
    case 0xC2:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_DTLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 16,
                              .partitioning = 0};
    case 0xC3:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_STLB,
                              .cache_size = 4 * KiB,
                              .ways = 6,
                              .line_size = UNDEF,
                              .tlb_entries = 1536,
                              .partitioning = 0};
    case 0xCA:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_STLB,
                              .cache_size = 4 * KiB,
                              .ways = 4,
                              .line_size = UNDEF,
                              .tlb_entries = 512,
                              .partitioning = 0};
    case 0xD0:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 512 * KiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xD1:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xD2:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 4,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xD6:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xD7:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xD8:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 4 * MiB,
                              .ways = 8,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xDC:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 1 * 1536 * KiB,
                              .ways = 12,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xDD:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 3 * MiB,
                              .ways = 12,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xDE:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 6 * MiB,
                              .ways = 12,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xE2:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 2 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xE3:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 4 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xE4:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 8 * MiB,
                              .ways = 16,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xEA:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 12 * MiB,
                              .ways = 24,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xEB:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 18 * MiB,
                              .ways = 24,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xEC:
      return (CacheLevelInfo){.level = 3,
                              .cache_type = CPU_FEATURE_CACHE_DATA,
                              .cache_size = 24 * MiB,
                              .ways = 24,
                              .line_size = 64,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xF0:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_PREFETCH,
                              .cache_size = 64 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xF1:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_PREFETCH,
                              .cache_size = 128 * KiB,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    case 0xFF:
      return (CacheLevelInfo){.level = UNDEF,
                              .cache_type = CPU_FEATURE_CACHE_NULL,
                              .cache_size = UNDEF,
                              .ways = UNDEF,
                              .line_size = UNDEF,
                              .tlb_entries = UNDEF,
                              .partitioning = 0};
    default:
      return kEmptyCacheLevelInfo;
  }
}

static void GetByteArrayFromRegister(uint32_t result[4], const uint32_t reg) {
  for (int i = 0; i < 4; ++i) {
    result[i] = ExtractBitRange(reg, (i + 1) * 8, i * 8);
  }
}

static void ParseLeaf2(const int max_cpuid_leaf, CacheInfo* info) {
  Leaf leaf = SafeCpuId(max_cpuid_leaf, 2);
  uint32_t registers[] = {leaf.eax, leaf.ebx, leaf.ecx, leaf.edx};
  for (int i = 0; i < 4; ++i) {
    if (registers[i] & (1U << 31)) {
      continue;  // register does not contains valid information
    }
    uint32_t bytes[4];
    GetByteArrayFromRegister(bytes, registers[i]);
    for (int j = 0; j < 4; ++j) {
      if (bytes[j] == 0xFF)
        break;  // leaf 4 should be used to fetch cache information
      info->levels[info->size] = GetCacheLevelInfo(bytes[j]);
    }
    info->size++;
  }
}

static void ParseLeaf4(const int max_cpuid_leaf, CacheInfo* info) {
  info->size = 0;
  for (int cache_id = 0; cache_id < CPU_FEATURES_MAX_CACHE_LEVEL; cache_id++) {
    const Leaf leaf = SafeCpuIdEx(max_cpuid_leaf, 4, cache_id);
    CacheType cache_type = ExtractBitRange(leaf.eax, 4, 0);
    if (cache_type == CPU_FEATURE_CACHE_NULL) {
      info->levels[cache_id] = kEmptyCacheLevelInfo;
      continue;
    }
    int level = ExtractBitRange(leaf.eax, 7, 5);
    int line_size = ExtractBitRange(leaf.ebx, 11, 0) + 1;
    int partitioning = ExtractBitRange(leaf.ebx, 21, 12) + 1;
    int ways = ExtractBitRange(leaf.ebx, 31, 22) + 1;
    int tlb_entries = leaf.ecx + 1;
    int cache_size = (ways * partitioning * line_size * (tlb_entries));
    info->levels[cache_id] = (CacheLevelInfo){.level = level,
                                              .cache_type = cache_type,
                                              .cache_size = cache_size,
                                              .ways = ways,
                                              .line_size = line_size,
                                              .tlb_entries = tlb_entries,
                                              .partitioning = partitioning};
    info->size++;
  }
}

// Internal structure to hold the OS support for vector operations.
// Avoid to recompute them since each call to cpuid is ~100 cycles.
typedef struct {
  bool have_sse_via_os;
  bool have_sse_via_cpuid;
  bool have_avx;
  bool have_avx512;
  bool have_amx;
} OsSupport;

static const OsSupport kEmptyOsSupport;

static OsSupport CheckOsSupport(const uint32_t max_cpuid_leaf) {
  const Leaf leaf_1 = SafeCpuId(max_cpuid_leaf, 1);
  const bool have_xsave = IsBitSet(leaf_1.ecx, 26);
  const bool have_osxsave = IsBitSet(leaf_1.ecx, 27);
  const bool have_xcr0 = have_xsave && have_osxsave;

  OsSupport os_support = kEmptyOsSupport;

  if (have_xcr0) {
    // AVX capable cpu will expose XCR0.
    const uint32_t xcr0_eax = GetXCR0Eax();
    os_support.have_sse_via_cpuid = HasXmmOsXSave(xcr0_eax);
    os_support.have_avx = HasYmmOsXSave(xcr0_eax);
    os_support.have_avx512 = HasZmmOsXSave(xcr0_eax);
    os_support.have_amx = HasTmmOsXSave(xcr0_eax);
  } else {
    // Atom based or older cpus need to ask the OS for sse support.
    os_support.have_sse_via_os = true;
  }

  return os_support;
}

#if defined(CPU_FEATURES_OS_WINDOWS)
#if defined(CPU_FEATURES_MOCK_CPUID_X86)
extern bool GetWindowsIsProcessorFeaturePresent(DWORD);
#else  // CPU_FEATURES_MOCK_CPUID_X86
static bool GetWindowsIsProcessorFeaturePresent(DWORD ProcessorFeature) {
  return IsProcessorFeaturePresent(ProcessorFeature);
}
#endif
#endif  // CPU_FEATURES_OS_WINDOWS

#if defined(CPU_FEATURES_OS_DARWIN)
#if defined(CPU_FEATURES_MOCK_CPUID_X86)
extern bool GetDarwinSysCtlByName(const char*);
#else  // CPU_FEATURES_MOCK_CPUID_X86
static bool GetDarwinSysCtlByName(const char* name) {
  int enabled;
  size_t enabled_len = sizeof(enabled);
  const int failure = sysctlbyname(name, &enabled, &enabled_len, NULL, 0);
  return failure ? false : enabled;
}
#endif
#endif  // CPU_FEATURES_OS_DARWIN

static void DetectSseViaOs(X86Features* features) {
#if defined(CPU_FEATURES_OS_WINDOWS)
  // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-isprocessorfeaturepresent
  features->sse =
      GetWindowsIsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
  features->sse2 =
      GetWindowsIsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE);
  features->sse3 =
      GetWindowsIsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE);
#elif defined(CPU_FEATURES_OS_DARWIN)
  // Handling Darwin platform through sysctlbyname.
  features->sse = GetDarwinSysCtlByName("hw.optional.sse");
  features->sse2 = GetDarwinSysCtlByName("hw.optional.sse2");
  features->sse3 = GetDarwinSysCtlByName("hw.optional.sse3");
  features->ssse3 = GetDarwinSysCtlByName("hw.optional.supplementalsse3");
  features->sse4_1 = GetDarwinSysCtlByName("hw.optional.sse4_1");
  features->sse4_2 = GetDarwinSysCtlByName("hw.optional.sse4_2");
#elif defined(CPU_FEATURES_OS_LINUX_OR_ANDROID)
  // Handling Linux platform through /proc/cpuinfo.
  const int fd = CpuFeatures_OpenFile("/proc/cpuinfo");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      const LineResult result = StackLineReader_NextLine(&reader);
      const StringView line = result.line;
      StringView key, value;
      if (CpuFeatures_StringView_GetAttributeKeyValue(line, &key, &value)) {
        if (CpuFeatures_StringView_IsEquals(key, str("flags"))) {
          features->sse = CpuFeatures_StringView_HasWord(value, "sse");
          features->sse2 = CpuFeatures_StringView_HasWord(value, "sse2");
          features->sse3 = CpuFeatures_StringView_HasWord(value, "sse3");
          features->ssse3 = CpuFeatures_StringView_HasWord(value, "ssse3");
          features->sse4_1 = CpuFeatures_StringView_HasWord(value, "sse4_1");
          features->sse4_2 = CpuFeatures_StringView_HasWord(value, "sse4_2");
          break;
        }
      }
      if (result.eof) break;
    }
    CpuFeatures_CloseFile(fd);
  }
#else
#error "Unsupported fallback detection of SSE OS support."
#endif
}

// Reference https://en.wikipedia.org/wiki/CPUID.
static void ParseCpuId(const uint32_t max_cpuid_leaf,
                       const OsSupport os_support, X86Info* info) {
  const Leaf leaf_1 = SafeCpuId(max_cpuid_leaf, 1);
  const Leaf leaf_7 = SafeCpuId(max_cpuid_leaf, 7);
  const Leaf leaf_7_1 = SafeCpuIdEx(max_cpuid_leaf, 7, 1);

  const uint32_t family = ExtractBitRange(leaf_1.eax, 11, 8);
  const uint32_t extended_family = ExtractBitRange(leaf_1.eax, 27, 20);
  const uint32_t model = ExtractBitRange(leaf_1.eax, 7, 4);
  const uint32_t extended_model = ExtractBitRange(leaf_1.eax, 19, 16);

  X86Features* const features = &info->features;

  info->family = extended_family + family;
  info->model = (extended_model << 4) + model;
  info->stepping = ExtractBitRange(leaf_1.eax, 3, 0);

  features->fpu = IsBitSet(leaf_1.edx, 0);
  features->tsc = IsBitSet(leaf_1.edx, 4);
  features->cx8 = IsBitSet(leaf_1.edx, 8);
  features->clfsh = IsBitSet(leaf_1.edx, 19);
  features->mmx = IsBitSet(leaf_1.edx, 23);
  features->ss = IsBitSet(leaf_1.edx, 27);
  features->pclmulqdq = IsBitSet(leaf_1.ecx, 1);
  features->smx = IsBitSet(leaf_1.ecx, 6);
  features->cx16 = IsBitSet(leaf_1.ecx, 13);
  features->dca = IsBitSet(leaf_1.ecx, 18);
  features->movbe = IsBitSet(leaf_1.ecx, 22);
  features->popcnt = IsBitSet(leaf_1.ecx, 23);
  features->aes = IsBitSet(leaf_1.ecx, 25);
  features->f16c = IsBitSet(leaf_1.ecx, 29);
  features->rdrnd = IsBitSet(leaf_1.ecx, 30);
  features->sgx = IsBitSet(leaf_7.ebx, 2);
  features->bmi1 = IsBitSet(leaf_7.ebx, 3);
  features->hle = IsBitSet(leaf_7.ebx, 4);
  features->bmi2 = IsBitSet(leaf_7.ebx, 8);
  features->erms = IsBitSet(leaf_7.ebx, 9);
  features->rtm = IsBitSet(leaf_7.ebx, 11);
  features->rdseed = IsBitSet(leaf_7.ebx, 18);
  features->clflushopt = IsBitSet(leaf_7.ebx, 23);
  features->clwb = IsBitSet(leaf_7.ebx, 24);
  features->sha = IsBitSet(leaf_7.ebx, 29);
  features->vaes = IsBitSet(leaf_7.ecx, 9);
  features->vpclmulqdq = IsBitSet(leaf_7.ecx, 10);

  if (os_support.have_sse_via_os) {
    DetectSseViaOs(features);
  } else if (os_support.have_sse_via_cpuid) {
    features->sse = IsBitSet(leaf_1.edx, 25);
    features->sse2 = IsBitSet(leaf_1.edx, 26);
    features->sse3 = IsBitSet(leaf_1.ecx, 0);
    features->ssse3 = IsBitSet(leaf_1.ecx, 9);
    features->sse4_1 = IsBitSet(leaf_1.ecx, 19);
    features->sse4_2 = IsBitSet(leaf_1.ecx, 20);
  }

  if (os_support.have_avx) {
    features->fma3 = IsBitSet(leaf_1.ecx, 12);
    features->avx = IsBitSet(leaf_1.ecx, 28);
    features->avx2 = IsBitSet(leaf_7.ebx, 5);
  }

  if (os_support.have_avx512) {
    features->avx512f = IsBitSet(leaf_7.ebx, 16);
    features->avx512cd = IsBitSet(leaf_7.ebx, 28);
    features->avx512er = IsBitSet(leaf_7.ebx, 27);
    features->avx512pf = IsBitSet(leaf_7.ebx, 26);
    features->avx512bw = IsBitSet(leaf_7.ebx, 30);
    features->avx512dq = IsBitSet(leaf_7.ebx, 17);
    features->avx512vl = IsBitSet(leaf_7.ebx, 31);
    features->avx512ifma = IsBitSet(leaf_7.ebx, 21);
    features->avx512vbmi = IsBitSet(leaf_7.ecx, 1);
    features->avx512vbmi2 = IsBitSet(leaf_7.ecx, 6);
    features->avx512vnni = IsBitSet(leaf_7.ecx, 11);
    features->avx512bitalg = IsBitSet(leaf_7.ecx, 12);
    features->avx512vpopcntdq = IsBitSet(leaf_7.ecx, 14);
    features->avx512_4vnniw = IsBitSet(leaf_7.edx, 2);
    features->avx512_4vbmi2 = IsBitSet(leaf_7.edx, 3);
    features->avx512_second_fma = HasSecondFMA(info->model);
    features->avx512_4fmaps = IsBitSet(leaf_7.edx, 3);
    features->avx512_bf16 = IsBitSet(leaf_7_1.eax, 5);
    features->avx512_vp2intersect = IsBitSet(leaf_7.edx, 8);
  }

  if (os_support.have_amx) {
    features->amx_bf16 = IsBitSet(leaf_7.edx, 22);
    features->amx_tile = IsBitSet(leaf_7.edx, 24);
    features->amx_int8 = IsBitSet(leaf_7.edx, 25);
  }
}

// Reference
// https://en.wikipedia.org/wiki/CPUID#EAX=80000000h:_Get_Highest_Extended_Function_Implemented.
static void ParseExtraAMDCpuId(X86Info* info, OsSupport os_support) {
  const Leaf leaf_80000000 = CpuId(0x80000000);
  const uint32_t max_extended_cpuid_leaf = leaf_80000000.eax;
  const Leaf leaf_80000001 = SafeCpuId(max_extended_cpuid_leaf, 0x80000001);

  X86Features* const features = &info->features;

  if (os_support.have_sse_via_cpuid) {
    features->sse4a = IsBitSet(leaf_80000001.ecx, 6);
  }

  if (os_support.have_avx) {
    features->fma4 = IsBitSet(leaf_80000001.ecx, 16);
  }
}

static const X86Info kEmptyX86Info;
static const CacheInfo kEmptyCacheInfo;

X86Info GetX86Info(void) {
  X86Info info = kEmptyX86Info;
  const Leaf leaf_0 = CpuId(0);
  const bool is_intel = IsVendor(leaf_0, "GenuineIntel");
  const bool is_amd = IsVendor(leaf_0, "AuthenticAMD");
  SetVendor(leaf_0, info.vendor);
  if (is_intel || is_amd) {
    const uint32_t max_cpuid_leaf = leaf_0.eax;
    const OsSupport os_support = CheckOsSupport(max_cpuid_leaf);
    ParseCpuId(max_cpuid_leaf, os_support, &info);
    if (is_amd) {
      ParseExtraAMDCpuId(&info, os_support);
    }
  }
  return info;
}

CacheInfo GetX86CacheInfo(void) {
  CacheInfo info = kEmptyCacheInfo;
  const Leaf leaf_0 = CpuId(0);
  const uint32_t max_cpuid_leaf = leaf_0.eax;
  if (IsVendor(leaf_0, "GenuineIntel")) {
    ParseLeaf2(max_cpuid_leaf, &info);
    ParseLeaf4(max_cpuid_leaf, &info);
  }
  return info;
}

#define CPUID(FAMILY, MODEL) ((((FAMILY)&0xFF) << 8) | ((MODEL)&0xFF))

X86Microarchitecture GetX86Microarchitecture(const X86Info* info) {
  if (memcmp(info->vendor, "GenuineIntel", sizeof(info->vendor)) == 0) {
    switch (CPUID(info->family, info->model)) {
      case CPUID(0x06, 0x35):
      case CPUID(0x06, 0x36):
        // https://en.wikipedia.org/wiki/Bonnell_(microarchitecture)
        return INTEL_ATOM_BNL;
      case CPUID(0x06, 0x37):
      case CPUID(0x06, 0x4C):
        // https://en.wikipedia.org/wiki/Silvermont
        return INTEL_ATOM_SMT;
      case CPUID(0x06, 0x5C):
        // https://en.wikipedia.org/wiki/Goldmont
        return INTEL_ATOM_GMT;
      case CPUID(0x06, 0x0F):
      case CPUID(0x06, 0x16):
        // https://en.wikipedia.org/wiki/Intel_Core_(microarchitecture)
        return INTEL_CORE;
      case CPUID(0x06, 0x17):
      case CPUID(0x06, 0x1D):
        // https://en.wikipedia.org/wiki/Penryn_(microarchitecture)
        return INTEL_PNR;
      case CPUID(0x06, 0x1A):
      case CPUID(0x06, 0x1E):
      case CPUID(0x06, 0x1F):
      case CPUID(0x06, 0x2E):
        // https://en.wikipedia.org/wiki/Nehalem_(microarchitecture)
        return INTEL_NHM;
      case CPUID(0x06, 0x25):
      case CPUID(0x06, 0x2C):
      case CPUID(0x06, 0x2F):
        // https://en.wikipedia.org/wiki/Westmere_(microarchitecture)
        return INTEL_WSM;
      case CPUID(0x06, 0x2A):
      case CPUID(0x06, 0x2D):
        // https://en.wikipedia.org/wiki/Sandy_Bridge#Models_and_steppings
        return INTEL_SNB;
      case CPUID(0x06, 0x3A):
      case CPUID(0x06, 0x3E):
        // https://en.wikipedia.org/wiki/Ivy_Bridge_(microarchitecture)#Models_and_steppings
        return INTEL_IVB;
      case CPUID(0x06, 0x3C):
      case CPUID(0x06, 0x3F):
      case CPUID(0x06, 0x45):
      case CPUID(0x06, 0x46):
        // https://en.wikipedia.org/wiki/Haswell_(microarchitecture)
        return INTEL_HSW;
      case CPUID(0x06, 0x3D):
      case CPUID(0x06, 0x47):
      case CPUID(0x06, 0x4F):
      case CPUID(0x06, 0x56):
        // https://en.wikipedia.org/wiki/Broadwell_(microarchitecture)
        return INTEL_BDW;
      case CPUID(0x06, 0x4E):
      case CPUID(0x06, 0x55):
      case CPUID(0x06, 0x5E):
        // https://en.wikipedia.org/wiki/Skylake_(microarchitecture)
        return INTEL_SKL;
      case CPUID(0x06, 0x66):
        // https://en.wikipedia.org/wiki/Cannon_Lake_(microarchitecture)
        return INTEL_CNL;
      case CPUID(0x06, 0x7D):  // client
      case CPUID(0x06, 0x7E):  // client
      case CPUID(0x06, 0x9D):  // NNP-I
      case CPUID(0x06, 0x6A):  // server
      case CPUID(0x06, 0x6C):  // server
        // https://en.wikipedia.org/wiki/Ice_Lake_(microprocessor)
        return INTEL_ICL;
      case CPUID(0x06, 0x8C):
      case CPUID(0x06, 0x8D):
        // https://en.wikipedia.org/wiki/Tiger_Lake_(microarchitecture)
        return INTEL_TGL;
      case CPUID(0x06, 0x8F):
        // https://en.wikipedia.org/wiki/Sapphire_Rapids
        return INTEL_SPR;
      case CPUID(0x06, 0x8E):
        switch (info->stepping) {
          case 9:
            return INTEL_KBL;  // https://en.wikipedia.org/wiki/Kaby_Lake
          case 10:
            return INTEL_CFL;  // https://en.wikipedia.org/wiki/Coffee_Lake
          case 11:
            return INTEL_WHL;  // https://en.wikipedia.org/wiki/Whiskey_Lake_(microarchitecture)
          default:
            return X86_UNKNOWN;
        }
      case CPUID(0x06, 0x9E):
        if (info->stepping > 9) {
          // https://en.wikipedia.org/wiki/Coffee_Lake
          return INTEL_CFL;
        } else {
          // https://en.wikipedia.org/wiki/Kaby_Lake
          return INTEL_KBL;
        }
      default:
        return X86_UNKNOWN;
    }
  }
  if (memcmp(info->vendor, "AuthenticAMD", sizeof(info->vendor)) == 0) {
    switch (info->family) {
        // https://en.wikipedia.org/wiki/List_of_AMD_CPU_microarchitectures
      case 0x0F:
        return AMD_HAMMER;
      case 0x10:
        return AMD_K10;
      case 0x14:
        return AMD_BOBCAT;
      case 0x15:
        return AMD_BULLDOZER;
      case 0x16:
        return AMD_JAGUAR;
      case 0x17:
        return AMD_ZEN;
      case 0x19:
        return AMD_ZEN3;
      default:
        return X86_UNKNOWN;
    }
  }
  return X86_UNKNOWN;
}

static void SetString(const uint32_t max_cpuid_ext_leaf, const uint32_t leaf_id,
                      char* buffer) {
  const Leaf leaf = SafeCpuId(max_cpuid_ext_leaf, leaf_id);
  // We allow calling memcpy from SetString which is only called when requesting
  // X86BrandString.
  memcpy(buffer, &leaf, sizeof(Leaf));
}

void FillX86BrandString(char brand_string[49]) {
  const Leaf leaf_ext_0 = CpuId(0x80000000);
  const uint32_t max_cpuid_leaf_ext = leaf_ext_0.eax;
  SetString(max_cpuid_leaf_ext, 0x80000002, brand_string);
  SetString(max_cpuid_leaf_ext, 0x80000003, brand_string + 16);
  SetString(max_cpuid_leaf_ext, 0x80000004, brand_string + 32);
  brand_string[48] = '\0';
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetX86FeaturesEnumValue(const X86Features* features,
                            X86FeaturesEnum value) {
  if (value >= X86_LAST_) return false;
  return kGetters[value](features);
}

const char* GetX86FeaturesEnumName(X86FeaturesEnum value) {
  if (value >= X86_LAST_) return "unknown_feature";
  return kCpuInfoFlags[value];
}

const char* GetX86MicroarchitectureName(X86Microarchitecture uarch) {
  switch (uarch) {
    case X86_UNKNOWN:
      return "X86_UNKNOWN";
    case INTEL_CORE:
      return "INTEL_CORE";
    case INTEL_PNR:
      return "INTEL_PNR";
    case INTEL_NHM:
      return "INTEL_NHM";
    case INTEL_ATOM_BNL:
      return "INTEL_ATOM_BNL";
    case INTEL_WSM:
      return "INTEL_WSM";
    case INTEL_SNB:
      return "INTEL_SNB";
    case INTEL_IVB:
      return "INTEL_IVB";
    case INTEL_ATOM_SMT:
      return "INTEL_ATOM_SMT";
    case INTEL_HSW:
      return "INTEL_HSW";
    case INTEL_BDW:
      return "INTEL_BDW";
    case INTEL_SKL:
      return "INTEL_SKL";
    case INTEL_ATOM_GMT:
      return "INTEL_ATOM_GMT";
    case INTEL_KBL:
      return "INTEL_KBL";
    case INTEL_CFL:
      return "INTEL_CFL";
    case INTEL_WHL:
      return "INTEL_WHL";
    case INTEL_CNL:
      return "INTEL_CNL";
    case INTEL_ICL:
      return "INTEL_ICL";
    case INTEL_TGL:
      return "INTEL_TGL";
    case INTEL_SPR:
      return "INTEL_SPR";
    case AMD_HAMMER:
      return "AMD_HAMMER";
    case AMD_K10:
      return "AMD_K10";
    case AMD_BOBCAT:
      return "AMD_BOBCAT";
    case AMD_BULLDOZER:
      return "AMD_BULLDOZER";
    case AMD_JAGUAR:
      return "AMD_JAGUAR";
    case AMD_ZEN:
      return "AMD_ZEN";
    case AMD_ZEN3:
      return "AMD_ZEN3";
  }
  return "unknown microarchitecture";
}
