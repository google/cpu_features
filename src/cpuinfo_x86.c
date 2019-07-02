// Copyright 2017 Google Inc.
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
#include "internal/bit_utils.h"
#include "internal/cpuid_x86.h"

#include <stdbool.h>
#include <string.h>

#if !defined(CPU_FEATURES_ARCH_X86)
#error "Cannot compile cpuinfo_x86 on a non x86 platform."
#endif

////////////////////////////////////////////////////////////////////////////////
// Definitions for CpuId and GetXCR0Eax.
////////////////////////////////////////////////////////////////////////////////

#if defined(CPU_FEATURES_MOCK_CPUID_X86)
// Implementation will be provided by test/cpuinfo_x86_test.cc.
#elif defined(CPU_FEATURES_COMPILER_CLANG) || defined(CPU_FEATURES_COMPILER_GCC)

#include <cpuid.h>

Leaf CpuIdEx(uint32_t leaf_id, int ecx) {
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

Leaf CpuIdEx(uint32_t leaf_id, int ecx) {
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

static Leaf CpuId(uint32_t leaf_id) { return CpuIdEx(leaf_id, 0); }

static const Leaf kEmptyLeaf;

static Leaf SafeCpuIdEx(uint32_t max_cpuid_leaf, uint32_t leaf_id, int ecx) {
  if (leaf_id <= max_cpuid_leaf) {
    return CpuIdEx(leaf_id, ecx);
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

static CacheLevelInfo MakeX86CacheLevelInfo(int level, CacheType cache_type,
                                            int cache_size, int ways,
                                            int line_size, int entries,
                                            int partitioning) {
  CacheLevelInfo info;
  info.level = level;
  info.cache_type = cache_type;
  info.cache_size = cache_size;
  info.ways = ways;
  info.line_size = line_size;
  info.tlb_entries = entries;
  info.partitioning = partitioning;
  return info;
}

static CacheLevelInfo GetCacheLevelInfo(const uint32_t reg) {
  const int UNDEF = -1;
  const int KiB = 1024;
  const int MiB = 1024 * KiB;
  const int GiB = 1024 * MiB;
  switch (reg) {
    case 0x01:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 32, 0);
    case 0x02:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * MiB, 0xFF,
                                   UNDEF, 2, 0);
    case 0x03:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 64, 0);
    case 0x04:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * MiB, 4,
                                   UNDEF, 8, 0);
    case 0x05:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * MiB, 4,
                                   UNDEF, 32, 0);
    case 0x06:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 8 * KiB, 4,
                                   32, UNDEF, 0);
    case 0x08:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 16 * KiB,
                                   4, 32, UNDEF, 0);
    case 0x09:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 32 * KiB,
                                   4, 64, UNDEF, 0);
    case 0x0A:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 8 * KiB, 2, 32,
                                   UNDEF, 0);
    case 0x0B:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * MiB, 4,
                                   UNDEF, 4, 0);
    case 0x0C:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 16 * KiB, 4, 32,
                                   UNDEF, 0);
    case 0x0D:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 16 * KiB, 4, 64,
                                   UNDEF, 0);
    case 0x0E:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 24 * KiB, 6, 64,
                                   UNDEF, 0);
    case 0x1D:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 128 * KiB, 2, 64,
                                   UNDEF, 0);
    case 0x21:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 256 * KiB, 8, 64,
                                   UNDEF, 0);
    case 0x22:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 512 * KiB, 4, 64,
                                   UNDEF, 2);
    case 0x23:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 1 * MiB, 8, 64,
                                   UNDEF, 2);
    case 0x24:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 1 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0x25:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 2 * MiB, 8, 64,
                                   UNDEF, 2);
    case 0x29:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 4 * MiB, 8, 64,
                                   UNDEF, 2);
    case 0x2C:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 32 * KiB, 8, 64,
                                   UNDEF, 0);
    case 0x30:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 32 * KiB,
                                   8, 64, UNDEF, 0);
    case 0x40:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_DATA, UNDEF, UNDEF,
                                   UNDEF, UNDEF, 0);
    case 0x41:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 128 * KiB, 4, 32,
                                   UNDEF, 0);
    case 0x42:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 256 * KiB, 4, 32,
                                   UNDEF, 0);
    case 0x43:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 512 * KiB, 4, 32,
                                   UNDEF, 0);
    case 0x44:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 1 * MiB, 4, 32,
                                   UNDEF, 0);
    case 0x45:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 2 * MiB, 4, 32,
                                   UNDEF, 0);
    case 0x46:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 4 * MiB, 4, 64,
                                   UNDEF, 0);
    case 0x47:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 8 * MiB, 8, 64,
                                   UNDEF, 0);
    case 0x48:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 3 * MiB, 12, 64,
                                   UNDEF, 0);
    case 0x49:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 4 * MiB, 16, 64,
                                   UNDEF, 0);
    case (0x49 | (1 << 8)):
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 4 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0x4A:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 6 * MiB, 12, 64,
                                   UNDEF, 0);
    case 0x4B:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 8 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0x4C:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 12 * MiB, 12, 64,
                                   UNDEF, 0);
    case 0x4D:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 16 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0x4E:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 6 * MiB, 24, 64,
                                   UNDEF, 0);
    case 0x4F:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, UNDEF,
                                   UNDEF, 32, 0);
    case 0x50:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, UNDEF,
                                   UNDEF, 64, 0);
    case 0x51:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, UNDEF,
                                   UNDEF, 128, 0);
    case 0x52:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, UNDEF,
                                   UNDEF, 256, 0);
    case 0x55:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 2 * MiB, 0xFF,
                                   UNDEF, 7, 0);
    case 0x56:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * MiB, 4,
                                   UNDEF, 16, 0);
    case 0x57:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 16, 0);
    case 0x59:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 0xFF,
                                   UNDEF, 16, 0);
    case 0x5A:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 2 * MiB, 4,
                                   UNDEF, 32, 0);
    case 0x5B:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, UNDEF,
                                   UNDEF, 64, 0);
    case 0x5C:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, UNDEF,
                                   UNDEF, 128, 0);
    case 0x5D:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4, UNDEF,
                                   UNDEF, 256, 0);
    case 0x60:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 16 * KiB, 8, 64,
                                   UNDEF, 0);
    case 0x61:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 0xFF,
                                   UNDEF, 48, 0);
    case 0x63:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 2 * MiB, 4,
                                   UNDEF, 4, 0);
    case 0x66:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 8 * KiB, 4, 64,
                                   UNDEF, 0);
    case 0x67:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 16 * KiB, 4, 64,
                                   UNDEF, 0);
    case 0x68:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_DATA, 32 * KiB, 4, 64,
                                   UNDEF, 0);
    case 0x70:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 12 * KiB,
                                   8, UNDEF, UNDEF, 0);
    case 0x71:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 16 * KiB,
                                   8, UNDEF, UNDEF, 0);
    case 0x72:
      return MakeX86CacheLevelInfo(1, CPU_FEATURE_CACHE_INSTRUCTION, 32 * KiB,
                                   8, UNDEF, UNDEF, 0);
    case 0x76:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 2 * MiB, 0xFF,
                                   UNDEF, 8, 0);
    case 0x78:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 1 * MiB, 4, 64,
                                   UNDEF, 0);
    case 0x79:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 128 * KiB, 8, 64,
                                   UNDEF, 2);
    case 0x7A:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 256 * KiB, 8, 64,
                                   UNDEF, 2);
    case 0x7B:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 512 * KiB, 8, 64,
                                   UNDEF, 2);
    case 0x7C:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 1 * MiB, 8, 64,
                                   UNDEF, 2);
    case 0x7D:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 2 * MiB, 8, 64,
                                   UNDEF, 0);
    case 0x7F:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 512 * KiB, 2, 64,
                                   UNDEF, 0);
    case 0x80:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 512 * KiB, 8, 64,
                                   UNDEF, 0);
    case 0x82:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 256 * KiB, 8, 32,
                                   UNDEF, 0);
    case 0x83:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 512 * KiB, 8, 32,
                                   UNDEF, 0);
    case 0x84:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 1 * MiB, 8, 32,
                                   UNDEF, 0);
    case 0x85:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 2 * MiB, 8, 32,
                                   UNDEF, 0);
    case 0x86:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 512 * KiB, 4, 32,
                                   UNDEF, 0);
    case 0x87:
      return MakeX86CacheLevelInfo(2, CPU_FEATURE_CACHE_DATA, 1 * MiB, 8, 64,
                                   UNDEF, 0);
    case 0xA0:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_DTLB, 4 * KiB, 0xFF,
                                   UNDEF, 32, 0);
    case 0xB0:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 128, 0);
    case 0xB1:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 2 * MiB, 4,
                                   UNDEF, 8, 0);
    case 0xB2:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 64, 0);
    case 0xB3:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 128, 0);
    case 0xB4:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 256, 0);
    case 0xB5:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 8,
                                   UNDEF, 64, 0);
    case 0xB6:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 8,
                                   UNDEF, 128, 0);
    case 0xBA:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 64, 0);
    case 0xC0:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_TLB, 4 * KiB, 4,
                                   UNDEF, 8, 0);
    case 0xC1:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_STLB, 4 * KiB, 8,
                                   UNDEF, 1024, 0);
    case 0xC2:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_DTLB, 4 * KiB, 4,
                                   UNDEF, 16, 0);
    case 0xC3:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_STLB, 4 * KiB, 6,
                                   UNDEF, 1536, 0);
    case 0xCA:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_STLB, 4 * KiB, 4,
                                   UNDEF, 512, 0);
    case 0xD0:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 512 * KiB, 4, 64,
                                   UNDEF, 0);
    case 0xD1:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 1 * MiB, 4, 64,
                                   UNDEF, 0);
    case 0xD2:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 2 * MiB, 4, 64,
                                   UNDEF, 0);
    case 0xD6:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 1 * MiB, 8, 64,
                                   UNDEF, 0);
    case 0xD7:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 2 * MiB, 8, 64,
                                   UNDEF, 0);
    case 0xD8:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 4 * MiB, 8, 64,
                                   UNDEF, 0);
    case 0xDC:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 1 * 1536 * KiB,
                                   12, 64, UNDEF, 0);
    case 0xDD:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 3 * MiB, 12, 64,
                                   UNDEF, 0);
    case 0xDE:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 6 * MiB, 12, 64,
                                   UNDEF, 0);
    case 0xE2:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 2 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0xE3:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 4 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0xE4:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 8 * MiB, 16, 64,
                                   UNDEF, 0);
    case 0xEA:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 12 * MiB, 24, 64,
                                   UNDEF, 0);
    case 0xEB:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 18 * MiB, 24, 64,
                                   UNDEF, 0);
    case 0xEC:
      return MakeX86CacheLevelInfo(3, CPU_FEATURE_CACHE_DATA, 24 * MiB, 24, 64,
                                   UNDEF, 0);
    case 0xF0:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_PREFETCH, 64 * KiB,
                                   UNDEF, UNDEF, UNDEF, 0);
    case 0xF1:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_PREFETCH, 128 * KiB,
                                   UNDEF, UNDEF, UNDEF, 0);
    case 0xFF:
      return MakeX86CacheLevelInfo(UNDEF, CPU_FEATURE_CACHE_NULL, UNDEF, UNDEF,
                                   UNDEF, UNDEF, 0);
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
    if (registers[i] & (1 << 31)) {
      continue;  // register does not contains valid information
    }
    uint32_t bytes[4];
    GetByteArrayFromRegister(bytes, registers[i]);
    for (int i = 0; i < 4; ++i) {
      if (bytes[i] == 0xFF)
        break;  // leaf 4 should be used to fetch cache information
      info->levels[info->size] = GetCacheLevelInfo(bytes[i]);
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
    int entries = leaf.ecx + 1;
    int cache_size = (ways * partitioning * line_size * (entries));
    info->levels[cache_id] = MakeX86CacheLevelInfo(
        level, cache_type, cache_size, ways, line_size, entries, partitioning);
    info->size++;
  }
}

// Reference https://en.wikipedia.org/wiki/CPUID.
static void ParseCpuId(const uint32_t max_cpuid_leaf, X86Info* info) {
  const Leaf leaf_1 = SafeCpuId(max_cpuid_leaf, 1);
  const Leaf leaf_7 = SafeCpuId(max_cpuid_leaf, 7);

  const bool have_xsave = IsBitSet(leaf_1.ecx, 26);
  const bool have_osxsave = IsBitSet(leaf_1.ecx, 27);
  const uint32_t xcr0_eax = (have_xsave && have_osxsave) ? GetXCR0Eax() : 0;
  const bool have_sse_os_support = HasXmmOsXSave(xcr0_eax);
  const bool have_avx_os_support = HasYmmOsXSave(xcr0_eax);
  const bool have_avx512_os_support = HasZmmOsXSave(xcr0_eax);

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

  if (have_sse_os_support) {
    features->sse = IsBitSet(leaf_1.edx, 25);
    features->sse2 = IsBitSet(leaf_1.edx, 26);
    features->sse3 = IsBitSet(leaf_1.ecx, 0);
    features->ssse3 = IsBitSet(leaf_1.ecx, 9);
    features->sse4_1 = IsBitSet(leaf_1.ecx, 19);
    features->sse4_2 = IsBitSet(leaf_1.ecx, 20);
  }

  if (have_avx_os_support) {
    features->fma3 = IsBitSet(leaf_1.ecx, 12);
    features->avx = IsBitSet(leaf_1.ecx, 28);
    features->avx2 = IsBitSet(leaf_7.ebx, 5);
  }

  if (have_avx512_os_support) {
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
  }
}

static const X86Info kEmptyX86Info;
static const CacheInfo kEmptyCacheInfo;

X86Info GetX86Info(void) {
  X86Info info = kEmptyX86Info;
  const Leaf leaf_0 = CpuId(0);
  const uint32_t max_cpuid_leaf = leaf_0.eax;
  SetVendor(leaf_0, info.vendor);
  if (IsVendor(leaf_0, "GenuineIntel") || IsVendor(leaf_0, "AuthenticAMD")) {
    ParseCpuId(max_cpuid_leaf, &info);
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
      case CPUID(0x06, 0x8E):
      case CPUID(0x06, 0x9E):
        // https://en.wikipedia.org/wiki/Kaby_Lake
        return INTEL_KBL;
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
  switch (value) {
    case X86_FPU:
      return features->fpu;
    case X86_TSC:
      return features->tsc;
    case X86_CX8:
      return features->cx8;
    case X86_CLFSH:
      return features->clfsh;
    case X86_MMX:
      return features->mmx;
    case X86_AES:
      return features->aes;
    case X86_ERMS:
      return features->erms;
    case X86_F16C:
      return features->f16c;
    case X86_FMA3:
      return features->fma3;
    case X86_VAES:
      return features->vaes;
    case X86_VPCLMULQDQ:
      return features->vpclmulqdq;
    case X86_BMI1:
      return features->bmi1;
    case X86_HLE:
      return features->hle;
    case X86_BMI2:
      return features->bmi2;
    case X86_RTM:
      return features->rtm;
    case X86_RDSEED:
      return features->rdseed;
    case X86_CLFLUSHOPT:
      return features->clflushopt;
    case X86_CLWB:
      return features->clwb;
    case X86_SSE:
      return features->sse;
    case X86_SSE2:
      return features->sse2;
    case X86_SSE3:
      return features->sse3;
    case X86_SSSE3:
      return features->ssse3;
    case X86_SSE4_1:
      return features->sse4_1;
    case X86_SSE4_2:
      return features->sse4_2;
    case X86_AVX:
      return features->avx;
    case X86_AVX2:
      return features->avx2;
    case X86_AVX512F:
      return features->avx512f;
    case X86_AVX512CD:
      return features->avx512cd;
    case X86_AVX512ER:
      return features->avx512er;
    case X86_AVX512PF:
      return features->avx512pf;
    case X86_AVX512BW:
      return features->avx512bw;
    case X86_AVX512DQ:
      return features->avx512dq;
    case X86_AVX512VL:
      return features->avx512vl;
    case X86_AVX512IFMA:
      return features->avx512ifma;
    case X86_AVX512VBMI:
      return features->avx512vbmi;
    case X86_AVX512VBMI2:
      return features->avx512vbmi2;
    case X86_AVX512VNNI:
      return features->avx512vnni;
    case X86_AVX512BITALG:
      return features->avx512bitalg;
    case X86_AVX512VPOPCNTDQ:
      return features->avx512vpopcntdq;
    case X86_AVX512_4VNNIW:
      return features->avx512_4vnniw;
    case X86_AVX512_4VBMI2:
      return features->avx512_4vbmi2;
    case X86_PCLMULQDQ:
      return features->pclmulqdq;
    case X86_SMX:
      return features->smx;
    case X86_SGX:
      return features->sgx;
    case X86_CX16:
      return features->cx16;
    case X86_SHA:
      return features->sha;
    case X86_POPCNT:
      return features->popcnt;
    case X86_MOVBE:
      return features->movbe;
    case X86_RDRND:
      return features->rdrnd;
    case X86_DCA:
      return features->dca;
    case X86_SS:
      return features->ss;
    case X86_LAST_:
      break;
  }
  return false;
}

const char* GetX86FeaturesEnumName(X86FeaturesEnum value) {
  switch (value) {
    case X86_FPU:
      return "fpu";
    case X86_TSC:
      return "tsc";
    case X86_CX8:
      return "cx8";
    case X86_CLFSH:
      return "clfsh";
    case X86_MMX:
      return "mmx";
    case X86_AES:
      return "aes";
    case X86_ERMS:
      return "erms";
    case X86_F16C:
      return "f16c";
    case X86_FMA3:
      return "fma3";
    case X86_VAES:
      return "vaes";
    case X86_VPCLMULQDQ:
      return "vpclmulqdq";
    case X86_BMI1:
      return "bmi1";
    case X86_HLE:
      return "hle";
    case X86_BMI2:
      return "bmi2";
    case X86_RTM:
      return "rtm";
    case X86_RDSEED:
      return "rdseed";
    case X86_CLFLUSHOPT:
      return "clflushopt";
    case X86_CLWB:
      return "clwb";
    case X86_SSE:
      return "sse";
    case X86_SSE2:
      return "sse2";
    case X86_SSE3:
      return "sse3";
    case X86_SSSE3:
      return "ssse3";
    case X86_SSE4_1:
      return "sse4_1";
    case X86_SSE4_2:
      return "sse4_2";
    case X86_AVX:
      return "avx";
    case X86_AVX2:
      return "avx2";
    case X86_AVX512F:
      return "avx512f";
    case X86_AVX512CD:
      return "avx512cd";
    case X86_AVX512ER:
      return "avx512er";
    case X86_AVX512PF:
      return "avx512pf";
    case X86_AVX512BW:
      return "avx512bw";
    case X86_AVX512DQ:
      return "avx512dq";
    case X86_AVX512VL:
      return "avx512vl";
    case X86_AVX512IFMA:
      return "avx512ifma";
    case X86_AVX512VBMI:
      return "avx512vbmi";
    case X86_AVX512VBMI2:
      return "avx512vbmi2";
    case X86_AVX512VNNI:
      return "avx512vnni";
    case X86_AVX512BITALG:
      return "avx512bitalg";
    case X86_AVX512VPOPCNTDQ:
      return "avx512vpopcntdq";
    case X86_AVX512_4VNNIW:
      return "avx512_4vnniw";
    case X86_AVX512_4VBMI2:
      return "avx512_4vbmi2";
    case X86_PCLMULQDQ:
      return "pclmulqdq";
    case X86_SMX:
      return "smx";
    case X86_SGX:
      return "sgx";
    case X86_CX16:
      return "cx16";
    case X86_SHA:
      return "sha";
    case X86_POPCNT:
      return "popcnt";
    case X86_MOVBE:
      return "movbe";
    case X86_RDRND:
      return "rdrnd";
    case X86_DCA:
      return "dca";
    case X86_SS:
      return "ss";
    case X86_LAST_:
      break;
  }
  return "unknown_feature";
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
    case INTEL_CNL:
      return "INTEL_CNL";
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
  }
  return "unknown microarchitecture";
}
