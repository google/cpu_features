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

uint64_t GetCpuid_MIDR_EL1();
uint64_t GetCpuid_ID_AA64ISAR0_EL1();
uint64_t GetCpuid_ID_AA64ISAR1_EL1();
uint64_t GetCpuid_ID_AA64PFR0_EL1();
uint64_t GetCpuid_ID_AA64ZFR0_EL1();

#define __stringify_1(x...) #x
#define __stringify(x...) __stringify_1(x)

#define __emit_inst(x) ".inst " __stringify((x)) "\n\t"

#define OP0_SHIFT 19
#define OP1_SHIFT 16
#define CRN_SHIFT 12
#define CRM_SHIFT 8
#define OP2_SHIFT 5

#define SYS_REG(op0, op1, crn, crm, op2)                                \
  (((op0) << OP0_SHIFT) | ((op1) << OP1_SHIFT) | ((crn) << CRN_SHIFT) | \
   ((crm) << CRM_SHIFT) | ((op2) << OP2_SHIFT))

#define SYS_MIDR_EL1 SYS_REG(3, 0, 0, 0, 0)
#define SYS_ID_AA64ZFR0_EL1 SYS_REG(3, 0, 0, 4, 4)

#define __DEFINE_MRS_MSR_S_REGNUM                                             \
  "	.irp     "                                                                \
  "num,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25, " \
  "26,27,28,29,30\n"                                                          \
  "	.equ	.L__reg_num_x\\num, \\num\n"                                         \
  "	.endr\n"                                                                  \
  "	.equ	.L__reg_num_xzr, 31\n"

#define DEFINE_MRS_S                            \
  __DEFINE_MRS_MSR_S_REGNUM                     \
  "	.macro	mrs_s, rt, sreg\n" __emit_inst( \
      0xD5200000 | (\\sreg) | (.L__reg_num_\\rt)) "	.endm\n"

#define UNDEFINE_MRS_S "	.purgem	mrs_s\n"

#define __mrs_s(v, r) \
  DEFINE_MRS_S        \
  "	mrs_s " v ", " __stringify(r) "\n" UNDEFINE_MRS_S

// For registers without architectural names.
#define READ_SYS_REG_S(r)                         \
  ({                                              \
    uint64_t __val;                               \
    asm volatile(__mrs_s("%0", r) : "=r"(__val)); \
    __val;                                        \
  })

#define READ_SYS_REG(reg)                                   \
  ({                                                        \
    uint64_t __val;                                         \
    asm volatile("mrs %0, "__stringify(reg) : "=r"(__val)); \
    __val;                                                  \
  })

CPU_FEATURES_END_CPP_NAMESPACE

#endif  // CPU_FEATURES_CPUTYPE_AARCH64_H
