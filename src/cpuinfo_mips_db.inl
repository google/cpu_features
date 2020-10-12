// Copyright 2020 Google LLC
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

#if defined(FEATURE)
// FEATURE(ENUM, NAME, CPUINFO_FLAG, HWCAP, HWCAP2)
// clang-format off
FEATURE(MIPS_MSA, msa, "msa", MIPS_HWCAP_MSA, 0)
FEATURE(MIPS_EVA, eva, "eva", 0, 0)
FEATURE(MIPS_R6, r6, "r6", MIPS_HWCAP_R6, 0)
// clang-format on
#else
#error "The FEATURE macro must be defined"
#endif  // FEATURE
