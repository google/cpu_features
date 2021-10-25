#include "cpuinfo_core_aarch64.h"

#include "bit_utils.h"
#include "cputype_aarch64.h"

///////////////////////////////////////////////////////////////////////////////
// Aarch64 info via mrs instruction
///////////////////////////////////////////////////////////////////////////////

uint64_t GetCpuid_MIDR_EL1() { return READ_MRS(MIDR_EL1); }
uint64_t GetCpuid_AA64ISAR0_EL1() { return READ_MRS(ID_AA64ISAR0_EL1); }
uint64_t GetCpuid_AA64ISAR1_EL1() { return READ_MRS(ID_AA64ISAR1_EL1); }
uint64_t GetCpuid_AA64PFR0_EL1() { return READ_MRS(ID_AA64PFR0_EL1); }

// Floating-point. Defined values are: 0b0000, 0b0001, 0b1111.
// All other values are reserved.
// Floating-point is not implemented when fp_value = 0b1111 or reserved.
// This field must have the same value as the AdvSimd field.
static void SetFpInfoAarch64(Aarch64InfoCpuid* info, uint64_t pfr0) {
  const uint64_t fp_value = ExtractBitRange(pfr0, 19, 16);

  // fp_value = 0b0000:
  // Floating-point is implemented, and includes support for:
  //  * Single-precision and double-precision floating-point types.
  //  * Conversions between single-precision and half-precision data types,
  //  and double-precision and half-precision data types.
  if (fp_value == 0) {
    info->features.fp = 1;
    return;
  }

  // fp_value = 0b0001:
  // As for 0b0000, and also includes support for half-precision
  // floating-point arithmetic.
  if (fp_value == 1) {
    info->features.fp = 1;
    info->features.fphp = 1;
  }
}

// Advanced SIMD. Defined values are: 0b0000, 0b0001, 0b1111.
// All other values are reserved.
// This field must have the same value as the FP field.
static void SetAdvSimdInfoAarch64(Aarch64InfoCpuid* info, uint64_t pfr0) {
  const uint64_t asimd_value = ExtractBitRange(pfr0, 23, 20);

  // Advanced SIMD is implemented, including support for the following SISD and
  // SIMD operations:
  //  * Integer byte, halfword, word and doubleword element operations.
  //  * Single-precision and double-precision floating-point arithmetic.
  //  * Conversions between single-precision and half-precision data types,
  // and double-precision and half-precision data types.
  if (asimd_value == 0) {
    info->features.asimd = 1;
    return;
  }

  // asimd_value = 0b0001:
  // As for 0b0000, and also includes support for half-precision
  // floating-point arithmetic.
  if (asimd_value == 1) {
    info->features.asimd = 1;
    info->features.asimdfhm = 1;
  }
}

// AMU indicates support for Activity Monitors Extension. Defined values are:
// 0b0000, 0b0001, 0b1111
static void SetAmuInfoAarch64(Aarch64InfoCpuid* info, uint64_t pfr0) {
  const uint64_t amu = ExtractBitRange(pfr0, 47, 44);

  // FEAT_AMUv1 is implemented.
  if (amu == 1) {
    info->features.amuv1 = 1;
    return;
  }

  // FEAT_AMUv1p1 is implemented. As 0b0001 and adds support for virtualization
  // of the activity monitor event counters.
  if (amu == 2) {
    info->features.amuv1 = 1;
    info->features.amuv1p1 = 1;
  }
}

static void SetInstructionSetAttribute0Aarch64(Aarch64InfoCpuid* info) {
  const uint64_t isa0 = GetCpuid_AA64ISAR0_EL1();
  info->features.aes = ExtractBitRange(isa0, 7, 4);
  info->features.sha1 = ExtractBitRange(isa0, 11, 8);
  info->features.sha2 = ExtractBitRange(isa0, 15, 12);
  info->features.crc32 = ExtractBitRange(isa0, 19, 16);
  info->features.atomics = ExtractBitRange(isa0, 23, 20);
  info->features.rdm = ExtractBitRange(isa0, 31, 28);
  info->features.atomics = ExtractBitRange(isa0, 31, 28);
  info->features.sha3 = ExtractBitRange(isa0, 35, 32);
  info->features.sm3 = ExtractBitRange(isa0, 39, 36);
  info->features.sm4 = ExtractBitRange(isa0, 43, 40);
  info->features.dp = ExtractBitRange(isa0, 47, 44);
  info->features.fhm = ExtractBitRange(isa0, 51, 48);
  info->features.ts = ExtractBitRange(isa0, 55, 52);
  info->features.tlb = ExtractBitRange(isa0, 59, 56);
  info->features.rndr = ExtractBitRange(isa0, 63, 60);
}

static void SetInstructionSetAttribute1Aarch64(Aarch64InfoCpuid* info) {
  const uint64_t isa1 = GetCpuid_AA64ISAR1_EL1();
  info->features.ls64 = ExtractBitRange(isa1, 63, 60);
  info->features.xs = ExtractBitRange(isa1, 59, 56);
  info->features.i8mm = ExtractBitRange(isa1, 55, 52);
  info->features.dgh = ExtractBitRange(isa1, 51, 48);
  info->features.bf16 = ExtractBitRange(isa1, 47, 44);
  info->features.specres = ExtractBitRange(isa1, 43, 40);
  info->features.sb = ExtractBitRange(isa1, 39, 36);
  info->features.frintts = ExtractBitRange(isa1, 35, 32);
  info->features.gpi = ExtractBitRange(isa1, 31, 28);
  info->features.gpa = ExtractBitRange(isa1, 27, 24);
  info->features.lrcpc = ExtractBitRange(isa1, 23, 20);
  info->features.fcma = ExtractBitRange(isa1, 19, 16);
  info->features.jscvt = ExtractBitRange(isa1, 15, 12);
  info->features.api = ExtractBitRange(isa1, 11, 8);
  info->features.apa = ExtractBitRange(isa1, 7, 4);
  info->features.dpb = ExtractBitRange(isa1, 3, 0);
}

static void SetProcessorFeatureRegister0Aarch64(Aarch64InfoCpuid* info) {
  const uint64_t pfr0 = GetCpuid_AA64PFR0_EL1();
  info->features.el0 = ExtractBitRange(pfr0, 3, 0);
  info->features.el1 = ExtractBitRange(pfr0, 7, 4);

  const uint64_t el2 = ExtractBitRange(pfr0, 11, 8);
  if (el2 == 1) info->features.el2_64 = 1;
  if (el2 == 2) info->features.el2_32_64 = 1;

  const uint64_t el3 = ExtractBitRange(pfr0, 15, 12);
  if (el3 == 1) info->features.el3_64 = 1;
  if (el3 == 2) info->features.el3_32_64 = 1;

  const uint64_t gic = ExtractBitRange(pfr0, 27, 24);
  if (gic == 1) info->features.gicv4 = 1;
  if (gic == 2) info->features.gicv41 = 1;

  const uint64_t ras = ExtractBitRange(pfr0, 31, 28);
  if (ras == 1) info->features.ras = 1;
  if (ras == 2) {
    info->features.ras = 1;
    info->features.rasv1p1 = 1;
  }
  if (ras >= 2 && info->features.el3_64 == 1) info->features.double_fault = 1;

  info->features.sve = ExtractBitRange(pfr0, 35, 32);
  info->features.sel2 = ExtractBitRange(pfr0, 39, 36);
  info->features.mpam = ExtractBitRange(pfr0, 43, 40);
  info->features.dit = ExtractBitRange(pfr0, 51, 48);

  SetFpInfoAarch64(info, pfr0);
  SetAdvSimdInfoAarch64(info, pfr0);
  SetAmuInfoAarch64(info, pfr0);
}

static Aarch64PartNum GetPartNumAarch64(const uint64_t midr) {
  const uint64_t midr_part_num = ExtractBitRange(midr, 15, 4);
  switch (midr_part_num) {
    case ARM_CPU_PART_AEM_V8:
      return AEM_V8;
    case ARM_CPU_PART_FOUNDATION:
      return FOUNDATION;
    case ARM_CPU_PART_NEOVERSE_N1:
      return NEOVERSE_N1;
    case ARM_CPU_PART_CORTEX_A35:
      return CORTEX_A35;
    case ARM_CPU_PART_CORTEX_A53:
      return CORTEX_A53;
    case ARM_CPU_PART_CORTEX_A55:
      return CORTEX_A55;
    case ARM_CPU_PART_CORTEX_A57:
      return CORTEX_A57;
    case ARM_CPU_PART_CORTEX_A72:
      return CORTEX_A72;
    case ARM_CPU_PART_CORTEX_A73:
      return CORTEX_A73;
    case ARM_CPU_PART_CORTEX_A75:
      return CORTEX_A75;
    case ARM_CPU_PART_CORTEX_A76:
      return CORTEX_A76;
    case ARM_CPU_PART_CORTEX_A77:
      return CORTEX_A77;
    case APM_CPU_PART_POTENZA:
      return POTENZA;
    case CAVIUM_CPU_PART_THUNDERX:
      return THUNDERX;
    case CAVIUM_CPU_PART_THUNDERX_81XX:
      return THUNDERX_81XX;
    case CAVIUM_CPU_PART_THUNDERX_83XX:
      return THUNDERX_83XX;
    case CAVIUM_CPU_PART_THUNDERX2:
      return THUNDERX2;
    case BRCM_CPU_PART_BRAHMA_B53:
      return BRAHMA_B53;
    case BRCM_CPU_PART_VULCAN:
      return VULCAN;
    case QCOM_CPU_PART_FALKOR_V1:
      return FALKOR_V1;
    case QCOM_CPU_PART_FALKOR:
      return FALKOR;
    case QCOM_CPU_PART_KRYO:
      return KRYO;
    case QCOM_CPU_PART_KRYO_2XX_SILVER:
      return KRYO_2XX_SILVER;
    case QCOM_CPU_PART_KRYO_3XX_SILVER:
      return KRYO_3XX_SILVER;
    case QCOM_CPU_PART_KRYO_4XX_GOLD:
      return KRYO_4XX_GOLD;
    case QCOM_CPU_PART_KRYO_4XX_SILVER:
      return KRYO_4XX_SILVER;
    case NVIDIA_CPU_PART_DENVER:
      return DENVER;
    case NVIDIA_CPU_PART_CARMEL:
      return CARMEL;
    case FUJITSU_CPU_PART_A64FX:
      return A64FX;
    case HISI_CPU_PART_TSV110:
      return TSV110;
    case APPLE_CPU_PART_M1_FIRESTORM:
      return M1_FIRESTORM;
    case APPLE_CPU_PART_M1_ICESTORM:
      return M1_ICESTORM;
    case APPLE_CPU_PART_M1_FIRESTORM_TONGA:
      return M1_FIRESTORM_TONGA;
    case APPLE_CPU_PART_M1_ICESTORM_TONGA:
      return M1_ICESTORM_TONGA;
    default:
      return UNKNOWN_PART_NUM_AARCH64;
  }
}

static Aarch64Implementer GetImplementerAarch64(const uint64_t midr) {
  const uint64_t midr_impl = ExtractBitRange(midr, 31, 24);
  switch (midr_impl) {
    case ARM_CPU_IMP_RESERVED:
      return RESERVED;
    case ARM_CPU_IMP_ARM:
      return ARM;
    case ARM_CPU_IMP_APM:
      return APM;
    case ARM_CPU_IMP_APPLE:
      return APPLE;
    case ARM_CPU_IMP_BRCM:
      return BRCM;
    case ARM_CPU_IMP_CAVIUM:
      return CAVIUM;
    case ARM_CPU_IMP_FUJITSU:
      return FUJITSU;
    case ARM_CPU_IMP_HISI:
      return HISI;
    case ARM_CPU_IMP_NVIDIA:
      return NVIDIA;
    case ARM_CPU_IMP_QCOM:
      return QCOM;
    case ARM_CPU_IMP_DEQ:
      return DEQ;
    case ARM_CPU_IMP_INFINEON:
      return INFINEON;
    case ARM_CPU_IMP_MOTOROLA:
      return MOTOROLA;
    case ARM_CPU_IMP_MARVELL:
      return MARVELL;
    case ARM_CPU_IMP_INTEL:
      return INTEL;
    case ARM_CPU_IMP_AMPERE:
      return AMPERE;
    default:
      return UNKNOWN_IMPL_AARCH64;
  }
}

static Aarch64Architecture GetArchitectureAarch64(const uint64_t midr) {
  const uint64_t architecture = ExtractBitRange(midr, 19, 16);
  switch (architecture) {
    case CPU_ARCH_ARMv4:
      return ARMv4;
    case CPU_ARCH_ARMv4T:
      return ARMv4T;
    case CPU_ARCH_ARMv5:
      return ARMv5;
    case CPU_ARCH_ARMv5T:
      return ARMv5T;
    case CPU_ARCH_ARMv5TE:
      return ARMv5TE;
    case CPU_ARCH_ARMv5TEJ:
      return ARMv5TEJ;
    case CPU_ARCH_ARMv6:
      return ARMv6;
    case CPU_ARCH_ARMv7:
      return ARMv7;
    case CPU_ARCH_ARMv7F:
      return ARMv7F;
    case CPU_ARCH_ARMv7S:
      return ARMv7S;
    case CPU_ARCH_ARMv7K:
      return ARMv7K;
    case CPU_ARCH_ARMv8:
      return ARMv8;
    case CPU_ARCH_ARMv8E:
      return ARMv8E;
    default:
      return UNKNOWN_ARCH_AARCH64;
  }
}

static const Aarch64InfoCpuid kEmptyAarch64InfoMrs;

Aarch64InfoCpuid GetAarch64InfoCpuid(void) {
  Aarch64InfoCpuid info = kEmptyAarch64InfoMrs;
  const uint64_t midr = GetCpuid_MIDR_EL1();
  info.implementer = GetImplementerAarch64(midr);
  info.variant = ExtractBitRange(midr, 23, 20);
  info.architecture = GetArchitectureAarch64(midr);
  info.part = GetPartNumAarch64(midr);
  info.revision = ExtractBitRange(midr, 3, 0);
  SetProcessorFeatureRegister0Aarch64(&info);
  SetInstructionSetAttribute0Aarch64(&info);
  SetInstructionSetAttribute1Aarch64(&info);
  return info;
}

const char* GetAarch64ImplementerString(const Aarch64InfoCpuid* info) {
  switch (info->implementer) {
    case RESERVED:
      return "RESERVED";
    case ARM:
      return "ARM";
    case CAVIUM:
      return "CAVIUM";
    case BRCM:
      return "BRCM";
    case HISI:
      return "HISI";
    case DEQ:
      return "DEQ";
    case FUJITSU:
      return "FUJITSU";
    case INFINEON:
      return "INFINEON";
    case MOTOROLA:
      return "MOTOROLA";
    case NVIDIA:
      return "NVIDIA";
    case APM:
      return "APM";
    case QCOM:
      return "QCOM";
    case MARVELL:
      return "MARVELL";
    case APPLE:
      return "APPLE";
    case INTEL:
      return "INTEL";
    case AMPERE:
      return "AMPERE";
    case UNKNOWN_IMPL_AARCH64:
      return "UNKNOWN_IMPL_AARCH64";
  }
}
const char* GetAarch64PartNumString(const Aarch64InfoCpuid* info) {
  switch (info->part) {
    case UNKNOWN_PART_NUM_AARCH64:
      return "UNKNOWN_PART_NUM_AARCH64";
    case AEM_V8:
      return "AEM_V8";
    case FOUNDATION:
      return "FOUNDATION";
    case CORTEX_A35:
      return "CORTEX_A35";
    case CORTEX_A53:
      return "CORTEX_A53";
    case CORTEX_A55:
      return "CORTEX_A55";
    case CORTEX_A57:
      return "CORTEX_A57";
    case CORTEX_A72:
      return "CORTEX_A72";
    case CORTEX_A73:
      return "CORTEX_A73";
    case CORTEX_A75:
      return "CORTEX_A75";
    case CORTEX_A76:
      return "CORTEX_A76";
    case CORTEX_A77:
      return "CORTEX_A77";
    case NEOVERSE_N1:
      return "NEOVERSE_N1";
    case POTENZA:
      return "POTENZA";
    case THUNDERX:
      return "THUNDERX";
    case THUNDERX_81XX:
      return "THUNDERX_81XX";
    case THUNDERX_83XX:
      return "THUNDERX_83XX";
    case THUNDERX2:
      return "THUNDERX2";
    case BRAHMA_B53:
      return "BRAHMA_B53";
    case VULCAN:
      return "VULCAN";
    case FALKOR_V1:
      return "FALKOR_V1";
    case FALKOR:
      return "FALKOR";
    case KRYO:
      return "KRYO";
    case KRYO_2XX_GOLD:
      return "KRYO_2XX_GOLD";
    case KRYO_2XX_SILVER:
      return "KRYO_2XX_SILVER";
    case KRYO_3XX_SILVER:
      return "KRYO_3XX_SILVER";
    case KRYO_4XX_GOLD:
      return "KRYO_4XX_GOLD";
    case KRYO_4XX_SILVER:
      return "KRYO_4XX_SILVER";
    case DENVER:
      return "DENVER";
    case CARMEL:
      return "CARMEL";
    case A64FX:
      return "A64FX";
    case TSV110:
      return "TSV110";
    case M1_ICESTORM:
      return "M1_ICESTORM";
    case M1_FIRESTORM:
      return "M1_FIRESTORM";
    case M1_ICESTORM_TONGA:
      return "M1_ICESTORM_TONGA";
    case M1_FIRESTORM_TONGA:
      return "M1_FIRESTORM_TONGA";
  }
}
const char* GetAarch64ArchitectureString(const Aarch64InfoCpuid* info) {
  switch (info->architecture) {
    case ARMv4:
      return "ARMv4";
    case ARMv4T:
      return "ARMv4T";
    case ARMv5:
      return "ARMv5";
    case ARMv5T:
      return "ARMv5T";
    case ARMv5TE:
      return "ARMv5TE";
    case ARMv5TEJ:
      return "ARMv5TEJ";
    case ARMv6:
      return "ARMv6";
    case ARMv7:
      return "ARMv7";
    case ARMv7F:
      return "ARMv7F";
    case ARMv7S:
      return "ARMv7S";
    case ARMv7K:
      return "ARMv7K";
    case ARMv8:
      return "ARMv8";
    case ARMv8E:
      return "ARMv8E";
    default:
      return "UNKNOWN_ARCH_AARCH64";
  }
}