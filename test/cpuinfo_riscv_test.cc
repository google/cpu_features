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

#include "cpuinfo_riscv.h"

#include "filesystem_for_testing.h"
#include "gtest/gtest.h"
#include "hwcaps_for_testing.h"

namespace cpu_features {
namespace {

TEST(CpuinfoRiscvTest, LicheeFromCpuInfo) {
    ResetHwcaps();
    auto& fs = GetEmptyFilesystem();
    fs.CreateFile("/proc/cpuinfo", R"(processor	: 0 
hart  : 0
isa   : rv64imafdc
mmu   : sv39
uarch : thead,c906)");
    const auto info = GetRiscvInfo();
    EXPECT_STREQ(info.uarch, "c906");
    EXPECT_STREQ(info.vendor, "thead");

    EXPECT_FALSE(info.features.riscv32);
    EXPECT_TRUE(info.features.riscv64);
    EXPECT_FALSE(info.features.riscv128);
    EXPECT_TRUE(info.features.a);
    EXPECT_TRUE(info.features.c);
    EXPECT_TRUE(info.features.d);
    EXPECT_FALSE(info.features.e);
    EXPECT_TRUE(info.features.f);
    EXPECT_TRUE(info.features.i);
    EXPECT_TRUE(info.features.m);
    EXPECT_FALSE(info.features.q);
    EXPECT_FALSE(info.features.v);
}

TEST(CpuinfoRiscvTest, KendryteFromCpuInfo) {
    ResetHwcaps();
    auto& fs = GetEmptyFilesystem();
    fs.CreateFile("/proc/cpuinfo", R"(
hart  : 0
isa  : rv64i2p0m2p0a2p0f2p0d2p0c2p0xv5-0p0
mmu	 : sv39

hart : 1
isa	 : rv64i2p0m2p0a2p0f2p0d2p0c2p0xv5-0p0
mmu	 : sv39)");
    const auto info = GetRiscvInfo();
    EXPECT_STREQ(info.uarch, "");
    EXPECT_STREQ(info.vendor, "");

    EXPECT_FALSE(info.features.riscv32);
    EXPECT_TRUE(info.features.riscv64);
    EXPECT_FALSE(info.features.riscv128);
    EXPECT_TRUE(info.features.a);
    EXPECT_TRUE(info.features.c);
    EXPECT_TRUE(info.features.d);
    EXPECT_FALSE(info.features.e);
    EXPECT_TRUE(info.features.f);
    EXPECT_TRUE(info.features.i);
    EXPECT_TRUE(info.features.m);
    EXPECT_FALSE(info.features.q);
    EXPECT_TRUE(info.features.v);
}

TEST(CpuinfoRiscvTest, UnknownFromCpuInfo) {
    ResetHwcaps();
    auto& fs = GetEmptyFilesystem();
    fs.CreateFile("/proc/cpuinfo", R"(
processor : 0
hart      : 2
isa       : rv64imafdc
mmu       : sv39
uarch     : sifive,bullet0

processor : 1
hart      : 1
isa       : rv64imafdc
mmu       : sv39
uarch     : sifive,bullet0

processor : 2
hart      : 3
isa       : rv64imafdc
mmu       : sv39
uarch     : sifive,bullet0

processor : 3
hart      : 4
isa       : rv64imafdc
mmu       : sv39
uarch     : sifive,bullet0)");
    const auto info = GetRiscvInfo();
    EXPECT_STREQ(info.uarch, "bullet0");
    EXPECT_STREQ(info.vendor, "sifive");

    EXPECT_FALSE(info.features.riscv32);
    EXPECT_TRUE(info.features.riscv64);
    EXPECT_FALSE(info.features.riscv128);
    EXPECT_TRUE(info.features.a);
    EXPECT_TRUE(info.features.c);
    EXPECT_TRUE(info.features.d);
    EXPECT_FALSE(info.features.e);
    EXPECT_TRUE(info.features.f);
    EXPECT_TRUE(info.features.i);
    EXPECT_TRUE(info.features.m);
    EXPECT_FALSE(info.features.q);
    EXPECT_FALSE(info.features.v);
}

}  // namespace
}  // namespace cpu_features
