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

#ifndef CPU_FEATURES_CPUINFO_CORE_AARCH64_H
#define CPU_FEATURES_CPUINFO_CORE_AARCH64_H

#include "cpu_features_macros.h"
#include "internal/cputype_aarch64.h"

CPU_FEATURES_START_CPP_NAMESPACE

///////////////////////////////////////////////////////////////////////////////
// Aarch64 info via mrs instruction
///////////////////////////////////////////////////////////////////////////////

typedef enum {
  UNKNOWN_IMPL_AARCH64,
  RESERVED,
  ARM,
  CAVIUM,
  BRCM,
  HISI,
  DEQ,
  FUJITSU,
  INFINEON,
  MOTOROLA,
  NVIDIA,
  APM,
  QCOM,
  MARVELL,
  APPLE,
  INTEL,
  AMPERE
} Aarch64Implementer;

typedef enum {
  UNKNOWN_PART_NUM_AARCH64,
  AEM_V8,
  FOUNDATION,
  CORTEX_A35,
  CORTEX_A53,
  CORTEX_A55,
  CORTEX_A57,
  CORTEX_A72,
  CORTEX_A73,
  CORTEX_A75,
  CORTEX_A76,
  CORTEX_A77,
  NEOVERSE_N1,
  POTENZA,
  THUNDERX,
  THUNDERX_81XX,
  THUNDERX_83XX,
  THUNDERX2,
  BRAHMA_B53,
  VULCAN,
  FALKOR_V1,
  FALKOR,
  KRYO,
  KRYO_2XX_GOLD,
  KRYO_2XX_SILVER,
  KRYO_3XX_SILVER,
  KRYO_4XX_GOLD,
  KRYO_4XX_SILVER,
  DENVER,
  CARMEL,
  A64FX,
  TSV110,
  M1_ICESTORM,
  M1_FIRESTORM,
  M1_ICESTORM_TONGA,
  M1_FIRESTORM_TONGA
} Aarch64PartNum;

typedef enum {
  UNKNOWN_ARCH_AARCH64,
  ARMv4,
  ARMv4T,
  ARMv5,
  ARMv5T,
  ARMv5TE,
  ARMv5TEJ,
  ARMv6,
  ARMv7,
  ARMv7F,
  ARMv7S,
  ARMv7K,
  ARMv8,
  ARMv8E
} Aarch64Architecture;

typedef struct {
  int fp;            // Floating-point.
  int fphp;          // Half-precision floating-point arithmetic.
  int i8mm;          // Int8 matrix multiplication instructions.
  int bf16;          // BFloat16 instructions.
  int gpi;           // Indicates support for an IMPLEMENTATION DEFINED.
  int gpa;           // Indicates whether the QARMA5 algorithm is implemented.
  int lrcpc;         // Support for weaker release consistency.
  int fcma;          // Floating point complex numbers.
  int jscvt;         // Support for JavaScript conversion.
  int api;           // Indicates whether an IMPLEMENTATION DEFINED algorithm.
  int apa;           // Indicates whether the QARMA5 algorithm is implemented.
  int dpb;           // Indicates support for the DC CVAP and DC CVADP.
  int ls64;          // Indicates support for LD64B and ST64B* instructions.
  int xs;            // Indicates support for the XS attribute.
  int specres;       // Indicates support for prediction invalid instructions.
  int sb;            // Speculation barrier.
  int frintts;       // Indicates support for the FRINT32Z,FRINT32X,FRINT64Z.
  int dp;            // Indicates support for Dot Product instructions.
  int dgh;           // Data Gathering Hint instruction.
  int fhm;           // Indicates support for FMLAL and FMLSL instructions.
  int ts;            // Indicates support for flag manipulation instructions.
  int tlb;           // Indicates support for Outer shareable and TLB range.
  int rndr;          // Indicates support for Random Number instructions.
  int amuv1;         // Indicates support for Activity Monitors Extension.
  int amuv1p1;       // Support for virtualization of the activity monitor
  int mpam;          // Indicates support for MPAM Extension.
  int sve;           // Scalable Vector Extension.
  int dit;           // Data independent timing.
  int sel2;          // Secure EL2.
  int ras;           // RAS Extension version.
  int rasv1p1;       // FEAT_RASv1p1.
  int double_fault;  // FEAT_DoubleFault.
  int gicv4;         // System register interface to versions 3.0 and 4.0 of the
                     // GIC CPU interface is supported.
  int gicv41;        // System register interface to version 4.1 of the GIC CPU
                     // interface is supported.
  int asimd;         // Advanced SIMD.
  int asimdfhm;      // Additional half-precision instructions.
  int el3_64;        // EL3 can be executed in AArch64 state only.
  int el3_32_64;     // EL3 can be executed in either AArch64 or AArch32 state.
  int el2_64;        // EL2 can be executed in AArch64 state only.
  int el2_32_64;     // EL2 can be executed in either AArch64 or AArch32 state.
  int el1;           // EL1 Exception level handling.
  int el0;           // EL0 Exception level handling.
  int aes;           // Hardware-accelerated Advanced Encryption Standard.
  int sha1;          // Hardware-accelerated SHA1.
  int sha2;          // Hardware-accelerated SHA2.
  int crc32;         // Hardware-accelerated CRC-32.
  int atomics;       // Armv8.1 atomic instructions.
  int rdm;           // Indicates support for SQRDMLAH and SQRDMLSH
  int sha3;          // Hardware-accelerated SHA3.
  int sm3;           // Hardware-accelerated SM3.
  int sm4;           // Hardware-accelerated SM4.
} Aarch64FeaturesCpuid;

typedef struct {
  Aarch64FeaturesCpuid features;
  Aarch64Implementer implementer;
  Aarch64PartNum part;
  Aarch64Architecture architecture;
  int variant;
  int revision;
} Aarch64InfoCpuid;

Aarch64InfoCpuid GetAarch64InfoCpuid(void);
const char* GetAarch64ImplementerString(const Aarch64InfoCpuid* info);
const char* GetAarch64PartNumString(const Aarch64InfoCpuid* info);
const char* GetAarch64ArchitectureString(const Aarch64InfoCpuid* info);

CPU_FEATURES_END_CPP_NAMESPACE

#if !defined(CPU_FEATURES_ARCH_AARCH64)
#error "Including cpuinfo_core_aarch64.h from a non-aarch64 target."
#endif

#endif  // CPU_FEATURES_CPUINFO_CORE_AARCH64_H
