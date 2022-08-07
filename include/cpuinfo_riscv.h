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

//#if !defined(CPU_FEATURES_ARCH_RISCV)
//#error "Including cpuinfo_riscv.h from a non-riscv target."
//#endif

CPU_FEATURES_START_CPP_NAMESPACE

typedef struct {
    int riscv32 : 1;  // Is a 32 bit architecture
    int riscv64 : 1;  // Is a 64 bit architecture
    int riscv128 : 1; // Is a 128 bit architecture
    int a : 1;        // Atomic Instructions
    int c : 1;        // Compressed Instructions
    int d : 1;        // Double Precision Floating Point
    int e : 1;        // Base Integer Reduced
    int f : 1;        // Single Precision Floating Point
    int i : 1;        // Base Integer
    int m : 1;        // Integer Multiplication and Division
    int v : 1;        // Vector Operations
    int q : 1;        // Quad Precision Floating Point
} RiscvFeatures;

typedef enum 
{
    RISCV_UNKNOWN,
    N25,
    CODIX_BK3,
    CODIX_BK5,
    CODIX_BK7,
    ET_MINION, 
    ET_MAXION,
    BOTTLEROCKET,
    MI_V,
    NV_RISCV,
    E31,
    E51,
    E54_MC,
    _7_SERIES,
    VANILLA_5,
    ROCKET,
    RISCV_MICROARCHITECTURE_LAST_
} RiscvMicroarchitecture;

typedef struct {
    RiscvFeatures          features;
    RiscvMicroarchitecture uarch;
    //Address space size (32, 64, 128)?
} RiscvInfo;

typedef enum {
    RISCV_32,
    RISCV_64,
    RISCV_128,
    RISCV_A,
    RISCV_C,
    RISCV_D,
    RISCV_E,
    RISCV_F,
    RISCV_I,
    RISCV_M,
    RISCV_V,
    RISCV_Q,
    RISCV_LAST_,
} RiscvFeaturesEnum;

RiscvInfo GetRiscvInfo(void);

CPU_FEATURES_END_CPP_NAMESPACE

#endif  // CPU_FEATURES_INCLUDE_CPUINFO_RISCV_H_

