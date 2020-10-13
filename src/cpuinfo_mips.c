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

#include "cpuinfo_mips.h"

#include <assert.h>

#include "internal/filesystem.h"
#include "internal/hwcaps.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"

// Generation of feature's getters/setters functions and kGetters, kSetters,
// kCpuInfoFlags and kHardwareCapabilities global tables.
#define DEFINE_TABLE_FEATURES                      \
  FEATURE(MIPS_MSA, msa, "msa", MIPS_HWCAP_MSA, 0) \
  FEATURE(MIPS_EVA, eva, "eva", 0, 0)              \
  FEATURE(MIPS_R6, r6, "r6", MIPS_HWCAP_R6, 0)
#define DEFINE_TABLE_FEATURE_TYPE MipsFeatures
#include "define_tables.h"

static bool HandleMipsLine(const LineResult result,
                           MipsFeatures* const features) {
  StringView key, value;
  // See tests for an example.
  if (CpuFeatures_StringView_GetAttributeKeyValue(result.line, &key, &value)) {
    if (CpuFeatures_StringView_IsEquals(key, str("ASEs implemented"))) {
      for (size_t i = 0; i < MIPS_LAST_; ++i) {
        kSetters[i](features,
                    CpuFeatures_StringView_HasWord(value, kCpuInfoFlags[i]));
      }
    }
  }
  return !result.eof;
}

static void FillProcCpuInfoData(MipsFeatures* const features) {
  const int fd = CpuFeatures_OpenFile("/proc/cpuinfo");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      if (!HandleMipsLine(StackLineReader_NextLine(&reader), features)) {
        break;
      }
    }
    CpuFeatures_CloseFile(fd);
  }
}

static const MipsInfo kEmptyMipsInfo;

MipsInfo GetMipsInfo(void) {
  // capabilities are fetched from both getauxval and /proc/cpuinfo so we can
  // have some information if the executable is sandboxed (aka no access to
  // /proc/cpuinfo).
  MipsInfo info = kEmptyMipsInfo;

  FillProcCpuInfoData(&info.features);
  const HardwareCapabilities hwcaps = CpuFeatures_GetHardwareCapabilities();
  for (size_t i = 0; i < MIPS_LAST_; ++i) {
    if (CpuFeatures_IsHwCapsSet(kHardwareCapabilities[i], hwcaps)) {
      kSetters[i](&info.features, true);
    }
  }
  return info;
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetMipsFeaturesEnumValue(const MipsFeatures* features,
                             MipsFeaturesEnum value) {
  if (value >= MIPS_LAST_) return false;
  return kGetters[value](features);
}

const char* GetMipsFeaturesEnumName(MipsFeaturesEnum value) {
  if (value >= MIPS_LAST_) return "unknown feature";
  return kCpuInfoFlags[value];
}
