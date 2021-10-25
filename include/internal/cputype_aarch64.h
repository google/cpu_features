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

#ifndef CPU_FEATURES_CPUTYPE_AARCH64_H
#define CPU_FEATURES_CPUTYPE_AARCH64_H

#include "cpu_features_macros.h"
#include "stdint.h"

CPU_FEATURES_START_CPP_NAMESPACE

#define __stringify_1(x...) #x
#define __stringify(x...) __stringify_1(x)

#define CPU_ARCH_ARMv4 0x1
#define CPU_ARCH_ARMv4T 0x2
#define CPU_ARCH_ARMv5 0x3
#define CPU_ARCH_ARMv5T 0x4
#define CPU_ARCH_ARMv5TE 0x5
#define CPU_ARCH_ARMv5TEJ 0x6
#define CPU_ARCH_ARMv6 0x7
#define CPU_ARCH_ARMv7 0x8
#define CPU_ARCH_ARMv7F 0x9
#define CPU_ARCH_ARMv7S 0xA
#define CPU_ARCH_ARMv7K 0xB
#define CPU_ARCH_ARMv8 0xC
#define CPU_ARCH_ARMv8E 0xD

// Special code indicating we need to look somewhere else for
// the architecture version.
#define CPU_ARCH_ARM_EXTENDED 0xF

#define ARM_CPU_IMP_RESERVED 0x00
#define ARM_CPU_IMP_ARM 0x41
#define ARM_CPU_IMP_BRCM 0x42
#define ARM_CPU_IMP_CAVIUM 0x43
#define ARM_CPU_IMP_DEQ 0x44
#define ARM_CPU_IMP_FUJITSU 0x46
#define ARM_CPU_IMP_HISI 0x48
#define ARM_CPU_IMP_INFINEON 0x49
#define ARM_CPU_IMP_MOTOROLA 0x4D
#define ARM_CPU_IMP_NVIDIA 0x4E
#define ARM_CPU_IMP_APM 0x50
#define ARM_CPU_IMP_QCOM 0x51
#define ARM_CPU_IMP_MARVELL 0x56
#define ARM_CPU_IMP_APPLE 0x61
#define ARM_CPU_IMP_INTEL 0x69
#define ARM_CPU_IMP_AMPERE 0xC0

#define ARM_CPU_PART_AEM_V8 0xD0F
#define ARM_CPU_PART_FOUNDATION 0xD00
#define ARM_CPU_PART_CORTEX_A57 0xD07
#define ARM_CPU_PART_CORTEX_A72 0xD08
#define ARM_CPU_PART_CORTEX_A53 0xD03
#define ARM_CPU_PART_CORTEX_A73 0xD09
#define ARM_CPU_PART_CORTEX_A75 0xD0A
#define ARM_CPU_PART_CORTEX_A35 0xD04
#define ARM_CPU_PART_CORTEX_A55 0xD05
#define ARM_CPU_PART_CORTEX_A76 0xD0B
#define ARM_CPU_PART_NEOVERSE_N1 0xD0C
#define ARM_CPU_PART_CORTEX_A77 0xD0D

#define APM_CPU_PART_POTENZA 0x000

#define CAVIUM_CPU_PART_THUNDERX 0x0A1
#define CAVIUM_CPU_PART_THUNDERX_81XX 0x0A2
#define CAVIUM_CPU_PART_THUNDERX_83XX 0x0A3
#define CAVIUM_CPU_PART_THUNDERX2 0x0AF

#define BRCM_CPU_PART_BRAHMA_B53 0x100
#define BRCM_CPU_PART_VULCAN 0x516

#define QCOM_CPU_PART_FALKOR_V1 0x800
#define QCOM_CPU_PART_FALKOR 0xC00
#define QCOM_CPU_PART_KRYO 0x200
#define QCOM_CPU_PART_KRYO_2XX_GOLD 0x800
#define QCOM_CPU_PART_KRYO_2XX_SILVER 0x801
#define QCOM_CPU_PART_KRYO_3XX_SILVER 0x803
#define QCOM_CPU_PART_KRYO_4XX_GOLD 0x804
#define QCOM_CPU_PART_KRYO_4XX_SILVER 0x805

#define NVIDIA_CPU_PART_DENVER 0x003
#define NVIDIA_CPU_PART_CARMEL 0x004

#define FUJITSU_CPU_PART_A64FX 0x001

#define HISI_CPU_PART_TSV110 0xD01

#define APPLE_CPU_PART_M1_ICESTORM 0x020
#define APPLE_CPU_PART_M1_FIRESTORM 0x021
#define APPLE_CPU_PART_M1_ICESTORM_TONGA 0x022
#define APPLE_CPU_PART_M1_FIRESTORM_TONGA 0x023

#define READ_MRS(reg)                                       \
  ({                                                        \
    uint64_t __val;                                         \
    asm volatile("mrs %0, "__stringify(reg) : "=r"(__val)); \
    __val;                                                  \
  })

uint64_t GetCpuid_MIDR_EL1();
uint64_t GetCpuid_AA64ISAR0_EL1();
uint64_t GetCpuid_AA64ISAR1_EL1();
uint64_t GetCpuid_AA64PFR0_EL1();

CPU_FEATURES_END_CPP_NAMESPACE

#if !defined(CPU_FEATURES_ARCH_AARCH64)
#error "Including internal/cputype_aarch64.h from a non-aarch64 target."
#endif

#endif  // CPU_FEATURES_CPUTYPE_AARCH64_H
