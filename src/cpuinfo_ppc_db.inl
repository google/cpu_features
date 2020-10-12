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
FEATURE(PPC_32, ppc32, "ppc32", PPC_FEATURE_32, 0)
FEATURE(PPC_64, ppc64, "ppc64", PPC_FEATURE_64, 0)
FEATURE(PPC_601_INSTR, ppc601, "ppc601", PPC_FEATURE_601_INSTR, 0)
FEATURE(PPC_HAS_ALTIVEC, altivec, "altivec", PPC_FEATURE_HAS_ALTIVEC, 0)
FEATURE(PPC_HAS_FPU, fpu, "fpu", PPC_FEATURE_HAS_FPU, 0)
FEATURE(PPC_HAS_MMU, mmu, "mmu", PPC_FEATURE_HAS_MMU, 0)
FEATURE(PPC_HAS_4xxMAC, mac_4xx, "4xxmac", PPC_FEATURE_HAS_4xxMAC, 0)
FEATURE(PPC_UNIFIED_CACHE, unifiedcache, "ucache", PPC_FEATURE_UNIFIED_CACHE, 0)
FEATURE(PPC_HAS_SPE, spe, "spe", PPC_FEATURE_HAS_SPE, 0)
FEATURE(PPC_HAS_EFP_SINGLE, efpsingle, "efpsingle", PPC_FEATURE_HAS_EFP_SINGLE, 0)
FEATURE(PPC_HAS_EFP_DOUBLE, efpdouble, "efpdouble", PPC_FEATURE_HAS_EFP_DOUBLE, 0)
FEATURE(PPC_NO_TB, no_tb, "notb", PPC_FEATURE_NO_TB, 0)
FEATURE(PPC_POWER4, power4, "power4", PPC_FEATURE_POWER4, 0)
FEATURE(PPC_POWER5, power5, "power5", PPC_FEATURE_POWER5, 0)
FEATURE(PPC_POWER5_PLUS, power5plus, "power5+", PPC_FEATURE_POWER5_PLUS, 0)
FEATURE(PPC_CELL, cell, "cellbe", PPC_FEATURE_CELL, 0)
FEATURE(PPC_BOOKE, booke, "booke", PPC_FEATURE_BOOKE, 0)
FEATURE(PPC_SMT, smt, "smt", PPC_FEATURE_SMT, 0)
FEATURE(PPC_ICACHE_SNOOP, icachesnoop, "ic_snoop", PPC_FEATURE_ICACHE_SNOOP, 0)
FEATURE(PPC_ARCH_2_05, arch205, "arch_2_05", PPC_FEATURE_ARCH_2_05, 0)
FEATURE(PPC_PA6T, pa6t, "pa6t", PPC_FEATURE_PA6T, 0)
FEATURE(PPC_HAS_DFP, dfp, "dfp", PPC_FEATURE_HAS_DFP, 0)
FEATURE(PPC_POWER6_EXT, power6ext, "power6x", PPC_FEATURE_POWER6_EXT, 0)
FEATURE(PPC_ARCH_2_06, arch206, "arch_2_06", PPC_FEATURE_ARCH_2_06, 0)
FEATURE(PPC_HAS_VSX, vsx, "vsx", PPC_FEATURE_HAS_VSX, 0)
FEATURE(PPC_PSERIES_PERFMON_COMPAT, pseries_perfmon_compat, "archpmu", PPC_FEATURE_PSERIES_PERFMON_COMPAT, 0)
FEATURE(PPC_TRUE_LE, truele, "true_le", PPC_FEATURE_TRUE_LE, 0)
FEATURE(PPC_PPC_LE, ppcle, "ppcle", PPC_FEATURE_PPC_LE, 0)
FEATURE(PPC_ARCH_2_07, arch207, "arch_2_07", 0, PPC_FEATURE2_ARCH_2_07)
FEATURE(PPC_HTM, htm, "htm", 0, PPC_FEATURE2_HTM)
FEATURE(PPC_DSCR, dscr, "dscr", 0, PPC_FEATURE2_DSCR)
FEATURE(PPC_EBB, ebb, "ebb", 0, PPC_FEATURE2_EBB)
FEATURE(PPC_ISEL, isel, "isel", 0, PPC_FEATURE2_ISEL)
FEATURE(PPC_TAR, tar, "tar", 0, PPC_FEATURE2_TAR)
FEATURE(PPC_VEC_CRYPTO, vcrypto, "vcrypto", 0, PPC_FEATURE2_VEC_CRYPTO)
FEATURE(PPC_HTM_NOSC, htm_nosc, "htm-nosc", 0, PPC_FEATURE2_HTM_NOSC)
FEATURE(PPC_ARCH_3_00, arch300, "arch_3_00", 0, PPC_FEATURE2_ARCH_3_00)
FEATURE(PPC_HAS_IEEE128, ieee128, "ieee128", 0, PPC_FEATURE2_HAS_IEEE128)
FEATURE(PPC_DARN, darn, "darn", 0, PPC_FEATURE2_DARN)
FEATURE(PPC_SCV, scv, "scv", 0, PPC_FEATURE2_SCV)
FEATURE(PPC_HTM_NO_SUSPEND, htm_no_suspend, "htm-no-suspend", 0, PPC_FEATURE2_HTM_NO_SUSPEND)
// clang-format on
#else
#error "The FEATURE macro must be defined"
#endif  // FEATURE
