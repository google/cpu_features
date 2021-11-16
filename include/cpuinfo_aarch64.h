// Copyright 2017 Google LLC
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

#ifndef CPU_FEATURES_INCLUDE_CPUINFO_AARCH64_H_
#define CPU_FEATURES_INCLUDE_CPUINFO_AARCH64_H_

#include "cpu_features_cache_info.h"
#include "cpu_features_macros.h"

CPU_FEATURES_START_CPP_NAMESPACE

typedef struct {
  int fp : 1;          // Floating-point.
  int asimd : 1;       // Advanced SIMD.
  int evtstrm : 1;     // Generic timer generated events.
  int aes : 1;         // Hardware-accelerated Advanced Encryption Standard.
  int pmull : 1;       // Polynomial multiply long.
  int sha1 : 1;        // Hardware-accelerated SHA1.
  int sha2 : 1;        // Hardware-accelerated SHA2-256.
  int crc32 : 1;       // Hardware-accelerated CRC-32.
  int atomics : 1;     // Armv8.1 atomic instructions.
  int fphp : 1;        // Half-precision floating point support.
  int asimdhp : 1;     // Advanced SIMD half-precision support.
  int cpuid : 1;       // Access to certain ID registers.
  int asimdrdm : 1;    // Rounding Double Multiply Accumulate/Subtract.
  int jscvt : 1;       // Support for JavaScript conversion.
  int fcma : 1;        // Floating point complex numbers.
  int lrcpc : 1;       // Support for weaker release consistency.
  int dcpop : 1;       // Data persistence writeback.
  int sha3 : 1;        // Hardware-accelerated SHA3.
  int sm3 : 1;         // Hardware-accelerated SM3.
  int sm4 : 1;         // Hardware-accelerated SM4.
  int asimddp : 1;     // Dot product instruction.
  int sha512 : 1;      // Hardware-accelerated SHA512.
  int sve : 1;         // Scalable Vector Extension.
  int asimdfhm : 1;    // Additional half-precision instructions.
  int dit : 1;         // Data independent timing.
  int uscat : 1;       // Unaligned atomics support.
  int ilrcpc : 1;      // Additional support for weaker release consistency.
  int flagm : 1;       // Flag manipulation instructions.
  int ssbs : 1;        // Speculative Store Bypass Safe PSTATE bit.
  int sb : 1;          // Speculation barrier.
  int paca : 1;        // Address authentication.
  int pacg : 1;        // Generic authentication.
  int dcpodp : 1;      // Data cache clean to point of persistence.
  int sve2 : 1;        // Scalable Vector Extension (version 2).
  int sveaes : 1;      // SVE AES instructions.
  int svepmull : 1;    // SVE polynomial multiply long instructions.
  int svebitperm : 1;  // SVE bit permute instructions.
  int svesha3 : 1;     // SVE SHA3 instructions.
  int svesm4 : 1;      // SVE SM4 instructions.
  int flagm2 : 1;      // Additional flag manipulation instructions.
  int frint : 1;       // Floating point to integer rounding.
  int svei8mm : 1;     // SVE Int8 matrix multiplication instructions.
  int svef32mm : 1;    // SVE FP32 matrix multiplication instruction.
  int svef64mm : 1;    // SVE FP64 matrix multiplication instructions.
  int svebf16 : 1;     // SVE BFloat16 instructions.
  int i8mm : 1;        // Int8 matrix multiplication instructions.
  int bf16 : 1;        // BFloat16 instructions.
  int dgh : 1;         // Data Gathering Hint instruction.
  int rng : 1;         // True random number generator support.
  int bti : 1;         // Branch target identification.
  int mte : 1;         // Memory tagging extension.

  // Make sure to update Aarch64FeaturesEnum below if you add a field here.
} Aarch64Features;

typedef struct {
  Aarch64Features features;
  int implementer;
  int variant;
  int part;
  int revision;
} Aarch64Info;

Aarch64Info GetAarch64Info(void);

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

typedef enum {
  AARCH64_FP,
  AARCH64_ASIMD,
  AARCH64_EVTSTRM,
  AARCH64_AES,
  AARCH64_PMULL,
  AARCH64_SHA1,
  AARCH64_SHA2,
  AARCH64_CRC32,
  AARCH64_ATOMICS,
  AARCH64_FPHP,
  AARCH64_ASIMDHP,
  AARCH64_CPUID,
  AARCH64_ASIMDRDM,
  AARCH64_JSCVT,
  AARCH64_FCMA,
  AARCH64_LRCPC,
  AARCH64_DCPOP,
  AARCH64_SHA3,
  AARCH64_SM3,
  AARCH64_SM4,
  AARCH64_ASIMDDP,
  AARCH64_SHA512,
  AARCH64_SVE,
  AARCH64_ASIMDFHM,
  AARCH64_DIT,
  AARCH64_USCAT,
  AARCH64_ILRCPC,
  AARCH64_FLAGM,
  AARCH64_SSBS,
  AARCH64_SB,
  AARCH64_PACA,
  AARCH64_PACG,
  AARCH64_DCPODP,
  AARCH64_SVE2,
  AARCH64_SVEAES,
  AARCH64_SVEPMULL,
  AARCH64_SVEBITPERM,
  AARCH64_SVESHA3,
  AARCH64_SVESM4,
  AARCH64_FLAGM2,
  AARCH64_FRINT,
  AARCH64_SVEI8MM,
  AARCH64_SVEF32MM,
  AARCH64_SVEF64MM,
  AARCH64_SVEBF16,
  AARCH64_I8MM,
  AARCH64_BF16,
  AARCH64_DGH,
  AARCH64_RNG,
  AARCH64_BTI,
  AARCH64_MTE,
  AARCH64_LAST_,
} Aarch64FeaturesEnum;

int GetAarch64FeaturesEnumValue(const Aarch64Features* features,
                                Aarch64FeaturesEnum value);

const char* GetAarch64FeaturesEnumName(Aarch64FeaturesEnum);

CPU_FEATURES_END_CPP_NAMESPACE

#if !defined(CPU_FEATURES_ARCH_AARCH64)
#error "Including cpuinfo_aarch64.h from a non-aarch64 target."
#endif

#endif  // CPU_FEATURES_INCLUDE_CPUINFO_AARCH64_H_
