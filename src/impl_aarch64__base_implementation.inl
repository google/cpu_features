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

#include "cpuinfo_aarch64.h"
#include "define_cpuid_aarch64.inl"
#include "internal/bit_utils.h"
#include "internal/cpuid_aarch64.h"

#ifndef CPU_FEATURES_ARCH_AARCH64
#error "Cannot compile cpuinfo_aarch64 on a non aarch64 platform."
#endif

///////////////////////////////////////////////////////////////////////////////
// Aarch64 info via mrs instruction
///////////////////////////////////////////////////////////////////////////////

#ifdef CPU_FEATURES_MOCK_CPUID_AARCH64
// Implementation will be provided by test/cpuinfo_aarch64_test.cc.
#else
uint64_t GetCpuid_MIDR_EL1() { return READ_SYS_REG_S(SYS_MIDR_EL1); }

uint64_t GetCpuid_ID_AA64PFR0_EL1() {
  return READ_SYS_REG_S(SYS_ID_AA64PFR0_EL1);
}

uint64_t GetCpuid_ID_AA64ISAR0_EL1() {
  return READ_SYS_REG_S(SYS_ID_AA64ISAR0_EL1);
}

uint64_t GetCpuid_ID_AA64ISAR1_EL1() {
  return READ_SYS_REG_S(SYS_ID_AA64ISAR1_EL1);
}

uint64_t GetCpuid_ID_AA64ZFR0_EL1() {
  return READ_SYS_REG_S(SYS_ID_AA64ZFR0_EL1);
}

uint64_t GetCpuid_ID_AA64MMFR2_EL1() {
  return READ_SYS_REG_S(SYS_ID_AA64MMFR2_EL1);
}

uint64_t GetCpuid_ID_AA64PFR1_EL1() {
  return READ_SYS_REG_S(SYS_ID_AA64PFR1_EL1);
}
#endif

#define ID_SCHEME(reg, msb, lsb) ExtractBitRange((reg), (msb), (lsb)) >= 1

// This function have to be implemented by the OS or
// can use base implementation DetectFeaturesBase.
static void DetectFeatures(Aarch64Info* info);

static void DetectFeaturesBase(Aarch64Info* info) {
  // ID_AA64PFR0_EL1
  const uint64_t pfr0 = GetCpuid_ID_AA64PFR0_EL1();

  info->features.sve = ID_SCHEME(pfr0, 35, 32);
  info->features.dit = ID_SCHEME(pfr0, 51, 48);

  const uint64_t fp = ExtractBitRange(pfr0, 19, 16);

  // fp_value = 0b0000:
  // Floating-point is implemented, and includes support for:
  //  * Single-precision and double-precision floating-point types.
  //  * Conversions between single-precision and half-precision data types,
  //  and double-precision and half-precision data types.
  if (fp == 0) info->features.fp = 1;

  // fp_value = 0b0001:
  // As for 0b0000, and also includes support for half-precision
  // floating-point arithmetic.
  if (fp == 1) {
    info->features.fp = 1;
    info->features.fphp = 1;
  }

  const uint64_t asimd = ExtractBitRange(pfr0, 23, 20);

  // Advanced SIMD is implemented, including support for the following SISD and
  // SIMD operations:
  //  * Integer byte, halfword, word and doubleword element operations.
  //  * Single-precision and double-precision floating-point arithmetic.
  //  * Conversions between single-precision and half-precision data types,
  // and double-precision and half-precision data types.
  if (asimd == 0) info->features.asimd = 1;

  // asimd = 0b0001:
  // As for 0b0000, and also includes support for half-precision
  // floating-point arithmetic.
  if (asimd == 1) {
    info->features.asimd = 1;
    info->features.asimdhp = 1;
  }

  // ID_AA64PFR1_EL1
  const uint64_t pfr1 = GetCpuid_ID_AA64PFR1_EL1();
  info->features.bti = ID_SCHEME(pfr1, 3, 0);

  const uint64_t ssbs = ExtractBitRange(pfr1, 7, 4);
  if (ssbs >= 2) info->features.ssbs = 1;

  info->features.bti = ID_SCHEME(pfr1, 11, 8);

  // ID_AA64ISAR0_EL1
  const uint64_t isa0 = GetCpuid_ID_AA64ISAR0_EL1();
  const uint64_t aes = ExtractBitRange(isa0, 7, 4);
  if (aes >= 1) info->features.aes = 1;
  if (aes >= 2) info->features.pmull = 1;

  info->features.sha1 = ID_SCHEME(isa0, 11, 8);

  const uint64_t sha2 = ExtractBitRange(isa0, 15, 12);
  if (sha2 >= 1) info->features.sha2 = 1;
  if (sha2 >= 2) info->features.sha512 = 1;

  info->features.crc32 = ID_SCHEME(isa0, 19, 16);
  info->features.atomics = ID_SCHEME(isa0, 23, 20);
  info->features.asimdrdm = ID_SCHEME(isa0, 31, 28);
  info->features.sha3 = ID_SCHEME(isa0, 35, 32);
  info->features.sm3 = ID_SCHEME(isa0, 39, 36);
  info->features.sm4 = ID_SCHEME(isa0, 43, 40);
  info->features.asimddp = ID_SCHEME(isa0, 47, 44);
  info->features.asimdfhm = ID_SCHEME(isa0, 51, 48);

  const uint64_t ts = ExtractBitRange(isa0, 55, 52);
  if (ts >= 1) info->features.flagm = 1;
  if (ts >= 2) info->features.flagm2 = 1;

  info->features.rng = ID_SCHEME(isa0, 63, 60);

  // ID_AA64ISAR1_EL1
  const uint64_t isa1 = GetCpuid_ID_AA64ISAR1_EL1();
  const uint64_t dpb = ExtractBitRange(isa1, 3, 0);
  if (dpb >= 1) info->features.dcpop = 1;
  if (dpb >= 2) info->features.dcpodp = 1;

  const uint64_t apa = ExtractBitRange(isa1, 7, 4);
  const uint64_t api = ExtractBitRange(isa1, 11, 8);
  if (apa >= 1 || api >= 1) info->features.paca = 1;

  info->features.jscvt = ID_SCHEME(isa1, 15, 12);
  info->features.fcma = ID_SCHEME(isa1, 19, 16);

  const uint64_t lrcpc = ExtractBitRange(isa1, 23, 20);
  if (lrcpc >= 1) info->features.lrcpc = 1;
  if (lrcpc >= 2) info->features.ilrcpc = 1;

  const uint64_t gpa = ExtractBitRange(isa1, 27, 24);
  const uint64_t gpi = ExtractBitRange(isa1, 31, 28);
  if (gpa >= 1 || gpi >= 1) info->features.pacg = 1;

  info->features.frint = ID_SCHEME(isa1, 35, 32);
  info->features.sb = ID_SCHEME(isa1, 39, 36);
  info->features.bf16 = ID_SCHEME(isa1, 47, 44);
  info->features.dgh = ID_SCHEME(isa1, 51, 48);
  info->features.i8mm = ID_SCHEME(isa1, 55, 52);

  // ID_AA64ZFR0_EL1
  if (info->features.sve) {
    const uint64_t zfr0 = GetCpuid_ID_AA64ZFR0_EL1();
    info->features.sve2 = ID_SCHEME(zfr0, 0, 3);
    info->features.svebitperm = ID_SCHEME(zfr0, 19, 16);
    info->features.svebf16 = ID_SCHEME(zfr0, 23, 20);
    info->features.svesha3 = ID_SCHEME(zfr0, 35, 32);
    info->features.svesm4 = ID_SCHEME(zfr0, 43, 40);
    info->features.svei8mm = ID_SCHEME(zfr0, 47, 44);
    info->features.svef32mm = ID_SCHEME(zfr0, 55, 52);
    info->features.svef64mm = ID_SCHEME(zfr0, 59, 56);

    const uint64_t sveaes = ExtractBitRange(zfr0, 7, 4);
    if (sveaes >= 1) info->features.sveaes = 1;
    if (sveaes >= 2) info->features.svepmull = 1;
  }

  // ID_AA64MMFR2_EL1
  const uint64_t mmfr2 = GetCpuid_ID_AA64MMFR2_EL1();
  info->features.uscat = ID_SCHEME(mmfr2, 35, 32);
}

static const Aarch64Info kEmptyAarch64Info;

Aarch64Info GetAarch64Info(void) {
  Aarch64Info info = kEmptyAarch64Info;
  const uint64_t midr = GetCpuid_MIDR_EL1();
  if (midr) {
    info.features.cpuid = 1;
    info.implementer = ExtractBitRange(midr, 31, 24);
    info.variant = ExtractBitRange(midr, 23, 20);
    info.part = ExtractBitRange(midr, 15, 4);
    info.revision = ExtractBitRange(midr, 3, 0);
    DetectFeatures(&info);
  }
  return info;
}
