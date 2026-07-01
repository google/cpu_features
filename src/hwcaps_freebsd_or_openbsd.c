// Copyright 2023 Google LLC
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

#include "cpu_features_macros.h"

#if defined(CPU_FEATURES_OS_FREEBSD) || defined(CPU_FEATURES_OS_OPENBSD)

#include "internal/hwcaps.h"

#ifdef CPU_FEATURES_TEST
// In test mode, hwcaps_for_testing will define the following functions.
HardwareCapabilities CpuFeatures_GetHardwareCapabilities(void);
const char* CpuFeatures_GetPlatformPointer(void);
const char* CpuFeatures_GetBasePlatformPointer(void);
#else

#ifdef HAVE_STRONG_ELF_AUX_INFO
#include <stddef.h>
#include <sys/auxv.h>

static unsigned long GetElfHwcapFromElfAuxInfo(int hwcap_type) {
  unsigned long hwcap = 0;
  // elf_aux_info() leaves the output buffer untouched and returns non-zero when
  // the requested entry is absent (e.g. AT_HWCAP2 on some arch/kernel combos),
  // so return 0 instead of an indeterminate value.
  if (elf_aux_info(hwcap_type, &hwcap, sizeof(hwcap)) != 0) return 0;
  return hwcap;
}

HardwareCapabilities CpuFeatures_GetHardwareCapabilities(void) {
  HardwareCapabilities capabilities;
  capabilities.hwcaps = GetElfHwcapFromElfAuxInfo(AT_HWCAP);
  capabilities.hwcaps2 = GetElfHwcapFromElfAuxInfo(AT_HWCAP2);
  return capabilities;
}

const char *CpuFeatures_GetPlatformPointer(void) { return NULL; }

const char *CpuFeatures_GetBasePlatformPointer(void) { return NULL; }

#else
#error "FreeBSD / OpenBSD needs support for elf_aux_info"
#endif  // HAVE_STRONG_ELF_AUX_INFO

#endif  // CPU_FEATURES_TEST
#endif  // CPU_FEATURES_OS_FREEBSD || CPU_FEATURES_OS_OPENBSD
