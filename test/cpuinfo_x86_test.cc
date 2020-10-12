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

#include "cpuinfo_x86.h"

#include <cassert>
#include <cstdio>
#include <map>
#include <set>
#if defined(CPU_FEATURES_OS_WINDOWS)
#include <windows.h>  // IsProcessorFeaturePresent
#endif                // CPU_FEATURES_OS_WINDOWS

#include "filesystem_for_testing.h"
#include "gtest/gtest.h"
#include "internal/cpuid_x86.h"

namespace cpu_features {

class FakeCpu {
 public:
  Leaf GetCpuidLeaf(uint32_t leaf_id, int ecx) const {
    const auto itr = cpuid_leaves_.find(std::make_pair(leaf_id, ecx));
    if (itr != cpuid_leaves_.end()) {
      return itr->second;
    }
    return {0, 0, 0, 0};
  }

  uint32_t GetXCR0Eax() const { return xcr0_eax_; }

  void SetLeaves(std::map<std::pair<uint32_t, int>, Leaf> configuration) {
    cpuid_leaves_ = std::move(configuration);
  }

  void SetOsBackupsExtendedRegisters(bool os_backups_extended_registers) {
    xcr0_eax_ = os_backups_extended_registers ? -1 : 0;
  }

#if defined(CPU_FEATURES_OS_DARWIN)
  bool GetDarwinSysCtlByName(std::string name) const {
    return darwin_sysctlbyname_.count(name);
  }

  void SetDarwinSysCtlByName(std::string name) {
    darwin_sysctlbyname_.insert(name);
  }
#endif  // CPU_FEATURES_OS_DARWIN

#if defined(CPU_FEATURES_OS_WINDOWS)
  bool GetWindowsIsProcessorFeaturePresent(DWORD ProcessorFeature) {
    return windows_isprocessorfeaturepresent_.count(ProcessorFeature);
  }

  void SetWindowsIsProcessorFeaturePresent(DWORD ProcessorFeature) {
    windows_isprocessorfeaturepresent_.insert(ProcessorFeature);
  }
#endif  // CPU_FEATURES_OS_WINDOWS

 private:
  std::map<std::pair<uint32_t, int>, Leaf> cpuid_leaves_;
#if defined(CPU_FEATURES_OS_DARWIN)
  std::set<std::string> darwin_sysctlbyname_;
#endif  // CPU_FEATURES_OS_DARWIN
#if defined(CPU_FEATURES_OS_WINDOWS)
  std::set<DWORD> windows_isprocessorfeaturepresent_;
#endif  // CPU_FEATURES_OS_WINDOWS
  uint32_t xcr0_eax_;
};

FakeCpu* g_fake_cpu = nullptr;

extern "C" Leaf GetCpuidLeaf(uint32_t leaf_id, int ecx) {
  return g_fake_cpu->GetCpuidLeaf(leaf_id, ecx);
}

extern "C" uint32_t GetXCR0Eax(void) { return g_fake_cpu->GetXCR0Eax(); }

#if defined(CPU_FEATURES_OS_DARWIN)
extern "C" bool GetDarwinSysCtlByName(const char* name) {
  return g_fake_cpu->GetDarwinSysCtlByName(name);
}
#endif  // CPU_FEATURES_OS_DARWIN

#if defined(CPU_FEATURES_OS_WINDOWS)
extern "C" bool GetWindowsIsProcessorFeaturePresent(DWORD ProcessorFeature) {
  return g_fake_cpu->GetWindowsIsProcessorFeaturePresent(ProcessorFeature);
}
#endif  // CPU_FEATURES_OS_WINDOWS

namespace {

class CpuidX86Test : public ::testing::Test {
 protected:
  void SetUp() override { g_fake_cpu = new FakeCpu(); }
  void TearDown() override { delete g_fake_cpu; }
};

TEST_F(CpuidX86Test, SandyBridge) {
  g_fake_cpu->SetOsBackupsExtendedRegisters(true);
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x0000000D, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000206A6, 0x00100800, 0x1F9AE3BF, 0xBFEBFBFF}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
  });
  const auto info = GetX86Info();
  EXPECT_STREQ(info.vendor, "GenuineIntel");
  EXPECT_EQ(info.family, 0x06);
  EXPECT_EQ(info.model, 0x02A);
  EXPECT_EQ(info.stepping, 0x06);
  // Leaf 7 is zeroed out so none of the Leaf 7 flags are set.
  const auto features = info.features;
  EXPECT_FALSE(features.erms);
  EXPECT_FALSE(features.avx2);
  EXPECT_FALSE(features.avx512f);
  EXPECT_FALSE(features.avx512cd);
  EXPECT_FALSE(features.avx512er);
  EXPECT_FALSE(features.avx512pf);
  EXPECT_FALSE(features.avx512bw);
  EXPECT_FALSE(features.avx512dq);
  EXPECT_FALSE(features.avx512vl);
  EXPECT_FALSE(features.avx512ifma);
  EXPECT_FALSE(features.avx512vbmi);
  EXPECT_FALSE(features.avx512vbmi2);
  EXPECT_FALSE(features.avx512vnni);
  EXPECT_FALSE(features.avx512bitalg);
  EXPECT_FALSE(features.avx512vpopcntdq);
  EXPECT_FALSE(features.avx512_4vnniw);
  EXPECT_FALSE(features.avx512_4fmaps);
  // All old cpu features should be set.
  EXPECT_TRUE(features.aes);
  EXPECT_TRUE(features.ssse3);
  EXPECT_TRUE(features.sse4_1);
  EXPECT_TRUE(features.sse4_2);
  EXPECT_TRUE(features.avx);
  EXPECT_FALSE(features.sha);
  EXPECT_TRUE(features.popcnt);
  EXPECT_FALSE(features.movbe);
  EXPECT_FALSE(features.rdrnd);
}

const int KiB = 1024;
const int MiB = 1024 * KiB;

TEST_F(CpuidX86Test, SandyBridgeTestOsSupport) {
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x0000000D, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000206A6, 0x00100800, 0x1F9AE3BF, 0xBFEBFBFF}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
  });
  // avx is disabled if os does not support backing up ymm registers.
  g_fake_cpu->SetOsBackupsExtendedRegisters(false);
  EXPECT_FALSE(GetX86Info().features.avx);
  // avx is disabled if os does not support backing up ymm registers.
  g_fake_cpu->SetOsBackupsExtendedRegisters(true);
  EXPECT_TRUE(GetX86Info().features.avx);
}

TEST_F(CpuidX86Test, SkyLake) {
  g_fake_cpu->SetOsBackupsExtendedRegisters(true);
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x00000016, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000406E3, 0x00100800, 0x7FFAFBBF, 0xBFEBFBFF}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x029C67AF, 0x00000000, 0x00000000}},
  });
  const auto info = GetX86Info();
  EXPECT_STREQ(info.vendor, "GenuineIntel");
  EXPECT_EQ(info.family, 0x06);
  EXPECT_EQ(info.model, 0x04E);
  EXPECT_EQ(info.stepping, 0x03);
  EXPECT_EQ(GetX86Microarchitecture(&info), X86Microarchitecture::INTEL_SKL);
}

TEST_F(CpuidX86Test, Branding) {
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x00000016, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000406E3, 0x00100800, 0x7FFAFBBF, 0xBFEBFBFF}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x029C67AF, 0x00000000, 0x00000000}},
      {{0x80000000, 0}, Leaf{0x80000008, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000001, 0}, Leaf{0x00000000, 0x00000000, 0x00000121, 0x2C100000}},
      {{0x80000002, 0}, Leaf{0x65746E49, 0x2952286C, 0x726F4320, 0x4D542865}},
      {{0x80000003, 0}, Leaf{0x37692029, 0x3035362D, 0x43205530, 0x40205550}},
      {{0x80000004, 0}, Leaf{0x352E3220, 0x7A484730, 0x00000000, 0x00000000}},
  });
  char brand_string[49];
  FillX86BrandString(brand_string);
  EXPECT_STREQ(brand_string, "Intel(R) Core(TM) i7-6500U CPU @ 2.50GHz");
}

TEST_F(CpuidX86Test, KabyLakeCache) {
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x00000016, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000406E3, 0x00100800, 0x7FFAFBBF, 0xBFEBFBFF}},
      {{0x00000004, 0}, Leaf{0x1C004121, 0x01C0003F, 0x0000003F, 0x00000000}},
      {{0x00000004, 1}, Leaf{0x1C004122, 0x01C0003F, 0x0000003F, 0x00000000}},
      {{0x00000004, 2}, Leaf{0x1C004143, 0x00C0003F, 0x000003FF, 0x00000000}},
      {{0x00000004, 3}, Leaf{0x1C03C163, 0x02C0003F, 0x00001FFF, 0x00000002}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x029C67AF, 0x00000000, 0x00000000}},
      {{0x80000000, 0}, Leaf{0x80000008, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000001, 0}, Leaf{0x00000000, 0x00000000, 0x00000121, 0x2C100000}},
      {{0x80000002, 0}, Leaf{0x65746E49, 0x2952286C, 0x726F4320, 0x4D542865}},
      {{0x80000003, 0}, Leaf{0x37692029, 0x3035362D, 0x43205530, 0x40205550}},
  });
  const auto info = GetX86CacheInfo();
  EXPECT_EQ(info.size, 4);
  EXPECT_EQ(info.levels[0].level, 1);
  EXPECT_EQ(info.levels[0].cache_type, 1);
  EXPECT_EQ(info.levels[0].cache_size, 32 * KiB);
  EXPECT_EQ(info.levels[0].ways, 8);
  EXPECT_EQ(info.levels[0].line_size, 64);
  EXPECT_EQ(info.levels[0].tlb_entries, 64);
  EXPECT_EQ(info.levels[0].partitioning, 1);

  EXPECT_EQ(info.levels[1].level, 1);
  EXPECT_EQ(info.levels[1].cache_type, 2);
  EXPECT_EQ(info.levels[1].cache_size, 32 * KiB);
  EXPECT_EQ(info.levels[1].ways, 8);
  EXPECT_EQ(info.levels[1].line_size, 64);
  EXPECT_EQ(info.levels[1].tlb_entries, 64);
  EXPECT_EQ(info.levels[1].partitioning, 1);

  EXPECT_EQ(info.levels[2].level, 2);
  EXPECT_EQ(info.levels[2].cache_type, 3);
  EXPECT_EQ(info.levels[2].cache_size, 256 * KiB);
  EXPECT_EQ(info.levels[2].ways, 4);
  EXPECT_EQ(info.levels[2].line_size, 64);
  EXPECT_EQ(info.levels[2].tlb_entries, 1024);
  EXPECT_EQ(info.levels[2].partitioning, 1);

  EXPECT_EQ(info.levels[3].level, 3);
  EXPECT_EQ(info.levels[3].cache_type, 3);
  EXPECT_EQ(info.levels[3].cache_size, 6 * MiB);
  EXPECT_EQ(info.levels[3].ways, 12);
  EXPECT_EQ(info.levels[3].line_size, 64);
  EXPECT_EQ(info.levels[3].tlb_entries, 8192);
  EXPECT_EQ(info.levels[3].partitioning, 1);
}

TEST_F(CpuidX86Test, HSWCache) {
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x00000016, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000406E3, 0x00100800, 0x7FFAFBBF, 0xBFEBFBFF}},
      {{0x00000004, 0}, Leaf{0x1C004121, 0x01C0003F, 0x0000003F, 0x00000000}},
      {{0x00000004, 1}, Leaf{0x1C004122, 0x01C0003F, 0x0000003F, 0x00000000}},
      {{0x00000004, 2}, Leaf{0x1C004143, 0x01C0003F, 0x000001FF, 0x00000000}},
      {{0x00000004, 3}, Leaf{0x1C03C163, 0x02C0003F, 0x00001FFF, 0x00000006}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x029C67AF, 0x00000000, 0x00000000}},
      {{0x80000000, 0}, Leaf{0x80000008, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000001, 0}, Leaf{0x00000000, 0x00000000, 0x00000121, 0x2C100000}},
      {{0x80000002, 0}, Leaf{0x65746E49, 0x2952286C, 0x726F4320, 0x4D542865}},
      {{0x80000003, 0}, Leaf{0x37692029, 0x3035362D, 0x43205530, 0x40205550}},
  });
  const auto info = GetX86CacheInfo();
  EXPECT_EQ(info.size, 4);
  EXPECT_EQ(info.levels[0].level, 1);
  EXPECT_EQ(info.levels[0].cache_type, 1);
  EXPECT_EQ(info.levels[0].cache_size, 32 * KiB);
  EXPECT_EQ(info.levels[0].ways, 8);
  EXPECT_EQ(info.levels[0].line_size, 64);
  EXPECT_EQ(info.levels[0].tlb_entries, 64);
  EXPECT_EQ(info.levels[0].partitioning, 1);

  EXPECT_EQ(info.levels[1].level, 1);
  EXPECT_EQ(info.levels[1].cache_type, 2);
  EXPECT_EQ(info.levels[1].cache_size, 32 * KiB);
  EXPECT_EQ(info.levels[1].ways, 8);
  EXPECT_EQ(info.levels[1].line_size, 64);
  EXPECT_EQ(info.levels[1].tlb_entries, 64);
  EXPECT_EQ(info.levels[1].partitioning, 1);

  EXPECT_EQ(info.levels[2].level, 2);
  EXPECT_EQ(info.levels[2].cache_type, 3);
  EXPECT_EQ(info.levels[2].cache_size, 256 * KiB);
  EXPECT_EQ(info.levels[2].ways, 8);
  EXPECT_EQ(info.levels[2].line_size, 64);
  EXPECT_EQ(info.levels[2].tlb_entries, 512);
  EXPECT_EQ(info.levels[2].partitioning, 1);

  EXPECT_EQ(info.levels[3].level, 3);
  EXPECT_EQ(info.levels[3].cache_type, 3);
  EXPECT_EQ(info.levels[3].cache_size, 6 * MiB);
  EXPECT_EQ(info.levels[3].ways, 12);
  EXPECT_EQ(info.levels[3].line_size, 64);
  EXPECT_EQ(info.levels[3].tlb_entries, 8192);
  EXPECT_EQ(info.levels[3].partitioning, 1);
}

// http://users.atw.hu/instlatx64/AuthenticAMD0630F81_K15_Godavari_CPUID.txt
TEST_F(CpuidX86Test, AMD_K15) {
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x0000000D, 0x68747541, 0x444D4163, 0x69746E65}},
      {{0x00000001, 0}, Leaf{0x00630F81, 0x00040800, 0x3E98320B, 0x178BFBFF}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000000, 0}, Leaf{0x8000001E, 0x68747541, 0x444D4163, 0x69746E65}},
      {{0x80000001, 0}, Leaf{0x00630F81, 0x10000000, 0x0FEBBFFF, 0x2FD3FBFF}},
      {{0x80000002, 0}, Leaf{0x20444D41, 0x372D3841, 0x4B303736, 0x64615220}},
      {{0x80000003, 0}, Leaf{0x206E6F65, 0x202C3752, 0x43203031, 0x75706D6F}},
      {{0x80000004, 0}, Leaf{0x43206574, 0x7365726F, 0x2B433420, 0x00204736}},
      {{0x80000005, 0}, Leaf{0xFF40FF18, 0xFF40FF30, 0x10040140, 0x60030140}},
  });
  const auto info = GetX86Info();

  EXPECT_STREQ(info.vendor, "AuthenticAMD");
  EXPECT_EQ(info.family, 0x15);
  EXPECT_EQ(info.model, 0x38);
  EXPECT_EQ(info.stepping, 0x01);
  EXPECT_EQ(GetX86Microarchitecture(&info),
            X86Microarchitecture::AMD_BULLDOZER);

  char brand_string[49];
  FillX86BrandString(brand_string);
  EXPECT_STREQ(brand_string, "AMD A8-7670K Radeon R7, 10 Compute Cores 4C+6G ");
}

// https://github.com/InstLatx64/InstLatx64/blob/master/GenuineIntel/GenuineIntel00106A1_Nehalem_CPUID.txt
TEST_F(CpuidX86Test, Nehalem) {
  // Pre AVX cpus don't have xsave
  g_fake_cpu->SetOsBackupsExtendedRegisters(false);
#if defined(CPU_FEATURES_OS_WINDOWS)
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_XMMI_INSTRUCTIONS_AVAILABLE);
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_XMMI64_INSTRUCTIONS_AVAILABLE);
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_SSE3_INSTRUCTIONS_AVAILABLE);
#endif  // CPU_FEATURES_OS_WINDOWS
#if defined(CPU_FEATURES_OS_DARWIN)
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse2");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse3");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.supplementalsse3");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse4_1");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse4_2");
#endif  // CPU_FEATURES_OS_DARWIN
#if defined(CPU_FEATURES_OS_LINUX_OR_ANDROID)
  auto& fs = GetEmptyFilesystem();
  fs.CreateFile("/proc/cpuinfo", R"(processor       :
flags           : fpu mmx sse sse2 sse3 ssse3 sse4_1 sse4_2
)");
#endif  // CPU_FEATURES_OS_LINUX_OR_ANDROID
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x0000000B, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x000106A2, 0x00100800, 0x00BCE3BD, 0xBFEBFBFF}},
      {{0x00000002, 0}, Leaf{0x55035A01, 0x00F0B0E3, 0x00000000, 0x09CA212C}},
      {{0x00000003, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x00000004, 0}, Leaf{0x1C004121, 0x01C0003F, 0x0000003F, 0x00000000}},
      {{0x00000004, 0}, Leaf{0x1C004122, 0x00C0003F, 0x0000007F, 0x00000000}},
      {{0x00000004, 0}, Leaf{0x1C004143, 0x01C0003F, 0x000001FF, 0x00000000}},
      {{0x00000004, 0}, Leaf{0x1C03C163, 0x03C0003F, 0x00000FFF, 0x00000002}},
      {{0x00000005, 0}, Leaf{0x00000040, 0x00000040, 0x00000003, 0x00021120}},
      {{0x00000006, 0}, Leaf{0x00000001, 0x00000002, 0x00000001, 0x00000000}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x00000008, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x00000009, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x0000000A, 0}, Leaf{0x07300403, 0x00000000, 0x00000000, 0x00000603}},
      {{0x0000000B, 0}, Leaf{0x00000001, 0x00000001, 0x00000100, 0x00000000}},
      {{0x0000000B, 0}, Leaf{0x00000004, 0x00000002, 0x00000201, 0x00000000}},
      {{0x80000000, 0}, Leaf{0x80000008, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000001, 0}, Leaf{0x00000000, 0x00000000, 0x00000001, 0x28100000}},
      {{0x80000002, 0}, Leaf{0x756E6547, 0x20656E69, 0x65746E49, 0x2952286C}},
      {{0x80000003, 0}, Leaf{0x55504320, 0x20202020, 0x20202020, 0x40202020}},
      {{0x80000004, 0}, Leaf{0x30303020, 0x20402030, 0x37382E31, 0x007A4847}},
      {{0x80000005, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000006, 0}, Leaf{0x00000000, 0x00000000, 0x01006040, 0x00000000}},
      {{0x80000007, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000100}},
      {{0x80000008, 0}, Leaf{0x00003028, 0x00000000, 0x00000000, 0x00000000}},
  });
  const auto info = GetX86Info();

  EXPECT_STREQ(info.vendor, "GenuineIntel");
  EXPECT_EQ(info.family, 0x06);
  EXPECT_EQ(info.model, 0x1A);
  EXPECT_EQ(info.stepping, 0x02);
  EXPECT_EQ(GetX86Microarchitecture(&info), X86Microarchitecture::INTEL_NHM);

  char brand_string[49];
  FillX86BrandString(brand_string);
  EXPECT_STREQ(brand_string, "Genuine Intel(R) CPU           @ 0000 @ 1.87GHz");

  EXPECT_TRUE(info.features.sse);
  EXPECT_TRUE(info.features.sse2);
  EXPECT_TRUE(info.features.sse3);
#ifndef CPU_FEATURES_OS_WINDOWS
  // Currently disabled on Windows as IsProcessorFeaturePresent do not support
  // feature detection > sse3.
  EXPECT_TRUE(info.features.ssse3);
  EXPECT_TRUE(info.features.sse4_1);
  EXPECT_TRUE(info.features.sse4_2);
#endif  // CPU_FEATURES_OS_WINDOWS
}

// https://github.com/InstLatx64/InstLatx64/blob/master/GenuineIntel/GenuineIntel0030673_Silvermont3_CPUID.txt
TEST_F(CpuidX86Test, Atom) {
  // Pre AVX cpus don't have xsave
  g_fake_cpu->SetOsBackupsExtendedRegisters(false);
#if defined(CPU_FEATURES_OS_WINDOWS)
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_XMMI_INSTRUCTIONS_AVAILABLE);
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_XMMI64_INSTRUCTIONS_AVAILABLE);
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_SSE3_INSTRUCTIONS_AVAILABLE);
#endif  // CPU_FEATURES_OS_WINDOWS
#if defined(CPU_FEATURES_OS_DARWIN)
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse2");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse3");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.supplementalsse3");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse4_1");
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse4_2");
#endif  // CPU_FEATURES_OS_DARWIN
#if defined(CPU_FEATURES_OS_LINUX_OR_ANDROID)
  auto& fs = GetEmptyFilesystem();
  fs.CreateFile("/proc/cpuinfo", R"(
flags           : fpu mmx sse sse2 sse3 ssse3 sse4_1 sse4_2
)");
#endif  // CPU_FEATURES_OS_LINUX_OR_ANDROID
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x0000000B, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x00030673, 0x00100800, 0x41D8E3BF, 0xBFEBFBFF}},
      {{0x00000002, 0}, Leaf{0x61B3A001, 0x0000FFC2, 0x00000000, 0x00000000}},
      {{0x00000003, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x00000004, 0}, Leaf{0x1C000121, 0x0140003F, 0x0000003F, 0x00000001}},
      {{0x00000004, 1}, Leaf{0x1C000122, 0x01C0003F, 0x0000003F, 0x00000001}},
      {{0x00000004, 2}, Leaf{0x1C00C143, 0x03C0003F, 0x000003FF, 0x00000001}},
      {{0x00000005, 0}, Leaf{0x00000040, 0x00000040, 0x00000003, 0x33000020}},
      {{0x00000006, 0}, Leaf{0x00000005, 0x00000002, 0x00000009, 0x00000000}},
      {{0x00000007, 0}, Leaf{0x00000000, 0x00002282, 0x00000000, 0x00000000}},
      {{0x00000008, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x00000009, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x0000000A, 0}, Leaf{0x07280203, 0x00000000, 0x00000000, 0x00004503}},
      {{0x0000000B, 0}, Leaf{0x00000001, 0x00000001, 0x00000100, 0x00000000}},
      {{0x0000000B, 1}, Leaf{0x00000004, 0x00000004, 0x00000201, 0x00000000}},
      {{0x80000000, 0}, Leaf{0x80000008, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000001, 0}, Leaf{0x00000000, 0x00000000, 0x00000101, 0x28100000}},
      {{0x80000002, 0}, Leaf{0x20202020, 0x6E492020, 0x286C6574, 0x43202952}},
      {{0x80000003, 0}, Leaf{0x72656C65, 0x52286E6F, 0x50432029, 0x4A202055}},
      {{0x80000004, 0}, Leaf{0x30303931, 0x20402020, 0x39392E31, 0x007A4847}},
      {{0x80000005, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000000}},
      {{0x80000006, 0}, Leaf{0x00000000, 0x00000000, 0x04008040, 0x00000000}},
      {{0x80000007, 0}, Leaf{0x00000000, 0x00000000, 0x00000000, 0x00000100}},
      {{0x80000008, 0}, Leaf{0x00003024, 0x00000000, 0x00000000, 0x00000000}},
  });
  const auto info = GetX86Info();

  EXPECT_STREQ(info.vendor, "GenuineIntel");
  EXPECT_EQ(info.family, 0x06);
  EXPECT_EQ(info.model, 0x37);
  EXPECT_EQ(info.stepping, 0x03);
  EXPECT_EQ(GetX86Microarchitecture(&info),
            X86Microarchitecture::INTEL_ATOM_SMT);

  char brand_string[49];
  FillX86BrandString(brand_string);
  EXPECT_STREQ(brand_string, "      Intel(R) Celeron(R) CPU  J1900  @ 1.99GHz");

  EXPECT_TRUE(info.features.sse);
  EXPECT_TRUE(info.features.sse2);
  EXPECT_TRUE(info.features.sse3);
#ifndef CPU_FEATURES_OS_WINDOWS
  // Currently disabled on Windows as IsProcessorFeaturePresent do not support
  // feature detection > sse3.
  EXPECT_TRUE(info.features.ssse3);
  EXPECT_TRUE(info.features.sse4_1);
  EXPECT_TRUE(info.features.sse4_2);
#endif  // CPU_FEATURES_OS_WINDOWS
}

// https://github.com/InstLatx64/InstLatx64/blob/master/GenuineIntel/GenuineIntel0000673_P3_KatmaiDP_CPUID.txt
TEST_F(CpuidX86Test, P3) {
  // Pre AVX cpus don't have xsave
  g_fake_cpu->SetOsBackupsExtendedRegisters(false);
#if defined(CPU_FEATURES_OS_WINDOWS)
  g_fake_cpu->SetWindowsIsProcessorFeaturePresent(
      PF_XMMI_INSTRUCTIONS_AVAILABLE);
#endif  // CPU_FEATURES_OS_WINDOWS
#if defined(CPU_FEATURES_OS_DARWIN)
  g_fake_cpu->SetDarwinSysCtlByName("hw.optional.sse");
#endif  // CPU_FEATURES_OS_DARWIN
#if defined(CPU_FEATURES_OS_LINUX_OR_ANDROID)
  auto& fs = GetEmptyFilesystem();
  fs.CreateFile("/proc/cpuinfo", R"(
flags           : fpu mmx sse
)");
#endif  // CPU_FEATURES_OS_LINUX_OR_ANDROID
  g_fake_cpu->SetLeaves({
      {{0x00000000, 0}, Leaf{0x00000003, 0x756E6547, 0x6C65746E, 0x49656E69}},
      {{0x00000001, 0}, Leaf{0x00000673, 0x00000000, 0x00000000, 0x0387FBFF}},
      {{0x00000002, 0}, Leaf{0x03020101, 0x00000000, 0x00000000, 0x0C040843}},
      {{0x00000003, 0}, Leaf{0x00000000, 0x00000000, 0x4CECC782, 0x00006778}},
  });
  const auto info = GetX86Info();

  EXPECT_STREQ(info.vendor, "GenuineIntel");
  EXPECT_EQ(info.family, 0x06);
  EXPECT_EQ(info.model, 0x07);
  EXPECT_EQ(info.stepping, 0x03);
  EXPECT_EQ(GetX86Microarchitecture(&info), X86Microarchitecture::X86_UNKNOWN);

  char brand_string[49];
  FillX86BrandString(brand_string);
  EXPECT_STREQ(brand_string, "");

  EXPECT_TRUE(info.features.mmx);
  EXPECT_TRUE(info.features.sse);
  EXPECT_FALSE(info.features.sse2);
  EXPECT_FALSE(info.features.sse3);
#ifndef CPU_FEATURES_OS_WINDOWS
  // Currently disabled on Windows as IsProcessorFeaturePresent do not support
  // feature detection > sse3.
  EXPECT_FALSE(info.features.ssse3);
  EXPECT_FALSE(info.features.sse4_1);
  EXPECT_FALSE(info.features.sse4_2);
#endif  // CPU_FEATURES_OS_WINDOWS
}

// TODO(user): test what happens when xsave/osxsave are not present.
// TODO(user): test what happens when xmm/ymm/zmm os support are not
// present.

}  // namespace
}  // namespace cpu_features
