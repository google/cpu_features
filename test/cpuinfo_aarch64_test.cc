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

#include "filesystem_for_testing.h"
#include "gtest/gtest.h"
#include "hwcaps_for_testing.h"

namespace cpu_features {
namespace {

#if defined(CPU_FEATURES_OS_DARWIN)

class FakeCpu {
 public:
  bool GetDarwinSysCtlByName(std::string name) const {
    return darwin_sysctlbyname_.count(name);
  }

  int GetDarwinSysCtlByNameValue(std::string name) const {
    std::map<std::string, int>::const_iterator iter =
        darwin_sysctlbynamevalue_.find(name);
    if (iter != std::end(darwin_sysctlbynamevalue_)) {
      return iter->second;
    }

    return 0;
  }

  void SetDarwinSysCtlByName(std::string name) {
    darwin_sysctlbyname_.insert(name);
  }

  void SetDarwinSysCtlByNameValue(std::string name, int value) {
    darwin_sysctlbynamevalue_[name] = value;
  }

 private:
  std::set<std::string> darwin_sysctlbyname_;
  std::map<std::string, int> darwin_sysctlbynamevalue_;
};

FakeCpu* g_fake_cpu = nullptr;

extern "C" bool GetDarwinSysCtlByName(const char* name) {
  return g_fake_cpu->GetDarwinSysCtlByName(name);
}

extern "C" int GetDarwinSysCtlByNameValue(const char* name) {
  return g_fake_cpu->GetDarwinSysCtlByNameValue(name);
}

class CpuinfoAarch64Test : public ::testing::Test {
 protected:
  void SetUp() override { g_fake_cpu = new FakeCpu(); }
  void TearDown() override { delete g_fake_cpu; }
};

TEST_F(CpuinfoAarch64Test, FromDarwinSysctlFromName) {
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.floatingpoint");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.neon");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.neon_hpfp");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.neon_fp16");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.armv8_1_atomics");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.armv8_crc32");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.armv8_2_fhm");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.armv8_2_sha512");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.armv8_2_sha3");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.amx_version");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.ucnormal_mem");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.arm64");

  g_fake_cpu->SetDarwinSysCtlByNameValue("hw.cputype", 16777228);
  g_fake_cpu->SetDarwinSysCtlByNameValue("hw.cpusubtype", 2);
  g_fake_cpu->SetDarwinSysCtlByNameValue("hw.cpu64bit", 1);
  g_fake_cpu->SetDarwinSysCtlByNameValue("hw.cpufamily", 458787763);
  g_fake_cpu->SetDarwinSysCtlByNameValue("hw.cpusubfamily", 2);

  const auto info = GetAarch64Info();

  EXPECT_EQ(info.implementer, 0x100000C);
  EXPECT_EQ(info.variant, 2);
  EXPECT_EQ(info.part, 0x1B588BB3);
  EXPECT_EQ(info.revision, 2);

  EXPECT_TRUE(info.features.fp);
  EXPECT_FALSE(info.features.asimd);
  EXPECT_FALSE(info.features.evtstrm);
  EXPECT_FALSE(info.features.aes);
  EXPECT_FALSE(info.features.pmull);
  EXPECT_FALSE(info.features.sha1);
  EXPECT_FALSE(info.features.sha2);
  EXPECT_TRUE(info.features.crc32);
  EXPECT_TRUE(info.features.atomics);
  EXPECT_TRUE(info.features.fphp);
  EXPECT_FALSE(info.features.asimdhp);
  EXPECT_FALSE(info.features.cpuid);
  EXPECT_FALSE(info.features.asimdrdm);
  EXPECT_FALSE(info.features.jscvt);
  EXPECT_FALSE(info.features.fcma);
  EXPECT_FALSE(info.features.lrcpc);
  EXPECT_FALSE(info.features.dcpop);
  EXPECT_TRUE(info.features.sha3);
  EXPECT_FALSE(info.features.sm3);
  EXPECT_FALSE(info.features.sm4);
  EXPECT_FALSE(info.features.asimddp);
  EXPECT_TRUE(info.features.sha512);
  EXPECT_FALSE(info.features.sve);
  EXPECT_TRUE(info.features.asimdfhm);
  EXPECT_FALSE(info.features.dit);
  EXPECT_FALSE(info.features.uscat);
  EXPECT_FALSE(info.features.ilrcpc);
  EXPECT_FALSE(info.features.flagm);
  EXPECT_FALSE(info.features.ssbs);
  EXPECT_FALSE(info.features.sb);
  EXPECT_FALSE(info.features.paca);
  EXPECT_FALSE(info.features.pacg);
}

#else

void DisableHardwareCapabilities() { SetHardwareCapabilities(0, 0); }

TEST(CpuinfoAarch64Test, FromHardwareCap) {
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

TEST(CpuinfoAarch64Test, FromHardwareCap2) {
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

TEST(CpuinfoAarch64Test, ARMCortexA53) {
  DisableHardwareCapabilities();
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
