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

#ifndef CPU_FEATURES_INCLUDE_CPUINFO_COMMON_H_
#define CPU_FEATURES_INCLUDE_CPUINFO_COMMON_H_

#include "cpu_features_macros.h"

CPU_FEATURES_START_CPP_NAMESPACE

typedef enum {
  EMPTY = 0,
  DATA_CACHE = 1,
  INSTRUCTION_CACHE = 2,
  UNIFIED_CACHE = 3,
  TLB = 4,
  DTLB = 5,
  STLB = 6,
  PREFETCH = 7
} CacheType;

typedef struct {
  int level;
  CacheType cache_type;
  int cache_size;    // Cache size in bytes
  int ways;          // Associativity, 0 undefined, 0xFF fully associative
  int line_size;     // Cache line size in bytes
  int tlb_entries;   // number of entries for TLB
  int partitioning;  // number of lines per sector
} CacheLevelInfo;

// Increase this value if more cache levels are needed.
#ifndef CPU_FEATURES_MAX_CACHE_LEVEL
#define CPU_FEATURES_MAX_CACHE_LEVEL 10
#endif
typedef struct {
  int size;
  CacheLevelInfo levels[CPU_FEATURES_MAX_CACHE_LEVEL];
} CacheInfo;

CPU_FEATURES_END_CPP_NAMESPACE

#endif  // CPU_FEATURES_INCLUDE_CPUINFO_COMMON_H_