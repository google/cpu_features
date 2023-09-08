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

#ifdef CPU_FEATURES_ARCH_AARCH64
#if defined(CPU_FEATURES_OS_FREEBSD)

#if defined(HAVE_STRONG_ELF_AUX_INFO)
#include <stddef.h>
#include <sys/auxv.h>

#include "internal/hwcaps.h"

static unsigned long GetElfHwcapFromElfAuxInfo(int hwcap_type) {
  unsigned long hwcap;
  elf_aux_info(hwcap_type, &hwcap, sizeof(hwcap));
  return hwcap;
}

HardwareCapabilities CpuFeatures_GetHardwareCapabilities(void) {
  HardwareCapabilities capabilities;
  capabilities.hwcaps = GetElfHwcapFromElfAuxInfo(AT_HWCAP);
  capabilities.hwcaps2 = GetElfHwcapFromElfAuxInfo(AT_HWCAP2);
  return capabilities;
}
#else
#error "Error"
#endif  // HAVE_STRONG_ELF_AUX_INFO

#endif  // CPU_FEATURES_OS_FREEBSD
#endif  // CPU_FEATURES_ARCH_AARCH64
