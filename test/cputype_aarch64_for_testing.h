// Copyright 2021 Google LLC
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

#ifndef CPUFEATURES_CPUTYPE_AARCH64_FOR_TESTING_H_
#define CPUFEATURES_CPUTYPE_AARCH64_FOR_TESTING_H_

#define ID_AA64ISAR0_EL1_AES_SHIFT 4
#define ID_AA64ISAR0_EL1_SHA1_SHIFT 8
#define ID_AA64ISAR0_EL1_SHA2_SHIFT 12
#define ID_AA64ISAR0_EL1_CRC32_SHIFT 16
#define ID_AA64ISAR0_EL1_ATOMIC_SHIFT 20
#define ID_AA64ISAR0_EL1_RDM_SHIFT 28
#define ID_AA64ISAR0_EL1_SHA3_SHIFT 32
#define ID_AA64ISAR0_EL1_SM3_SHIFT 36
#define ID_AA64ISAR0_EL1_SM4_SHIFT 40
#define ID_AA64ISAR0_EL1_DP_SHIFT 44
#define ID_AA64ISAR0_EL1_FHM_SHIFT 48
#define ID_AA64ISAR0_EL1_TS_SHIFT 52
#define ID_AA64ISAR0_EL1_TLB_SHIFT 56
#define ID_AA64ISAR0_EL1_RNDR_SHIFT 60

#define ID_AA64ISAR0_EL1_MODEL(rndr, tlb, ts, fhm, dp, sm4, sm3, sha3, rdm, \
                               atomic, crc32, sha2, sha1, aes)              \
  (((rndr) << ID_AA64ISAR0_EL1_RNDR_SHIFT) |                                \
   ((tlb) << ID_AA64ISAR0_EL1_TLB_SHIFT) |                                  \
   ((ts) << ID_AA64ISAR0_EL1_TS_SHIFT) |                                    \
   ((fhm) << ID_AA64ISAR0_EL1_FHM_SHIFT) |                                  \
   ((dp) << ID_AA64ISAR0_EL1_DP_SHIFT) |                                    \
   ((sm4) << ID_AA64ISAR0_EL1_SM4_SHIFT) |                                  \
   ((sm3) << ID_AA64ISAR0_EL1_SM3_SHIFT) |                                  \
   ((sha3) << ID_AA64ISAR0_EL1_SHA3_SHIFT) |                                \
   ((rdm) << ID_AA64ISAR0_EL1_RDM_SHIFT) |                                  \
   ((atomic) << ID_AA64ISAR0_EL1_ATOMIC_SHIFT) |                            \
   ((crc32) << ID_AA64ISAR0_EL1_CRC32_SHIFT) |                              \
   ((sha2) << ID_AA64ISAR0_EL1_SHA2_SHIFT) |                                \
   ((sha1) << ID_AA64ISAR0_EL1_SHA1_SHIFT) |                                \
   ((aes) << ID_AA64ISAR0_EL1_AES_SHIFT))

#define ID_AA64ISAR0_EL1_ARM_CORTEX_A53_R3                                 \
  ID_AA64ISAR0_EL1_MODEL(0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, \
                         1UL, 1UL, 1UL, 2UL)

#endif  // CPUFEATURES_CPUTYPE_AARCH64_FOR_TESTING_H_
