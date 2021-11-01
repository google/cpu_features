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

#include <stdint.h>

#include "cpu_features_cache_info.h"
#include "cpu_features_macros.h"

CPU_FEATURES_START_CPP_NAMESPACE

typedef struct {
  uint8_t fp;          // Floating-point.
  uint8_t asimd;       // Advanced SIMD.
  uint8_t evtstrm;     // Generic timer generated events.
  uint8_t aes;         // Hardware-accelerated Advanced Encryption Standard.
  uint8_t pmull;       // Polynomial multiply long.
  uint8_t sha1;        // Hardware-accelerated SHA1.
  uint8_t sha2;        // Hardware-accelerated SHA2-256.
  uint8_t crc32;       // Hardware-accelerated CRC-32.
  uint8_t atomics;     // Armv8.1 atomic instructions.
  uint8_t fphp;        // Half-precision floating point support.
  uint8_t asimdhp;     // Advanced SIMD half-precision support.
  uint8_t cpuid;       // Access to certain ID registers.
  uint8_t asimdrdm;    // Rounding Double Multiply Accumulate/Subtract.
  uint8_t jscvt;       // Support for JavaScript conversion.
  uint8_t fcma;        // Floating point complex numbers.
  uint8_t lrcpc;       // Support for weaker release consistency.
  uint8_t dcpop;       // Data persistence writeback.
  uint8_t sha3;        // Hardware-accelerated SHA3.
  uint8_t sm3;         // Hardware-accelerated SM3.
  uint8_t sm4;         // Hardware-accelerated SM4.
  uint8_t asimddp;     // Dot product instruction.
  uint8_t sha512;      // Hardware-accelerated SHA512.
  uint8_t sve;         // Scalable Vector Extension.
  uint8_t asimdfhm;    // Additional half-precision instructions.
  uint8_t dit;         // Data independent timing.
  uint8_t uscat;       // Unaligned atomics support.
  uint8_t ilrcpc;      // Additional support for weaker release consistency.
  uint8_t flagm;       // Flag manipulation instructions.
  uint8_t ssbs;        // Speculative Store Bypass Safe PSTATE bit.
  uint8_t sb;          // Speculation barrier.
  uint8_t paca;        // Address authentication.
  uint8_t pacg;        // Generic authentication.
  uint8_t dcpodp;      // Data cache clean to point of persistence.
  uint8_t sve2;        // Scalable Vector Extension (version 2).
  uint8_t sveaes;      // SVE AES instructions.
  uint8_t svepmull;    // SVE polynomial multiply long instructions.
  uint8_t svebitperm;  // SVE bit permute instructions.
  uint8_t svesha3;     // SVE SHA3 instructions.
  uint8_t svesm4;      // SVE SM4 instructions.
  uint8_t flagm2;      // Additional flag manipulation instructions.
  uint8_t frint;       // Floating point to integer rounding.
  uint8_t svei8mm;     // SVE Int8 matrix multiplication instructions.
  uint8_t svef32mm;    // SVE FP32 matrix multiplication instruction.
  uint8_t svef64mm;    // SVE FP64 matrix multiplication instructions.
  uint8_t svebf16;     // SVE BFloat16 instructions.
  uint8_t i8mm;        // Int8 matrix multiplication instructions.
  uint8_t bf16;        // BFloat16 instructions.
  uint8_t dgh;         // Data Gathering Hint instruction.
  uint8_t rng;         // True random number generator support.
  uint8_t bti;         // Branch target identification.
  uint8_t mte;         // Memory tagging extension.

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
