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

#include "cpu_features_macros.h"

//#ifndef CPU_FEATURES_ARCH_RISCV
//#if defined(CPU_FEATURES_OS_LINUX) || defined(CPU_FEATURES_OS_ANDROID)

#include "cpuinfo_riscv.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions for introspection.
////////////////////////////////////////////////////////////////////////////////
#define INTROSPECTION_TABLE                                      \
  LINE(RISCV_32, riscv32, "32", RISCV_HWCAP_32, 0)               \
  LINE(RISCV_64, riscv64, "64", RISCV_HWCAP_64, 0)               \
  LINE(RISCV_128, riscv128, "128", RISCV_HWCAP_128, 0)           \
  LINE(RISCV_A, a, "a", RISCV_HWCAP_A, 0)                        \
  LINE(RISCV_C, c, "c", RISCV_HWCAP_C, 0)                        \
  LINE(RISCV_D, d, "d", RISCV_HWCAP_D, 0)                        \
  LINE(RISCV_E, e, "e", RISCV_HWCAP_E, 0)                        \
  LINE(RISCV_F, f, "f", RISCV_HWCAP_F, 0)                        \
  LINE(RISCV_I, i, "i", RISCV_HWCAP_I, 0)                        \
  LINE(RISCV_M, m, "m", RISCV_HWCAP_M, 0)                        \
  LINE(RISCV_V, v, "v", RISCV_HWCAP_V, 0)                        \
  LINE(RISCV_Q, q, "q", RISCV_HWCAP_Q, 0)
#define INTROSPECTION_PREFIX Riscv
#define INTROSPECTION_ENUM_PREFIX RISCV
#include "define_introspection_and_hwcaps.inl"

////////////////////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////////////////////

/*
  UARCH FOUND:
  sifive,bullet0 -> Series 7
*/

#include "internal/stack_line_reader.h"
#include "internal/filesystem.h"

#include <stdbool.h>
#include <stdio.h>

static const RiscvInfo kEmptyRiscvInfo;

static bool HandleRiscVLine(const LineResult result,
                            RiscvInfo* const info) {
  StringView line = result.line;
  StringView key, value;
  if (CpuFeatures_StringView_GetAttributeKeyValue(line, &key, &value)) {
    if (CpuFeatures_StringView_IsEquals(key, str("isa"))) {
      //CpuFeatures_StringView_IndexOf
      StringView tmp_sv;
      for (size_t i = 0; i < RISCV_LAST_; ++i){
        tmp_sv.ptr = kCpuInfoFlags[i];
        tmp_sv.size = strlen(kCpuInfoFlags[i]);

        kSetters[i](&info->features,
                    CpuFeatures_StringView_IndexOf(value, tmp_sv) > 1);
        
      }
    }
    if (CpuFeatures_StringView_IsEquals(key, str("uarch"))){
      //TODO
    }
  }
  return !result.eof;
}

static void FillProcCpuInfoData(RiscvInfo* const info) {
  const int fd = CpuFeatures_OpenFile("/Users/danieleaffinita/Desktop/cpu/infocpu");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);

    for(;;){
      if(!HandleRiscVLine(StackLineReader_NextLine(&reader), info))
        break;
    }
    
    CpuFeatures_CloseFile(fd);
  }
}

RiscvInfo GetRiscvInfo(void){
    RiscvInfo info = kEmptyRiscvInfo;
    FillProcCpuInfoData(&info);

    //just testing
    printf("PRETEST\n");
    printf("32 -> %d\n",info.features.riscv32);
    printf("64 -> %d\n",info.features.riscv64);
    printf("128 -> %d\n",info.features.riscv128);
    printf("a -> %d\n",info.features.a);
    printf("c -> %d\n",info.features.c);
    printf("d -> %d\n",info.features.d);
    printf("e -> %d\n",info.features.e);
    printf("f -> %d\n",info.features.f);
    printf("i -> %d\n",info.features.i);
    printf("m -> %d\n",info.features.m);
    printf("v -> %d\n",info.features.v);
    printf("q -> %d\n",info.features.q);
    fflush(stdout);
}

//#endif  //  defined(CPU_FEATURES_OS_LINUX) || defined(CPU_FEATURES_OS_ANDROID)
//#endif  // CPU_FEATURES_ARCH_RISCV