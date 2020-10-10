// Copyright 2020 Google Inc.
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
FEATURE(ARM_SWP, swp, "swp", ARM_HWCAP_SWP, 0)
FEATURE(ARM_HALF, half, "half", ARM_HWCAP_HALF, 0)
FEATURE(ARM_THUMB, thumb, "thumb", ARM_HWCAP_THUMB, 0)
FEATURE(ARM_26BIT, _26bit, "26bit", ARM_HWCAP_26BIT, 0)
FEATURE(ARM_FASTMULT, fastmult, "fastmult", ARM_HWCAP_FAST_MULT, 0)
FEATURE(ARM_FPA, fpa, "fpa", ARM_HWCAP_FPA, 0)
FEATURE(ARM_VFP, vfp, "vfp", ARM_HWCAP_VFP, 0)
FEATURE(ARM_EDSP, edsp, "edsp", ARM_HWCAP_EDSP, 0)
FEATURE(ARM_JAVA, java, "java", ARM_HWCAP_JAVA, 0)
FEATURE(ARM_IWMMXT, iwmmxt, "iwmmxt", ARM_HWCAP_IWMMXT, 0)
FEATURE(ARM_CRUNCH, crunch, "crunch", ARM_HWCAP_CRUNCH, 0)
FEATURE(ARM_THUMBEE, thumbee, "thumbee", ARM_HWCAP_THUMBEE, 0)
FEATURE(ARM_NEON, neon, "neon", ARM_HWCAP_NEON, 0)
FEATURE(ARM_VFPV3, vfpv3, "vfpv3", ARM_HWCAP_VFPV3, 0)
FEATURE(ARM_VFPV3D16, vfpv3d16, "vfpv3d16", ARM_HWCAP_VFPV3D16, 0)
FEATURE(ARM_TLS, tls, "tls", ARM_HWCAP_TLS, 0)
FEATURE(ARM_VFPV4, vfpv4, "vfpv4", ARM_HWCAP_VFPV4, 0)
FEATURE(ARM_IDIVA, idiva, "idiva", ARM_HWCAP_IDIVA, 0)
FEATURE(ARM_IDIVT, idivt, "idivt", ARM_HWCAP_IDIVT, 0)
FEATURE(ARM_VFPD32, vfpd32, "vfpd32", ARM_HWCAP_VFPD32, 0)
FEATURE(ARM_LPAE, lpae, "lpae", ARM_HWCAP_LPAE, 0)
FEATURE(ARM_EVTSTRM, evtstrm, "evtstrm", ARM_HWCAP_EVTSTRM, 0)
FEATURE(ARM_AES, aes, "aes", 0, ARM_HWCAP2_AES)
FEATURE(ARM_PMULL, pmull, "pmull", 0, ARM_HWCAP2_PMULL)
FEATURE(ARM_SHA1, sha1, "sha1", 0, ARM_HWCAP2_SHA1)
FEATURE(ARM_SHA2, sha2, "sha2", 0, ARM_HWCAP2_SHA2)
FEATURE(ARM_CRC32, crc32, "crc32", 0, ARM_HWCAP2_CRC32)
// clang-format on
#else
#error "The FEATURE macro must be defined"
#endif  // FEATURE
