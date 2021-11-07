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

#include "cpuinfo_aarch64.h"

#include <internal/cputype_aarch64.h>

#include "filesystem_for_testing.h"
#include "gtest/gtest.h"
#include "hwcaps_for_testing.h"
#include "internal/cpuid_aarch64.h"

namespace cpu_features {
namespace {

class FakeCpuAarch64 {
 public:
  uint64_t GetCpuid_MIDR_EL1() const { return _midr_el1; }

  uint64_t GetCpuid_ID_AA64ISAR0_EL1() const { return _id_aa64isar0_el1; }
  uint64_t GetCpuid_ID_AA64ISAR1_EL1() const { return _id_aa64isar1_el1; }
  uint64_t GetCpuid_ID_AA64PFR0_EL1() const { return _id_aa64pfr0_el1; }
  uint64_t GetCpuid_ID_AA64ZFR0_EL1() const { return _id_aa64zfr0_el1; }

  void SetCpuid_MIDR_EL1(uint64_t midr_el1) { _midr_el1 = midr_el1; }

  void SetCpuid_ID_AA64ISAR0_EL1(uint64_t id_aa64isar0_el1) {
    _id_aa64isar0_el1 = id_aa64isar0_el1;
  }

  void SetCpuid_ID_AA64ISAR1_EL1(uint64_t id_aa64isar1_el1) {
    _id_aa64isar1_el1 = id_aa64isar1_el1;
  }

  void SetCpuid_ID_AA64PFR0_EL1(uint64_t id_aa64pfr0_el1) {
    _id_aa64pfr0_el1 = id_aa64pfr0_el1;
  }

  void SetCpuid_ID_AA64ZFR0_EL1(uint64_t id_aa64zfr0_el1) {
    _id_aa64pfr0_el1 = id_aa64zfr0_el1;
  }

 private:
  uint64_t _midr_el1;
  uint64_t _id_aa64isar0_el1;
  uint64_t _id_aa64isar1_el1;
  uint64_t _id_aa64pfr0_el1;
  uint64_t _id_aa64zfr0_el1;
};

FakeCpuAarch64* g_fake_cpu_aarch64;

FakeCpuAarch64& cpu() {
  assert(g_fake_cpu_aarch64 != nullptr);
  return *g_fake_cpu_aarch64;
}

extern "C" uint64_t GetCpuid_MIDR_EL1() { return cpu().GetCpuid_MIDR_EL1(); }

extern "C" uint64_t GetCpuid_ID_AA64ISAR0_EL1() {
  return cpu().GetCpuid_ID_AA64ISAR0_EL1();
}

extern "C" uint64_t GetCpuid_ID_AA64ISAR1_EL1() {
  return cpu().GetCpuid_ID_AA64ISAR1_EL1();
}

extern "C" uint64_t GetCpuid_ID_AA64PFR0_EL1() {
  return cpu().GetCpuid_ID_AA64ISAR1_EL1();
}

extern "C" uint64_t GetCpuid_ID_AA64ZFR0_EL1() {
  return cpu().GetCpuid_ID_AA64ISAR1_EL1();
}

void DisableHardwareCapabilities() { SetHardwareCapabilities(0, 0); }

class CpuInfoAarch64Test : public ::testing::Test {
 protected:
  void SetUp() override {
    assert(g_fake_cpu_aarch64 == nullptr);
    g_fake_cpu_aarch64 = new FakeCpuAarch64();
  }

  void TearDown() override {
    delete g_fake_cpu_aarch64;
    g_fake_cpu_aarch64 = nullptr;
  }
};

#if defined(CPU_FEATURES_OS_LINUX) || defined(CPU_FEATURES_OS_ANDROID)
TEST_F(CpuInfoAarch64Test, FromHardwareCap_HWCAP) {
  ResetHwcaps();
  SetHardwareCapabilities(AARCH64_HWCAP_FP | AARCH64_HWCAP_AES, 0);
  GetEmptyFilesystem();  // disabling /proc/cpuinfo
  const auto info = GetAarch64Info();
  EXPECT_TRUE(info.features.fp);
  EXPECT_FALSE(info.features.asimd);
  EXPECT_FALSE(info.features.evtstrm);
  EXPECT_TRUE(info.features.aes);
  EXPECT_FALSE(info.features.pmull);
  EXPECT_FALSE(info.features.sha1);
  EXPECT_FALSE(info.features.sha2);
  EXPECT_FALSE(info.features.crc32);
  EXPECT_FALSE(info.features.atomics);
  EXPECT_FALSE(info.features.fphp);
  EXPECT_FALSE(info.features.asimdhp);
  EXPECT_FALSE(info.features.cpuid);
  EXPECT_FALSE(info.features.asimdrdm);
  EXPECT_FALSE(info.features.jscvt);
  EXPECT_FALSE(info.features.fcma);
  EXPECT_FALSE(info.features.lrcpc);
  EXPECT_FALSE(info.features.dcpop);
  EXPECT_FALSE(info.features.sha3);
  EXPECT_FALSE(info.features.sm3);
  EXPECT_FALSE(info.features.sm4);
  EXPECT_FALSE(info.features.asimddp);
  EXPECT_FALSE(info.features.sha512);
  EXPECT_FALSE(info.features.sve);
  EXPECT_FALSE(info.features.asimdfhm);
  EXPECT_FALSE(info.features.dit);
  EXPECT_FALSE(info.features.uscat);
  EXPECT_FALSE(info.features.ilrcpc);
  EXPECT_FALSE(info.features.flagm);
  EXPECT_FALSE(info.features.ssbs);
  EXPECT_FALSE(info.features.sb);
  EXPECT_FALSE(info.features.paca);
  EXPECT_FALSE(info.features.pacg);
}

TEST_F(CpuInfoAarch64Test, FromHardwareCap2_HWCAP2) {
  ResetHwcaps();
  SetHardwareCapabilities(AARCH64_HWCAP_FP,
                          AARCH64_HWCAP2_SVE2 | AARCH64_HWCAP2_BTI);
  GetEmptyFilesystem();  // disabling /proc/cpuinfo
  const auto info = GetAarch64Info();
  EXPECT_TRUE(info.features.fp);

  EXPECT_TRUE(info.features.sve2);
  EXPECT_TRUE(info.features.bti);

  EXPECT_FALSE(info.features.dcpodp);
  EXPECT_FALSE(info.features.sveaes);
  EXPECT_FALSE(info.features.svepmull);
  EXPECT_FALSE(info.features.svebitperm);
  EXPECT_FALSE(info.features.svesha3);
  EXPECT_FALSE(info.features.svesm4);
  EXPECT_FALSE(info.features.flagm2);
  EXPECT_FALSE(info.features.frint);
  EXPECT_FALSE(info.features.svei8mm);
  EXPECT_FALSE(info.features.svef32mm);
  EXPECT_FALSE(info.features.svef64mm);
  EXPECT_FALSE(info.features.svebf16);
  EXPECT_FALSE(info.features.i8mm);
  EXPECT_FALSE(info.features.bf16);
  EXPECT_FALSE(info.features.dgh);
  EXPECT_FALSE(info.features.rng);
}

TEST_F(CpuInfoAarch64Test, ARMCortexA53_PROC_CPUINFO) {
  ResetHwcaps();
  auto& fs = GetEmptyFilesystem();
  fs.CreateFile("/proc/cpuinfo",
                R"(Processor   : AArch64 Processor rev 3 (aarch64)
processor   : 0
processor   : 1
processor   : 2
processor   : 3
processor   : 4
processor   : 5
processor   : 6
processor   : 7
Features    : fp asimd evtstrm aes pmull sha1 sha2 crc32
CPU implementer : 0x41
CPU architecture: AArch64
CPU variant : 0x0
CPU part    : 0xd03
CPU revision    : 3)");
  cpu().SetCpuid_MIDR_EL1(MIDR_CORTEX_A53_R3);
  const auto info = GetAarch64Info();
  EXPECT_EQ(info.implementer, 0x41);
  EXPECT_EQ(info.variant, 0x0);
  EXPECT_EQ(info.part, 0xd03);
  EXPECT_EQ(info.revision, 3);

  EXPECT_TRUE(info.features.fp);
  EXPECT_TRUE(info.features.asimd);
  EXPECT_TRUE(info.features.evtstrm);
  EXPECT_TRUE(info.features.aes);
  EXPECT_TRUE(info.features.pmull);
  EXPECT_TRUE(info.features.sha1);
  EXPECT_TRUE(info.features.sha2);
  EXPECT_TRUE(info.features.crc32);

  EXPECT_FALSE(info.features.atomics);
  EXPECT_FALSE(info.features.fphp);
  EXPECT_FALSE(info.features.asimdhp);
  EXPECT_FALSE(info.features.cpuid);
  EXPECT_FALSE(info.features.asimdrdm);
  EXPECT_FALSE(info.features.jscvt);
  EXPECT_FALSE(info.features.fcma);
  EXPECT_FALSE(info.features.lrcpc);
  EXPECT_FALSE(info.features.dcpop);
  EXPECT_FALSE(info.features.sha3);
  EXPECT_FALSE(info.features.sm3);
  EXPECT_FALSE(info.features.sm4);
  EXPECT_FALSE(info.features.asimddp);
  EXPECT_FALSE(info.features.sha512);
  EXPECT_FALSE(info.features.sve);
  EXPECT_FALSE(info.features.asimdfhm);
  EXPECT_FALSE(info.features.dit);
  EXPECT_FALSE(info.features.uscat);
  EXPECT_FALSE(info.features.ilrcpc);
  EXPECT_FALSE(info.features.flagm);
  EXPECT_FALSE(info.features.ssbs);
  EXPECT_FALSE(info.features.sb);
  EXPECT_FALSE(info.features.paca);
  EXPECT_FALSE(info.features.pacg);
  EXPECT_FALSE(info.features.dcpodp);
  EXPECT_FALSE(info.features.sve2);
  EXPECT_FALSE(info.features.sveaes);
  EXPECT_FALSE(info.features.svepmull);
  EXPECT_FALSE(info.features.svebitperm);
  EXPECT_FALSE(info.features.svesha3);
  EXPECT_FALSE(info.features.svesm4);
  EXPECT_FALSE(info.features.flagm2);
  EXPECT_FALSE(info.features.frint);
  EXPECT_FALSE(info.features.svei8mm);
  EXPECT_FALSE(info.features.svef32mm);
  EXPECT_FALSE(info.features.svef64mm);
  EXPECT_FALSE(info.features.svebf16);
  EXPECT_FALSE(info.features.i8mm);
  EXPECT_FALSE(info.features.bf16);
  EXPECT_FALSE(info.features.dgh);
  EXPECT_FALSE(info.features.rng);
  EXPECT_FALSE(info.features.bti);
  EXPECT_FALSE(info.features.mte);
}
#endif

}  // namespace
}  // namespace cpu_features
