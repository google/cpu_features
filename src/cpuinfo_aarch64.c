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

#include <assert.h>
#include <ctype.h>

#include "internal/filesystem.h"
#include "internal/hwcaps.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"

// Generation of feature's getters/setters functions and kGetters, kSetters,
// kCpuInfoFlags and kHardwareCapabilities global tables.
#define DEFINE_TABLE_FEATURES                                                 \
  FEATURE(AARCH64_FP, fp, "fp", AARCH64_HWCAP_FP, 0)                          \
  FEATURE(AARCH64_ASIMD, asimd, "asimd", AARCH64_HWCAP_ASIMD, 0)              \
  FEATURE(AARCH64_EVTSTRM, evtstrm, "evtstrm", AARCH64_HWCAP_EVTSTRM, 0)      \
  FEATURE(AARCH64_AES, aes, "aes", AARCH64_HWCAP_AES, 0)                      \
  FEATURE(AARCH64_PMULL, pmull, "pmull", AARCH64_HWCAP_PMULL, 0)              \
  FEATURE(AARCH64_SHA1, sha1, "sha1", AARCH64_HWCAP_SHA1, 0)                  \
  FEATURE(AARCH64_SHA2, sha2, "sha2", AARCH64_HWCAP_SHA2, 0)                  \
  FEATURE(AARCH64_CRC32, crc32, "crc32", AARCH64_HWCAP_CRC32, 0)              \
  FEATURE(AARCH64_ATOMICS, atomics, "atomics", AARCH64_HWCAP_ATOMICS, 0)      \
  FEATURE(AARCH64_FPHP, fphp, "fphp", AARCH64_HWCAP_FPHP, 0)                  \
  FEATURE(AARCH64_ASIMDHP, asimdhp, "asimdhp", AARCH64_HWCAP_ASIMDHP, 0)      \
  FEATURE(AARCH64_CPUID, cpuid, "cpuid", AARCH64_HWCAP_CPUID, 0)              \
  FEATURE(AARCH64_ASIMDRDM, asimdrdm, "asimdrdm", AARCH64_HWCAP_ASIMDRDM, 0)  \
  FEATURE(AARCH64_JSCVT, jscvt, "jscvt", AARCH64_HWCAP_JSCVT, 0)              \
  FEATURE(AARCH64_FCMA, fcma, "fcma", AARCH64_HWCAP_FCMA, 0)                  \
  FEATURE(AARCH64_LRCPC, lrcpc, "lrcpc", AARCH64_HWCAP_LRCPC, 0)              \
  FEATURE(AARCH64_DCPOP, dcpop, "dcpop", AARCH64_HWCAP_DCPOP, 0)              \
  FEATURE(AARCH64_SHA3, sha3, "sha3", AARCH64_HWCAP_SHA3, 0)                  \
  FEATURE(AARCH64_SM3, sm3, "sm3", AARCH64_HWCAP_SM3, 0)                      \
  FEATURE(AARCH64_SM4, sm4, "sm4", AARCH64_HWCAP_SM4, 0)                      \
  FEATURE(AARCH64_ASIMDDP, asimddp, "asimddp", AARCH64_HWCAP_ASIMDDP, 0)      \
  FEATURE(AARCH64_SHA512, sha512, "sha512", AARCH64_HWCAP_SHA512, 0)          \
  FEATURE(AARCH64_SVE, sve, "sve", AARCH64_HWCAP_SVE, 0)                      \
  FEATURE(AARCH64_ASIMDFHM, asimdfhm, "asimdfhm", AARCH64_HWCAP_ASIMDFHM, 0)  \
  FEATURE(AARCH64_DIT, dit, "dit", AARCH64_HWCAP_DIT, 0)                      \
  FEATURE(AARCH64_USCAT, uscat, "uscat", AARCH64_HWCAP_USCAT, 0)              \
  FEATURE(AARCH64_ILRCPC, ilrcpc, "ilrcpc", AARCH64_HWCAP_ILRCPC, 0)          \
  FEATURE(AARCH64_FLAGM, flagm, "flagm", AARCH64_HWCAP_FLAGM, 0)              \
  FEATURE(AARCH64_SSBS, ssbs, "ssbs", AARCH64_HWCAP_SSBS, 0)                  \
  FEATURE(AARCH64_SB, sb, "sb", AARCH64_HWCAP_SB, 0)                          \
  FEATURE(AARCH64_PACA, paca, "paca", AARCH64_HWCAP_PACA, 0)                  \
  FEATURE(AARCH64_PACG, pacg, "pacg", AARCH64_HWCAP_PACG, 0)                  \
  FEATURE(AARCH64_DCPODP, dcpodp, "dcpodp", 0, AARCH64_HWCAP2_DCPODP)         \
  FEATURE(AARCH64_SVE2, sve2, "sve2", 0, AARCH64_HWCAP2_SVE2)                 \
  FEATURE(AARCH64_SVEAES, sveaes, "sveaes", 0, AARCH64_HWCAP2_SVEAES)         \
  FEATURE(AARCH64_SVEPMULL, svepmull, "svepmull", 0, AARCH64_HWCAP2_SVEPMULL) \
  FEATURE(AARCH64_SVEBITPERM, svebitperm, "svebitperm", 0,                    \
          AARCH64_HWCAP2_SVEBITPERM)                                          \
  FEATURE(AARCH64_SVESHA3, svesha3, "svesha3", 0, AARCH64_HWCAP2_SVESHA3)     \
  FEATURE(AARCH64_SVESM4, svesm4, "svesm4", 0, AARCH64_HWCAP2_SVESM4)         \
  FEATURE(AARCH64_FLAGM2, flagm2, "flagm2", 0, AARCH64_HWCAP2_FLAGM2)         \
  FEATURE(AARCH64_FRINT, frint, "frint", 0, AARCH64_HWCAP2_FRINT)             \
  FEATURE(AARCH64_SVEI8MM, svei8mm, "svei8mm", 0, AARCH64_HWCAP2_SVEI8MM)     \
  FEATURE(AARCH64_SVEF32MM, svef32mm, "svef32mm", 0, AARCH64_HWCAP2_SVEF32MM) \
  FEATURE(AARCH64_SVEF64MM, svef64mm, "svef64mm", 0, AARCH64_HWCAP2_SVEF64MM) \
  FEATURE(AARCH64_SVEBF16, svebf16, "svebf16", 0, AARCH64_HWCAP2_SVEBF16)     \
  FEATURE(AARCH64_I8MM, i8mm, "i8mm", 0, AARCH64_HWCAP2_I8MM)                 \
  FEATURE(AARCH64_BF16, bf16, "bf16", 0, AARCH64_HWCAP2_BF16)                 \
  FEATURE(AARCH64_DGH, dgh, "dgh", 0, AARCH64_HWCAP2_DGH)                     \
  FEATURE(AARCH64_RNG, rng, "rng", 0, AARCH64_HWCAP2_RNG)                     \
  FEATURE(AARCH64_BTI, bti, "bti", 0, AARCH64_HWCAP2_BTI)
#define DEFINE_TABLE_FEATURE_TYPE Aarch64Features
#include "define_tables.h"

static bool HandleAarch64Line(const LineResult result,
                              Aarch64Info* const info) {
  StringView line = result.line;
  StringView key, value;
  if (CpuFeatures_StringView_GetAttributeKeyValue(line, &key, &value)) {
    if (CpuFeatures_StringView_IsEquals(key, str("Features"))) {
      for (size_t i = 0; i < AARCH64_LAST_; ++i) {
        kSetters[i](&info->features,
                    CpuFeatures_StringView_HasWord(value, kCpuInfoFlags[i]));
      }
    } else if (CpuFeatures_StringView_IsEquals(key, str("CPU implementer"))) {
      info->implementer = CpuFeatures_StringView_ParsePositiveNumber(value);
    } else if (CpuFeatures_StringView_IsEquals(key, str("CPU variant"))) {
      info->variant = CpuFeatures_StringView_ParsePositiveNumber(value);
    } else if (CpuFeatures_StringView_IsEquals(key, str("CPU part"))) {
      info->part = CpuFeatures_StringView_ParsePositiveNumber(value);
    } else if (CpuFeatures_StringView_IsEquals(key, str("CPU revision"))) {
      info->revision = CpuFeatures_StringView_ParsePositiveNumber(value);
    }
  }
  return !result.eof;
}

static void FillProcCpuInfoData(Aarch64Info* const info) {
  const int fd = CpuFeatures_OpenFile("/proc/cpuinfo");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      if (!HandleAarch64Line(StackLineReader_NextLine(&reader), info)) {
        break;
      }
    }
    CpuFeatures_CloseFile(fd);
  }
}

static const Aarch64Info kEmptyAarch64Info;

Aarch64Info GetAarch64Info(void) {
  // capabilities are fetched from both getauxval and /proc/cpuinfo so we can
  // have some information if the executable is sandboxed (aka no access to
  // /proc/cpuinfo).
  Aarch64Info info = kEmptyAarch64Info;

  FillProcCpuInfoData(&info);
  const HardwareCapabilities hwcaps = CpuFeatures_GetHardwareCapabilities();
  for (size_t i = 0; i < AARCH64_LAST_; ++i) {
    if (CpuFeatures_IsHwCapsSet(kHardwareCapabilities[i], hwcaps)) {
      kSetters[i](&info.features, true);
    }
  }

  return info;
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetAarch64FeaturesEnumValue(const Aarch64Features* features,
                                Aarch64FeaturesEnum value) {
  if (value >= AARCH64_LAST_) return false;
  return kGetters[value](features);
}

const char* GetAarch64FeaturesEnumName(Aarch64FeaturesEnum value) {
  if (value >= AARCH64_LAST_) return "unknown feature";
  return kCpuInfoFlags[value];
}
