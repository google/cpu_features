// Copyright 2017 Google Inc.
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

#include <stdio.h>

#include <algorithm>
#include <string>
#include <vector>

#include "cpu_features_macros.h"
#include "cpuinfo_aarch64.h"
#include "cpuinfo_arm.h"
#include "cpuinfo_mips.h"
#include "cpuinfo_x86.h"

namespace cpu_features {

// Prints a named numeric value in both decimal and hexadecimal.
void PrintN(const char* field, int value) {
  printf("%-15s : %3d (0x%02X)\n", field, value, value);
}

// Prints a named string.
void PrintS(const char* field, const char* value) {
  printf("%-15s : %s\n", field, value);
}

template <typename HasFeatureFun, typename FeatureNameFun, typename FeatureType,
          typename EnumType>
std::string GetFlags(const HasFeatureFun HasFeature,
                     const FeatureNameFun FeatureName,
                     const FeatureType* features, const EnumType last) {
  std::vector<std::string> flags;
  for (int i = 0; i < last; ++i) {
    const EnumType enum_value = static_cast<EnumType>(i);
    if (HasFeature(features, enum_value)) {
      flags.push_back(FeatureName(enum_value));
    }
  }
  std::sort(flags.begin(), flags.end());
  std::string buffer;
  for (const auto& flag : flags) {
    if (!buffer.empty()) buffer += ' ';
    buffer += flag;
  }
  return buffer;
}

void Main() {
#if defined(CPU_FEATURES_ARCH_X86)
  char brand_string[49];
  const X86Info info = GetX86Info();
  const auto flags = GetFlags(&GetX86FeaturesEnumValue, &GetX86FeaturesEnumName,
                              &info.features, X86FeaturesEnum::X86_LAST_);
  FillX86BrandString(brand_string);
  PrintS("arch", "x86");
  PrintS("brand", brand_string);
  PrintN("family", info.family);
  PrintN("model", info.model);
  PrintN("stepping", info.stepping);
  PrintS("uarch", GetX86MicroarchitectureName(GetX86Microarchitecture(&info)));
  PrintS("flags", flags.c_str());
#elif defined(CPU_FEATURES_ARCH_ARM)
  const ArmInfo info = GetArmInfo();
  const auto flags = GetFlags(&GetArmFeaturesEnumValue, &GetArmFeaturesEnumName,
                              &info.features, ArmFeaturesEnum::ARM_LAST_);
  PrintS("arch", "ARM");
  PrintN("implementer", info.implementer);
  PrintN("architecture", info.architecture);
  PrintN("variant", info.variant);
  PrintN("part", info.part);
  PrintN("revision", info.revision);
  PrintS("flags", flags.c_str());
#elif defined(CPU_FEATURES_ARCH_AARCH64)
  const Aarch64Info info = GetAarch64Info();
  const auto flags =
      GetFlags(&GetAarch64FeaturesEnumValue, &GetAarch64FeaturesEnumName,
               &info.features, Aarch64FeaturesEnum::AARCH64_LAST_);
  PrintS("arch", "aarch64");
  PrintN("implementer", info.implementer);
  PrintN("variant", info.variant);
  PrintN("part", info.part);
  PrintN("revision", info.revision);
  PrintS("flags", flags.c_str());
#elif defined(CPU_FEATURES_ARCH_MIPS)
  const MipsInfo info = GetMipsInfo();
  const auto flags =
      GetFlags(&GetMipsFeaturesEnumValue, &GetMipsFeaturesEnumName,
               &info.features, MipsFeaturesEnum::MIPS_LAST_);
  PrintS("arch", "mips");
  PrintS("flags", flags.c_str());
#endif
}

}  // namespace cpu_features

int main(int argc, char** argv) {
  cpu_features::Main();
  return 0;
}
