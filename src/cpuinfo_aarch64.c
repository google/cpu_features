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

#include "cpuinfo_aarch64.h"

#include <assert.h>
#include <ctype.h>

#include "internal/filesystem.h"
#include "internal/hwcaps.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"
#include "internal/unix_features_aggregator.h"

DECLARE_SETTER_AND_GETTER(Aarch64Features, fp)
DECLARE_SETTER_AND_GETTER(Aarch64Features, asimd)
DECLARE_SETTER_AND_GETTER(Aarch64Features, evtstrm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, aes)
DECLARE_SETTER_AND_GETTER(Aarch64Features, pmull)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sha1)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sha2)
DECLARE_SETTER_AND_GETTER(Aarch64Features, crc32)
DECLARE_SETTER_AND_GETTER(Aarch64Features, atomics)
DECLARE_SETTER_AND_GETTER(Aarch64Features, fphp)
DECLARE_SETTER_AND_GETTER(Aarch64Features, asimdhp)
DECLARE_SETTER_AND_GETTER(Aarch64Features, cpuid)
DECLARE_SETTER_AND_GETTER(Aarch64Features, asimdrdm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, jscvt)
DECLARE_SETTER_AND_GETTER(Aarch64Features, fcma)
DECLARE_SETTER_AND_GETTER(Aarch64Features, lrcpc)
DECLARE_SETTER_AND_GETTER(Aarch64Features, dcpop)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sha3)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sm3)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sm4)
DECLARE_SETTER_AND_GETTER(Aarch64Features, asimddp)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sha512)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sve)
DECLARE_SETTER_AND_GETTER(Aarch64Features, asimdfhm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, dit)
DECLARE_SETTER_AND_GETTER(Aarch64Features, uscat)
DECLARE_SETTER_AND_GETTER(Aarch64Features, ilrcpc)
DECLARE_SETTER_AND_GETTER(Aarch64Features, flagm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, ssbs)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sb)
DECLARE_SETTER_AND_GETTER(Aarch64Features, paca)
DECLARE_SETTER_AND_GETTER(Aarch64Features, pacg)
DECLARE_SETTER_AND_GETTER(Aarch64Features, dcpodp)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sve2)
DECLARE_SETTER_AND_GETTER(Aarch64Features, sveaes)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svepmull)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svebitperm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svesha3)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svesm4)
DECLARE_SETTER_AND_GETTER(Aarch64Features, flagm2)
DECLARE_SETTER_AND_GETTER(Aarch64Features, frint)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svei8mm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svef32mm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svef64mm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, svebf16)
DECLARE_SETTER_AND_GETTER(Aarch64Features, i8mm)
DECLARE_SETTER_AND_GETTER(Aarch64Features, bf16)
DECLARE_SETTER_AND_GETTER(Aarch64Features, dgh)
DECLARE_SETTER_AND_GETTER(Aarch64Features, rng)
DECLARE_SETTER_AND_GETTER(Aarch64Features, bti)

static const CapabilityConfig kConfigs[] = {
    // clang-format off
    [AARCH64_FP] = {{AARCH64_HWCAP_FP, 0}, "fp", &set_fp, &get_fp},
    [AARCH64_ASIMD] = {{AARCH64_HWCAP_ASIMD, 0}, "asimd", &set_asimd, &get_asimd},
    [AARCH64_EVTSTRM] = {{AARCH64_HWCAP_EVTSTRM, 0}, "evtstrm", &set_evtstrm, &get_evtstrm},
    [AARCH64_AES] = {{AARCH64_HWCAP_AES, 0}, "aes", &set_aes, &get_aes},
    [AARCH64_PMULL] = {{AARCH64_HWCAP_PMULL, 0}, "pmull", &set_pmull, &get_pmull},
    [AARCH64_SHA1] = {{AARCH64_HWCAP_SHA1, 0}, "sha1", &set_sha1, &get_sha1},
    [AARCH64_SHA2] = {{AARCH64_HWCAP_SHA2, 0}, "sha2", &set_sha2, &get_sha2},
    [AARCH64_CRC32] = {{AARCH64_HWCAP_CRC32, 0}, "crc32", &set_crc32, &get_crc32},
    [AARCH64_ATOMICS] = {{AARCH64_HWCAP_ATOMICS, 0}, "atomics", &set_atomics, &get_atomics},
    [AARCH64_FPHP] = {{AARCH64_HWCAP_FPHP, 0}, "fphp", &set_fphp, &get_fphp},
    [AARCH64_ASIMDHP] = {{AARCH64_HWCAP_ASIMDHP, 0}, "asimdhp", &set_asimdhp, &get_asimdhp},
    [AARCH64_CPUID] = {{AARCH64_HWCAP_CPUID, 0}, "cpuid", &set_cpuid, &get_cpuid},
    [AARCH64_ASIMDRDM] = {{AARCH64_HWCAP_ASIMDRDM, 0}, "asimdrdm", &set_asimdrdm, &get_asimdrdm},
    [AARCH64_JSCVT] = {{AARCH64_HWCAP_JSCVT, 0}, "jscvt", &set_jscvt, &get_jscvt},
    [AARCH64_FCMA] = {{AARCH64_HWCAP_FCMA, 0}, "fcma", &set_fcma, &get_fcma},
    [AARCH64_LRCPC] = {{AARCH64_HWCAP_LRCPC, 0}, "lrcpc", &set_lrcpc, &get_lrcpc},
    [AARCH64_DCPOP] = {{AARCH64_HWCAP_DCPOP, 0}, "dcpop", &set_dcpop, &get_dcpop},
    [AARCH64_SHA3] = {{AARCH64_HWCAP_SHA3, 0}, "sha3", &set_sha3, &get_sha3},
    [AARCH64_SM3] = {{AARCH64_HWCAP_SM3, 0}, "sm3", &set_sm3, &get_sm3},
    [AARCH64_SM4] = {{AARCH64_HWCAP_SM4, 0}, "sm4", &set_sm4, &get_sm4},
    [AARCH64_ASIMDDP] = {{AARCH64_HWCAP_ASIMDDP, 0}, "asimddp", &set_asimddp, &get_asimddp},
    [AARCH64_SHA512] = {{AARCH64_HWCAP_SHA512, 0}, "sha512", &set_sha512, &get_sha512},
    [AARCH64_SVE] = {{AARCH64_HWCAP_SVE, 0}, "sve", &set_sve, &get_sve},
    [AARCH64_ASIMDFHM] = {{AARCH64_HWCAP_ASIMDFHM, 0}, "asimdfhm", &set_asimdfhm, &get_asimdfhm},
    [AARCH64_DIT] = {{AARCH64_HWCAP_DIT, 0}, "dit", &set_dit, &get_dit},
    [AARCH64_USCAT] = {{AARCH64_HWCAP_USCAT, 0}, "uscat", &set_uscat, &get_uscat},
    [AARCH64_ILRCPC] = {{AARCH64_HWCAP_ILRCPC, 0}, "ilrcpc", &set_ilrcpc, &get_ilrcpc},
    [AARCH64_FLAGM] = {{AARCH64_HWCAP_FLAGM, 0}, "flagm", &set_flagm, &get_flagm},
    [AARCH64_SSBS] = {{AARCH64_HWCAP_SSBS, 0}, "ssbs", &set_ssbs, &get_ssbs},
    [AARCH64_SB] = {{AARCH64_HWCAP_SB, 0}, "sb", &set_sb, &get_sb},
    [AARCH64_PACA] = {{AARCH64_HWCAP_PACA, 0}, "paca", &set_paca, &get_paca},
    [AARCH64_PACG] = {{AARCH64_HWCAP_PACG, 0}, "pacg", &set_pacg, &get_pacg},
    [AARCH64_DCPODP] = {{0, AARCH64_HWCAP2_DCPODP}, "dcpodp", &set_dcpodp, &get_dcpodp},
    [AARCH64_SVE2] = {{0, AARCH64_HWCAP2_SVE2}, "sve2", &set_sve2, &get_sve2},
    [AARCH64_SVEAES] = {{0, AARCH64_HWCAP2_SVEAES}, "sveaes", &set_sveaes, &get_sveaes},
    [AARCH64_SVEPMULL] = {{0, AARCH64_HWCAP2_SVEPMULL}, "svepmull", &set_svepmull, &get_svepmull},
    [AARCH64_SVEBITPERM] = {{0, AARCH64_HWCAP2_SVEBITPERM}, "svebitperm", &set_svebitperm, &get_svebitperm},
    [AARCH64_SVESHA3] = {{0, AARCH64_HWCAP2_SVESHA3}, "svesha3", &set_svesha3, &get_svesha3},
    [AARCH64_SVESM4] = {{0, AARCH64_HWCAP2_SVESM4}, "svesm4", &set_svesm4, &get_svesm4},
    [AARCH64_FLAGM2] = {{0, AARCH64_HWCAP2_FLAGM2}, "flagm2", &set_flagm2, &get_flagm2},
    [AARCH64_FRINT] = {{0, AARCH64_HWCAP2_FRINT}, "frint", &set_frint, &get_frint},
    [AARCH64_SVEI8MM] = {{0, AARCH64_HWCAP2_SVEI8MM}, "svei8mm", &set_svei8mm, &get_svei8mm},
    [AARCH64_SVEF32MM] = {{0, AARCH64_HWCAP2_SVEF32MM}, "svef32mm", &set_svef32mm, &get_svef32mm},
    [AARCH64_SVEF64MM] = {{0, AARCH64_HWCAP2_SVEF64MM}, "svef64mm", &set_svef64mm, &get_svef64mm},
    [AARCH64_SVEBF16] = {{0, AARCH64_HWCAP2_SVEBF16}, "svebf16", &set_svebf16, &get_svebf16},
    [AARCH64_I8MM] = {{0, AARCH64_HWCAP2_I8MM}, "i8mm", &set_i8mm, &get_i8mm},
    [AARCH64_BF16] = {{0, AARCH64_HWCAP2_BF16}, "bf16", &set_bf16, &get_bf16},
    [AARCH64_DGH] = {{0, AARCH64_HWCAP2_DGH}, "dgh", &set_dgh, &get_dgh},
    [AARCH64_RNG] = {{0, AARCH64_HWCAP2_RNG}, "rng", &set_rng, &get_rng},
    [AARCH64_BTI] = {{0, AARCH64_HWCAP2_BTI}, "bti", &set_bti, &get_bti},
    // clang-format on
};

static const size_t kConfigsSize = sizeof(kConfigs) / sizeof(CapabilityConfig);

static bool HandleAarch64Line(const LineResult result,
                              Aarch64Info* const info) {
  StringView line = result.line;
  StringView key, value;
  if (CpuFeatures_StringView_GetAttributeKeyValue(line, &key, &value)) {
    if (CpuFeatures_StringView_IsEquals(key, str("Features"))) {
      CpuFeatures_SetFromFlags(kConfigsSize, kConfigs, value, &info->features);
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
  assert(kConfigsSize == AARCH64_LAST_);

  // capabilities are fetched from both getauxval and /proc/cpuinfo so we can
  // have some information if the executable is sandboxed (aka no access to
  // /proc/cpuinfo).
  Aarch64Info info = kEmptyAarch64Info;

  FillProcCpuInfoData(&info);
  CpuFeatures_OverrideFromHwCaps(kConfigsSize, kConfigs,
                                 CpuFeatures_GetHardwareCapabilities(),
                                 &info.features);

  return info;
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetAarch64FeaturesEnumValue(const Aarch64Features* features,
                                Aarch64FeaturesEnum value) {
  if (value >= kConfigsSize) return false;
  return kConfigs[value].get_bit((Aarch64Features*)features);
}

const char* GetAarch64FeaturesEnumName(Aarch64FeaturesEnum value) {
  if (value >= kConfigsSize) return "unknown feature";
  return kConfigs[value].proc_cpuinfo_flag;
}
