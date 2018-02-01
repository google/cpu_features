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

#include "internal/hwcaps.h"
#include "cpu_features_macros.h"
#include "internal/filesystem.h"

#if defined(NDEBUG)
#define D(...)
#else
#include <stdio.h>
#define D(...)           \
  do {                   \
    printf(__VA_ARGS__); \
    fflush(stdout);      \
  } while (0)
#endif

#if defined(CPU_FEATURES_ARCH_MIPS) || defined(CPU_FEATURES_ARCH_ANY_ARM)
#define HWCAPS_ANDROID_MIPS_OR_ARM
#endif

#if defined(CPU_FEATURES_OS_LINUX_OR_ANDROID) && \
    !defined(HWCAPS_ANDROID_MIPS_OR_ARM)
#define HWCAPS_REGULAR_LINUX
#endif

#if defined(HWCAPS_ANDROID_MIPS_OR_ARM) || defined(HWCAPS_REGULAR_LINUX)
#define HWCAPS_SUPPORTED
#endif

////////////////////////////////////////////////////////////////////////////////
// Implementation of GetElfHwcapFromGetauxval
////////////////////////////////////////////////////////////////////////////////

// On Linux we simply use getauxval.
#if defined(HWCAPS_REGULAR_LINUX)
#include <dlfcn.h>
#include <sys/auxv.h>
static uint32_t GetElfHwcapFromGetauxval(uint32_t hwcap_type) {
  return getauxval(hwcap_type);
}
#endif  // defined(HWCAPS_REGULAR_LINUX)

// On Android we probe the system's C library for a 'getauxval' function and
// call it if it exits, or return 0 for failure. This function is available
// since API level 20.
//
// This code does *NOT* check for '__ANDROID_API__ >= 20' to support the edge
// case where some NDK developers use headers for a platform that is newer than
// the one really targetted by their application. This is typically done to use
// newer native APIs only when running on more recent Android versions, and
// requires careful symbol management.
//
// Note that getauxval() can't really be re-implemented here, because its
// implementation does not parse /proc/self/auxv. Instead it depends on values
// that are passed by the kernel at process-init time to the C runtime
// initialization layer.
#if defined(HWCAPS_ANDROID_MIPS_OR_ARM)
#include <dlfcn.h>
#define AT_HWCAP 16
#define AT_HWCAP2 26
typedef unsigned long getauxval_func_t(unsigned long);

static uint32_t GetElfHwcapFromGetauxval(uint32_t hwcap_type) {
  uint32_t ret = 0;
  void* libc_handle = NULL;
  getauxval_func_t* func = NULL;

  dlerror();  // Cleaning error state before calling dlopen.
  libc_handle = dlopen("libc.so", RTLD_NOW);
  if (!libc_handle) {
    D("Could not dlopen() C library: %s\n", dlerror());
    return 0;
  }
  func = (getauxval_func_t*)dlsym(libc_handle, "getauxval");
  if (!func) {
    D("Could not find getauxval() in C library\n");
  } else {
    // Note: getauxval() returns 0 on failure. Doesn't touch errno.
    ret = (uint32_t)(*func)(hwcap_type);
  }
  dlclose(libc_handle);
  return ret;
}
#endif  // defined(HWCAPS_ANDROID_MIPS_OR_ARM)

#if defined(HWCAPS_SUPPORTED)
////////////////////////////////////////////////////////////////////////////////
// Implementation of GetHardwareCapabilities for Android and Linux
////////////////////////////////////////////////////////////////////////////////

// Fallback when getauxval is not available, retrieves hwcaps from
// "/proc/self/auxv".
static uint32_t GetElfHwcapFromProcSelfAuxv(uint32_t hwcap_type) {
  struct {
    uint32_t tag;
    uint32_t value;
  } entry;
  uint32_t result = 0;
  const char filepath[] = "/proc/self/auxv";
  const int fd = OpenFile(filepath);
  if (fd < 0) {
    D("Could not open %s\n", filepath);
    return 0;
  }
  for (;;) {
    const int ret = ReadFile(fd, (char*)&entry, sizeof entry);
    if (ret < 0) {
      D("Error while reading %s\n", filepath);
      break;
    }
    // Detect end of list.
    if (ret == 0 || (entry.tag == 0 && entry.value == 0)) {
      break;
    }
    if (entry.tag == hwcap_type) {
      result = entry.value;
      break;
    }
  }
  CloseFile(fd);
  return result;
}

// Retrieves hardware capabilities by first trying to call getauxval, if not
// available falls back to reading "/proc/self/auxv".
static uint32_t GetHardwareCapabilitiesFor(uint32_t type) {
  uint32_t hwcaps = GetElfHwcapFromGetauxval(type);
  if (!hwcaps) {
    D("Parsing /proc/self/auxv to extract ELF hwcaps!\n");
    hwcaps = GetElfHwcapFromProcSelfAuxv(type);
  }
  return hwcaps;
}

HardwareCapabilities GetHardwareCapabilities(void) {
  HardwareCapabilities capabilities;
  capabilities.hwcaps = GetHardwareCapabilitiesFor(AT_HWCAP);
  capabilities.hwcaps2 = GetHardwareCapabilitiesFor(AT_HWCAP2);
  return capabilities;
}

#else  // (defined(HWCAPS_SUPPORTED)

////////////////////////////////////////////////////////////////////////////////
// Implementation of GetHardwareCapabilities for unsupported platforms.
////////////////////////////////////////////////////////////////////////////////

const HardwareCapabilities kEmptyHardwareCapabilities;
HardwareCapabilities GetHardwareCapabilities(void) {
  return kEmptyHardwareCapabilities;
}
#endif
