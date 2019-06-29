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

#include "cpuinfo_arm.h"

#include "internal/bit_utils.h"
#include "internal/filesystem.h"
#include "internal/hwcaps.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"
#include "internal/unix_features_aggregator.h"

#include <assert.h>
#include <ctype.h>

DECLARE_SETTER(ArmFeatures, swp)
DECLARE_SETTER(ArmFeatures, half)
DECLARE_SETTER(ArmFeatures, thumb)
DECLARE_SETTER(ArmFeatures, _26bit)
DECLARE_SETTER(ArmFeatures, fastmult)
DECLARE_SETTER(ArmFeatures, fpa)
DECLARE_SETTER(ArmFeatures, vfp)
DECLARE_SETTER(ArmFeatures, edsp)
DECLARE_SETTER(ArmFeatures, java)
DECLARE_SETTER(ArmFeatures, iwmmxt)
DECLARE_SETTER(ArmFeatures, crunch)
DECLARE_SETTER(ArmFeatures, thumbee)
DECLARE_SETTER(ArmFeatures, neon)
DECLARE_SETTER(ArmFeatures, vfpv3)
DECLARE_SETTER(ArmFeatures, vfpv3d16)
DECLARE_SETTER(ArmFeatures, tls)
DECLARE_SETTER(ArmFeatures, vfpv4)
DECLARE_SETTER(ArmFeatures, idiva)
DECLARE_SETTER(ArmFeatures, idivt)
DECLARE_SETTER(ArmFeatures, vfpd32)
DECLARE_SETTER(ArmFeatures, lpae)
DECLARE_SETTER(ArmFeatures, evtstrm)
DECLARE_SETTER(ArmFeatures, aes)
DECLARE_SETTER(ArmFeatures, pmull)
DECLARE_SETTER(ArmFeatures, sha1)
DECLARE_SETTER(ArmFeatures, sha2)
DECLARE_SETTER(ArmFeatures, crc32)

static const CapabilityConfig kConfigs[] = {
  [ARM_SWP] = {{ARM_HWCAP_SWP, 0}, "swp", &set_swp},                      //
  [ARM_HALF] = {{ARM_HWCAP_HALF, 0}, "half", &set_half},                  //
  [ARM_THUMB] = {{ARM_HWCAP_THUMB, 0}, "thumb", &set_thumb},              //
  [ARM_26BIT] = {{ARM_HWCAP_26BIT, 0}, "26bit", &set__26bit},             //
  [ARM_FASTMULT] = {{ARM_HWCAP_FAST_MULT, 0}, "fastmult", &set_fastmult}, //
  [ARM_FPA] = {{ARM_HWCAP_FPA, 0}, "fpa", &set_fpa},                      //
  [ARM_VFP] = {{ARM_HWCAP_VFP, 0}, "vfp", &set_vfp},                      //
  [ARM_EDSP] = {{ARM_HWCAP_EDSP, 0}, "edsp", &set_edsp},                  //
  [ARM_JAVA] = {{ARM_HWCAP_JAVA, 0}, "java", &set_java},                  //
  [ARM_IWMMXT] = {{ARM_HWCAP_IWMMXT, 0}, "iwmmxt", &set_iwmmxt},          //
  [ARM_CRUNCH] = {{ARM_HWCAP_CRUNCH, 0}, "crunch", &set_crunch},          //
  [ARM_THUMBEE] = {{ARM_HWCAP_THUMBEE, 0}, "thumbee", &set_thumbee},      //
  [ARM_NEON] = {{ARM_HWCAP_NEON, 0}, "neon", &set_neon},                  //
  [ARM_VFPV3] = {{ARM_HWCAP_VFPV3, 0}, "vfpv3", &set_vfpv3},              //
  [ARM_VFPV3D16] = {{ARM_HWCAP_VFPV3D16, 0}, "vfpv3d16", &set_vfpv3d16},  //
  [ARM_TLS] = {{ARM_HWCAP_TLS, 0}, "tls", &set_tls},                      //
  [ARM_VFPV4] = {{ARM_HWCAP_VFPV4, 0}, "vfpv4", &set_vfpv4},              //
  [ARM_IDIVA] = {{ARM_HWCAP_IDIVA, 0}, "idiva", &set_idiva},              //
  [ARM_IDIVT] = {{ARM_HWCAP_IDIVT, 0}, "idivt", &set_idivt},              //
  [ARM_VFPD32] = {{ARM_HWCAP_VFPD32, 0}, "vfpd32", &set_vfpd32},          //
  [ARM_LPAE] = {{ARM_HWCAP_LPAE, 0}, "lpae", &set_lpae},                  //
  [ARM_EVTSTRM] = {{ARM_HWCAP_EVTSTRM, 0}, "evtstrm", &set_evtstrm},      //
  [ARM_AES] = {{0, ARM_HWCAP2_AES}, "aes", &set_aes},                     //
  [ARM_PMULL] = {{0, ARM_HWCAP2_PMULL}, "pmull", &set_pmull},             //
  [ARM_SHA1] = {{0, ARM_HWCAP2_SHA1}, "sha1", &set_sha1},                 //
  [ARM_SHA2] = {{0, ARM_HWCAP2_SHA2}, "sha2", &set_sha2},                 //
  [ARM_CRC32] = {{0, ARM_HWCAP2_CRC32}, "crc32", &set_crc32},             //
};

static const size_t kConfigsSize = sizeof(kConfigs) / sizeof(CapabilityConfig);

typedef struct {
  bool processor_reports_armv6;
  bool hardware_reports_goldfish;
} ProcCpuInfoData;

static int IndexOfNonDigit(StringView str) {
  size_t index = 0;
  while (str.size && isdigit(CpuFeatures_StringView_Front(str))) {
    str = CpuFeatures_StringView_PopFront(str, 1);
    ++index;
  }
  return index;
}

static bool HandleArmLine(const LineResult result, ArmInfo* const info,
                          ProcCpuInfoData* const proc_info) {
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
    } else if (CpuFeatures_StringView_IsEquals(key, str("CPU architecture"))) {
      // CPU architecture is a number that may be followed by letters. e.g.
      // "6TEJ", "7".
      const StringView digits =
          CpuFeatures_StringView_KeepFront(value, IndexOfNonDigit(value));
      info->architecture = CpuFeatures_StringView_ParsePositiveNumber(digits);
    } else if (CpuFeatures_StringView_IsEquals(key, str("Processor"))
               || CpuFeatures_StringView_IsEquals(key, str("model name")) ) {
      // Android reports this in a non-Linux standard "Processor" but sometimes
      // also in "model name", Linux reports it only in "model name"
      // see RaspberryPiZero (Linux) vs InvalidArmv7 (Android) test-cases
      proc_info->processor_reports_armv6 =
          CpuFeatures_StringView_IndexOf(value, str("(v6l)")) >= 0;
    } else if (CpuFeatures_StringView_IsEquals(key, str("Hardware"))) {
      proc_info->hardware_reports_goldfish =
          CpuFeatures_StringView_IsEquals(value, str("Goldfish"));
    }
  }
  return !result.eof;
}

uint32_t GetArmCpuId(const ArmInfo* const info) {
  return (ExtractBitRange(info->implementer, 7, 0) << 24) |
         (ExtractBitRange(info->variant, 3, 0) << 20) |
         (ExtractBitRange(info->part, 11, 0) << 4) |
         (ExtractBitRange(info->revision, 3, 0) << 0);
}

static void FixErrors(ArmInfo* const info,
                      ProcCpuInfoData* const proc_cpu_info_data) {
  // Fixing Samsung kernel reporting invalid cpu architecture.
  // http://code.google.com/p/android/issues/detail?id=10812
  if (proc_cpu_info_data->processor_reports_armv6 && info->architecture >= 7) {
    info->architecture = 6;
  }

  // Handle kernel configuration bugs that prevent the correct reporting of CPU
  // features.
  switch (GetArmCpuId(info)) {
    case 0x4100C080:
      // Special case: The emulator-specific Android 4.2 kernel fails to report
      // support for the 32-bit ARM IDIV instruction. Technically, this is a
      // feature of the virtual CPU implemented by the emulator. Note that it
      // could also support Thumb IDIV in the future, and this will have to be
      // slightly updated.
      if (info->architecture >= 7 &&
          proc_cpu_info_data->hardware_reports_goldfish) {
        info->features.idiva = true;
      }
      break;
    case 0x511004D0:
      // https://crbug.com/341598.
      info->features.neon = false;
      break;
    case 0x510006F2:
    case 0x510006F3:
      // The Nexus 4 (Qualcomm Krait) kernel configuration forgets to report
      // IDIV support.
      info->features.idiva = true;
      info->features.idivt = true;
      break;
  }

  // Propagate cpu features.
  if (info->features.vfpv4) info->features.vfpv3 = true;
  if (info->features.neon) info->features.vfpv3 = true;
  if (info->features.vfpv3) info->features.vfp = true;
}

static void FillProcCpuInfoData(ArmInfo* const info,
                                ProcCpuInfoData* proc_cpu_info_data) {
  const int fd = CpuFeatures_OpenFile("/proc/cpuinfo");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      if (!HandleArmLine(StackLineReader_NextLine(&reader), info,
                         proc_cpu_info_data)) {
        break;
      }
    }
    CpuFeatures_CloseFile(fd);
  }
}

static const ArmInfo kEmptyArmInfo;

static const ProcCpuInfoData kEmptyProcCpuInfoData;

ArmInfo GetArmInfo(void) {
  // capabilities are fetched from both getauxval and /proc/cpuinfo so we can
  // have some information if the executable is sandboxed (aka no access to
  // /proc/cpuinfo).
  ArmInfo info = kEmptyArmInfo;
  ProcCpuInfoData proc_cpu_info_data = kEmptyProcCpuInfoData;

  FillProcCpuInfoData(&info, &proc_cpu_info_data);
  CpuFeatures_OverrideFromHwCaps(kConfigsSize, kConfigs,
                                 CpuFeatures_GetHardwareCapabilities(),
                                 &info.features);

  FixErrors(&info, &proc_cpu_info_data);

  return info;
}

////////////////////////////////////////////////////////////////////////////////
// Introspection functions

int GetArmFeaturesEnumValue(const ArmFeatures* features,
                            ArmFeaturesEnum value) {
  switch (value) {
    case ARM_SWP:
      return features->swp;
    case ARM_HALF:
      return features->half;
    case ARM_THUMB:
      return features->thumb;
    case ARM_26BIT:
      return features->_26bit;
    case ARM_FASTMULT:
      return features->fastmult;
    case ARM_FPA:
      return features->fpa;
    case ARM_VFP:
      return features->vfp;
    case ARM_EDSP:
      return features->edsp;
    case ARM_JAVA:
      return features->java;
    case ARM_IWMMXT:
      return features->iwmmxt;
    case ARM_CRUNCH:
      return features->crunch;
    case ARM_THUMBEE:
      return features->thumbee;
    case ARM_NEON:
      return features->neon;
    case ARM_VFPV3:
      return features->vfpv3;
    case ARM_VFPV3D16:
      return features->vfpv3d16;
    case ARM_TLS:
      return features->tls;
    case ARM_VFPV4:
      return features->vfpv4;
    case ARM_IDIVA:
      return features->idiva;
    case ARM_IDIVT:
      return features->idivt;
    case ARM_VFPD32:
      return features->vfpd32;
    case ARM_LPAE:
      return features->lpae;
    case ARM_EVTSTRM:
      return features->evtstrm;
    case ARM_AES:
      return features->aes;
    case ARM_PMULL:
      return features->pmull;
    case ARM_SHA1:
      return features->sha1;
    case ARM_SHA2:
      return features->sha2;
    case ARM_CRC32:
      return features->crc32;
    case ARM_LAST_:
      break;
  }
  return false;
}

const char* GetArmFeaturesEnumName(ArmFeaturesEnum value) {
  if(value >= kConfigsSize)
    return "unknown feature";
  return kConfigs[value].proc_cpuinfo_flag;
}
