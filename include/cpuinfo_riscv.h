// Copyright 2022 Google LLC
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

#ifndef CPU_FEATURES_INCLUDE_CPUINFO_RISCV_H_
#define CPU_FEATURES_INCLUDE_CPUINFO_RISCV_H_

#include "cpu_features_cache_info.h"
#include "cpu_features_macros.h"

#if !defined(CPU_FEATURES_ARCH_RISCV)
#error "Including cpuinfo_riscv.h from a non-riscv target."
#endif

CPU_FEATURES_START_CPP_NAMESPACE

typedef struct {
  // Base
  int RV32I : 1;  // Base Integer Instruction Set, 32-bit
  int RV64I : 1;  // Base Integer Instruction Set, 64-bit

  // Extension
  int M : 1;         // Standard Extension for Integer Multiplication/Division
  int A : 1;         // Standard Extension for Atomic Instructions
  int F : 1;         // Standard Extension for Single-Precision Floating-Point
  int D : 1;         // Standard Extension for Double-Precision Floating-Point
  int Q : 1;         // Standard Extension for Quad-Precision Floating-Point
  int C : 1;         // Standard Extension for Compressed Instructions
  int V : 1;         // Standard Extension for Vector Instructions
  int Zicsr : 1;     // Control and Status Register (CSR)
  int Zifencei : 1;  // Instruction-Fetch Fence
  int Zfa : 1;       // Additional Floating-Point
  int Zfh : 1;       // Half-Word Floating-Point
  int Zfhmin : 1;    // Minimal Half-Word Floating-Point
  int Zca : 1;       // Compressed Instructions A
  int Zcb : 1;       // Compressed Instructions B
  int Zcd : 1;       // Compressed Double Precision Instructions
  int Zba : 1;       // Address Generation
  int Zbb : 1;       // Bit Manipulation
  int Zbc : 1;       // Carryless Multiply
  int Zbs : 1;       // Single Bit Instructions
  int Zvbb : 1;      // Vector Bit Manipulation
  int Zvbc : 1;      // Vector Carryless Multiply
  int Zvfh : 1;      // Vector Half-Word Floating-Point
  int Zvfhmin : 1;   // Vector Minimal Half-Word Floating-Point
} RiscvFeatures;

typedef struct {
  RiscvFeatures features;
  char uarch[64];   // 0 terminated string
  char vendor[64];  // 0 terminated string
} RiscvInfo;

typedef enum {
  RISCV_RV32I,
  RISCV_RV64I,
  RISCV_M,
  RISCV_A,
  RISCV_F,
  RISCV_D,
  RISCV_Q,
  RISCV_C,
  RISCV_V,
  RISCV_Zicsr,
  RISCV_Zifencei,
  RISCV_Zfa,
  RISCV_Zfh,
  RISCV_Zfhmin,
  RISCV_Zca,
  RISCV_Zcb,
  RISCV_Zcd,
  RISCV_Zba,
  RISCV_Zbb,
  RISCV_Zbc,
  RISCV_Zbs,
  RISCV_Zvbb,
  RISCV_Zvbc,
  RISCV_Zvfh,
  RISCV_Zvfhmin,
  RISCV_LAST_,
} RiscvFeaturesEnum;

RiscvInfo GetRiscvInfo(void);
int GetRiscvFeaturesEnumValue(const RiscvFeatures* features,
                              RiscvFeaturesEnum value);
const char* GetRiscvFeaturesEnumName(RiscvFeaturesEnum);

CPU_FEATURES_END_CPP_NAMESPACE

#endif  // CPU_FEATURES_INCLUDE_CPUINFO_RISCV_H_
