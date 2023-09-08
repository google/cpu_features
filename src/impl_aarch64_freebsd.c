#include "cpu_features_macros.h"

#ifdef CPU_FEATURES_ARCH_AARCH64
#ifdef CPU_FEATURES_OS_FREEBSD

#include "cpuinfo_aarch64.h"

////////////////////////////////////////////////////////////////////////////////
// Definitions for introspection.
////////////////////////////////////////////////////////////////////////////////
#define INTROSPECTION_TABLE                  \
  LINE(AARCH64_FP, fp, , , )                 \
  LINE(AARCH64_ASIMD, asimd, , , )           \
  LINE(AARCH64_EVTSTRM, evtstrm, , , )       \
  LINE(AARCH64_AES, aes, , , )               \
  LINE(AARCH64_PMULL, pmull, , , )           \
  LINE(AARCH64_SHA1, sha1, , , )             \
  LINE(AARCH64_SHA2, sha2, , , )             \
  LINE(AARCH64_CRC32, crc32, , , )           \
  LINE(AARCH64_ATOMICS, atomics, , , )       \
  LINE(AARCH64_FPHP, fphp, , , )             \
  LINE(AARCH64_ASIMDHP, asimdhp, , , )       \
  LINE(AARCH64_CPUID, cpuid, , , )           \
  LINE(AARCH64_ASIMDRDM, asimdrdm, , , )     \
  LINE(AARCH64_JSCVT, jscvt, , , )           \
  LINE(AARCH64_FCMA, fcma, , , )             \
  LINE(AARCH64_LRCPC, lrcpc, , , )           \
  LINE(AARCH64_DCPOP, dcpop, , , )           \
  LINE(AARCH64_SHA3, sha3, , , )             \
  LINE(AARCH64_SM3, sm3, , , )               \
  LINE(AARCH64_SM4, sm4, , , )               \
  LINE(AARCH64_ASIMDDP, asimddp, , , )       \
  LINE(AARCH64_SHA512, sha512, , , )         \
  LINE(AARCH64_SVE, sve, , , )               \
  LINE(AARCH64_ASIMDFHM, asimdfhm, , , )     \
  LINE(AARCH64_DIT, dit, , , )               \
  LINE(AARCH64_USCAT, uscat, , , )           \
  LINE(AARCH64_ILRCPC, ilrcpc, , , )         \
  LINE(AARCH64_FLAGM, flagm, , , )           \
  LINE(AARCH64_SSBS, ssbs, , , )             \
  LINE(AARCH64_SB, sb, , , )                 \
  LINE(AARCH64_PACA, paca, , , )             \
  LINE(AARCH64_PACG, pacg, , , )             \
  LINE(AARCH64_DCPODP, dcpodp, , , )         \
  LINE(AARCH64_SVE2, sve2, , , )             \
  LINE(AARCH64_SVEAES, sveaes, , , )         \
  LINE(AARCH64_SVEPMULL, svepmull, , , )     \
  LINE(AARCH64_SVEBITPERM, svebitperm, , , ) \
  LINE(AARCH64_SVESHA3, svesha3, , , )       \
  LINE(AARCH64_SVESM4, svesm4, , , )         \
  LINE(AARCH64_FLAGM2, flagm2, , , )         \
  LINE(AARCH64_FRINT, frint, , , )           \
  LINE(AARCH64_SVEI8MM, svei8mm, , , )       \
  LINE(AARCH64_SVEF32MM, svef32mm, , , )     \
  LINE(AARCH64_SVEF64MM, svef64mm, , , )     \
  LINE(AARCH64_SVEBF16, svebf16, , , )       \
  LINE(AARCH64_I8MM, i8mm, , , )             \
  LINE(AARCH64_BF16, bf16, , , )             \
  LINE(AARCH64_DGH, dgh, , , )               \
  LINE(AARCH64_RNG, rng, , , )               \
  LINE(AARCH64_BTI, bti, , , )               \
  LINE(AARCH64_MTE, mte, , , )               \
  LINE(AARCH64_ECV, ecv, , , )               \
  LINE(AARCH64_AFP, afp, , , )               \
  LINE(AARCH64_RPRES, rpres, , , )
#define INTROSPECTION_PREFIX Aarch64
#define INTROSPECTION_ENUM_PREFIX AARCH64
#include "define_introspection.inl"

////////////////////////////////////////////////////////////////////////////////
// Implementation.
////////////////////////////////////////////////////////////////////////////////

#include "internal/filesystem.h"
#include "internal/stack_line_reader.h"
#include "internal/string_view.h"

static const Aarch64Info kEmptyAarch64Info;

#define ID_AA64ISAR0_KEY " Instruction Set Attributes 0"
#define ID_AA64ISAR1_KEY " Instruction Set Attributes 1"
#define ID_AA64ISAR2_KEY " Instruction Set Attributes 2"
#define ID_AA64PFR0_KEY "         Processor Features 0"
#define ID_AA64PFR1_KEY "         Processor Features 1"
#define ID_AA64ZFR0_KEY "               SVE Features 0"

typedef struct {
  const char* name;
  Aarch64FeaturesEnum* features_enum;
} MrsFeature;

#define MRS_FEATURE(feature, mrs_prefix, feat_suffix) \
  (MrsFeature) { feature, mrs_prefix##_##feat_suffix }

#define MRS_FEATURE_END \
  (MrsFeature) { NULL, NULL }

#define ID_AA64ISAR0_SET_FEATURE(feat_name, feature) \
  MRS_FEATURE(feature, id_aa64isar0, feat_name)

#define ID_AA64ISAR1_SET_FEATURE(feat_name, feature) \
  MRS_FEATURE(feature, id_aa64isar1, feat_name)

#define ID_AA64ISAR2_SET_FEATURE(feat_name, feature) \
  MRS_FEATURE(feature, id_aa64isar2, feat_name)

#define ID_AA64PFR0_SET_FEATURE(feat_name, feature) \
  MRS_FEATURE(feature, id_aa64pfr0, feat_name)

#define ID_AA64PFR1_SET_FEATURE(feat_name, feature) \
  MRS_FEATURE(feature, id_aa64pfr1, feat_name)

#define ID_AA64ZFR0_SET_FEATURE(feat_name, feature) \
  MRS_FEATURE(feature, id_aa64zfr0, feat_name)

typedef struct {
  const char* key;
  MrsFeature* features;
} MrsField;

#define MRS_FIELD(mrs, mrs_features) \
  (MrsField) { mrs, mrs_features }

#define MRS_FIELD_END \
  (MrsField) { NULL, NULL }

#define AARCH64_FEAT_END AARCH64_LAST_

#define MRS_FEATURES(mrs_name, feat_name, ...)            \
  static Aarch64FeaturesEnum mrs_name##_##feat_name[] = { \
      __VA_ARGS__,                                        \
      AARCH64_FEAT_END,                                   \
  };

#define ID_AA64ISAR0_FEATURES(feat_name, ...) \
  MRS_FEATURES(id_aa64isar0, feat_name, __VA_ARGS__)

#define ID_AA64ISAR1_FEATURES(feat_name, ...) \
  MRS_FEATURES(id_aa64isar1, feat_name, __VA_ARGS__)

#define ID_AA64ISAR2_FEATURES(feat_name, ...) \
  MRS_FEATURES(id_aa64isar2, feat_name, __VA_ARGS__)

#define ID_AA64PFR0_FEATURES(feat_name, ...) \
  MRS_FEATURES(id_aa64pfr0, feat_name, __VA_ARGS__)

#define ID_AA64PFR1_FEATURES(feat_name, ...) \
  MRS_FEATURES(id_aa64pfr1, feat_name, __VA_ARGS__)

#define ID_AA64ZFR0_FEATURES(feat_name, ...) \
  MRS_FEATURES(id_aa64zfr0, feat_name, __VA_ARGS__)

// ID_AA64ISAR0_EL1
// -----------------------------------------------------------------------------
ID_AA64ISAR0_FEATURES(fhm, AARCH64_ASIMDFHM)
ID_AA64ISAR0_FEATURES(dp, AARCH64_ASIMDDP)
ID_AA64ISAR0_FEATURES(crc32, AARCH64_CRC32)
ID_AA64ISAR0_FEATURES(rdm, AARCH64_ASIMDRDM)
ID_AA64ISAR0_FEATURES(atomics, AARCH64_ATOMICS)
ID_AA64ISAR0_FEATURES(aes, AARCH64_AES)
ID_AA64ISAR0_FEATURES(sha1, AARCH64_SHA1)
ID_AA64ISAR0_FEATURES(sha2, AARCH64_SHA2)
ID_AA64ISAR0_FEATURES(sha3, AARCH64_SHA3)
ID_AA64ISAR0_FEATURES(sm3, AARCH64_SM3)
ID_AA64ISAR0_FEATURES(sm4, AARCH64_SM4)
ID_AA64ISAR0_FEATURES(aes_pmull, AARCH64_AES, AARCH64_PMULL)
ID_AA64ISAR0_FEATURES(sha2_sha512, AARCH64_SHA2, AARCH64_SHA512)
ID_AA64ISAR0_FEATURES(flagm, AARCH64_FLAGM)
ID_AA64ISAR0_FEATURES(flagm2, AARCH64_FLAGM, AARCH64_FLAGM2)
ID_AA64ISAR0_FEATURES(rng, AARCH64_RNG)

static MrsFeature id_aa64isar0[] = {
    ID_AA64ISAR0_SET_FEATURE(fhm, "FHM"),
    ID_AA64ISAR0_SET_FEATURE(dp, "DP"),
    ID_AA64ISAR0_SET_FEATURE(crc32, "CRC32"),
    ID_AA64ISAR0_SET_FEATURE(rdm, "RDM"),
    ID_AA64ISAR0_SET_FEATURE(atomics, "Atomic"),
    ID_AA64ISAR0_SET_FEATURE(aes, "AES"),
    ID_AA64ISAR0_SET_FEATURE(sha1, "SHA1"),
    ID_AA64ISAR0_SET_FEATURE(sha2, "SHA2"),
    ID_AA64ISAR0_SET_FEATURE(sha3, "SHA3"),
    ID_AA64ISAR0_SET_FEATURE(sm3, "SM3"),
    ID_AA64ISAR0_SET_FEATURE(sm4, "SM4"),
    ID_AA64ISAR0_SET_FEATURE(aes_pmull, "AES+PMULL"),
    ID_AA64ISAR0_SET_FEATURE(sha2_sha512, "SHA2+SHA512"),
    ID_AA64ISAR0_SET_FEATURE(flagm, "CondM-8.4"),
    ID_AA64ISAR0_SET_FEATURE(flagm2, "CondM-8.5"),
    ID_AA64ISAR0_SET_FEATURE(rng, "RNG"),
    MRS_FEATURE_END,
};

// ID_AA64ISAR1_EL1
// -----------------------------------------------------------------------------
ID_AA64ISAR1_FEATURES(i8mm, AARCH64_I8MM)
ID_AA64ISAR1_FEATURES(dgh, AARCH64_DGH)
ID_AA64ISAR1_FEATURES(bf16, AARCH64_BF16)
ID_AA64ISAR1_FEATURES(sb, AARCH64_SB)
ID_AA64ISAR1_FEATURES(frint, AARCH64_FRINT)
ID_AA64ISAR1_FEATURES(pacg_gpi, AARCH64_PACG)
ID_AA64ISAR1_FEATURES(pacg_gpa, AARCH64_PACG)
ID_AA64ISAR1_FEATURES(api_pac, AARCH64_PACA)
ID_AA64ISAR1_FEATURES(api_epac, AARCH64_PACA)
ID_AA64ISAR1_FEATURES(api_epac2, AARCH64_PACA)
ID_AA64ISAR1_FEATURES(api_fpac, AARCH64_PACA)
ID_AA64ISAR1_FEATURES(api_fpac_combined, AARCH64_PACA)
ID_AA64ISAR1_FEATURES(paca, AARCH64_PACA)
ID_AA64ISAR1_FEATURES(lrcpc, AARCH64_LRCPC)
ID_AA64ISAR1_FEATURES(ilrcpc, AARCH64_LRCPC, AARCH64_ILRCPC)
ID_AA64ISAR1_FEATURES(fcma, AARCH64_FCMA)
ID_AA64ISAR1_FEATURES(jscvt, AARCH64_JSCVT)
ID_AA64ISAR1_FEATURES(dcpop, AARCH64_DCPOP)
ID_AA64ISAR1_FEATURES(dcpodp, AARCH64_DCPOP, AARCH64_DCPODP)

static MrsFeature id_aa64isar1[] = {
    ID_AA64ISAR1_SET_FEATURE(i8mm, "I8MM"),
    ID_AA64ISAR1_SET_FEATURE(dgh, "DGH"),
    ID_AA64ISAR1_SET_FEATURE(bf16, "BF16"),
    ID_AA64ISAR1_SET_FEATURE(sb, "SB"),
    ID_AA64ISAR1_SET_FEATURE(frint, "FRINTTS"),
    ID_AA64ISAR1_SET_FEATURE(pacg_gpi, "GPI"),
    ID_AA64ISAR1_SET_FEATURE(pacg_gpa, "GPA"),
    ID_AA64ISAR1_SET_FEATURE(lrcpc, "RCPC-8.3"),
    ID_AA64ISAR1_SET_FEATURE(ilrcpc, "RCPC-8.4"),
    ID_AA64ISAR1_SET_FEATURE(fcma, "FCMA"),
    ID_AA64ISAR1_SET_FEATURE(jscvt, "JSCVT"),
    ID_AA64ISAR1_SET_FEATURE(dcpop, "DCPoP"),
    ID_AA64ISAR1_SET_FEATURE(dcpodp, "DCCVADP"),
    ID_AA64ISAR1_SET_FEATURE(api_pac, "API PAC"),
    ID_AA64ISAR1_SET_FEATURE(api_epac, "API EPAC"),
    ID_AA64ISAR1_SET_FEATURE(api_epac2, "Impl PAuth+EPAC2"),
    ID_AA64ISAR1_SET_FEATURE(api_fpac, "Impl PAuth+FPAC"),
    ID_AA64ISAR1_SET_FEATURE(api_fpac_combined, "Impl PAuth+FPAC+Combined"),
    MRS_FEATURE_END,
};

// ID_AA64ISAR2_EL1
// -----------------------------------------------------------------------------
ID_AA64ISAR2_FEATURES(rpres, AARCH64_RPRES)

static MrsFeature id_aa64isar2[] = {
    ID_AA64ISAR2_SET_FEATURE(rpres, "RPRES"),
    MRS_FEATURE_END,
};

// ID_AA64PFR0_EL1
// -----------------------------------------------------------------------------
ID_AA64PFR0_FEATURES(fp, AARCH64_FP)
ID_AA64PFR0_FEATURES(fphp, AARCH64_FP, AARCH64_FPHP)
ID_AA64PFR0_FEATURES(asimd, AARCH64_ASIMD)
ID_AA64PFR0_FEATURES(asimdhp, AARCH64_ASIMD, AARCH64_ASIMDHP)
ID_AA64PFR0_FEATURES(dit, AARCH64_DIT)
ID_AA64PFR0_FEATURES(sve, AARCH64_SVE)

static MrsFeature id_aa64pfr0[] = {
    ID_AA64PFR0_SET_FEATURE(fp, "FP"),
    ID_AA64PFR0_SET_FEATURE(fphp, "FP+HP"),
    ID_AA64PFR0_SET_FEATURE(asimd, "AdvSIMD"),
    ID_AA64PFR0_SET_FEATURE(asimdhp, "AdvSIMD+HP"),
    ID_AA64PFR0_SET_FEATURE(dit, "PSTATE.DIT"),
    ID_AA64PFR0_SET_FEATURE(sve, "SVE"),
    MRS_FEATURE_END,
};

// ID_AA64PFR1_EL1
// -----------------------------------------------------------------------------
ID_AA64PFR1_FEATURES(ssbs, AARCH64_SSBS)
// TODO: Add ssbs2 detection
ID_AA64PFR1_FEATURES(ssbs2, AARCH64_SSBS)
ID_AA64PFR1_FEATURES(mte, AARCH64_MTE)
ID_AA64PFR1_FEATURES(bti, AARCH64_BTI)

static MrsFeature id_aa64pfr1[] = {
    ID_AA64PFR1_SET_FEATURE(ssbs, "PSTATE.SSBS"),
    ID_AA64PFR1_SET_FEATURE(ssbs2, "PSTATE.SSBS MSR"),
    ID_AA64PFR1_SET_FEATURE(mte, "MTE"),
    ID_AA64PFR1_SET_FEATURE(bti, "BTI"),
    MRS_FEATURE_END,
};

// ID_AA64ZFR0_EL1
// -----------------------------------------------------------------------------
ID_AA64ZFR0_FEATURES(svef64mm, AARCH64_SVEF64MM)
ID_AA64ZFR0_FEATURES(svef32mm, AARCH64_SVEF32MM)
ID_AA64ZFR0_FEATURES(svei8mm, AARCH64_SVEI8MM)
ID_AA64ZFR0_FEATURES(svesm4, AARCH64_SVESM4)
ID_AA64ZFR0_FEATURES(svesha3, AARCH64_SVESHA3)
ID_AA64ZFR0_FEATURES(svebf16, AARCH64_SVEBF16)
// TODO: Add sveebf16 detection
ID_AA64ZFR0_FEATURES(sveebf16, AARCH64_SVEBF16)
ID_AA64ZFR0_FEATURES(svebitper, AARCH64_SVEBITPERM)
ID_AA64ZFR0_FEATURES(sveaes, AARCH64_SVEAES)
ID_AA64ZFR0_FEATURES(sve_aes_pmull, AARCH64_SVEAES, AARCH64_SVEPMULL)
ID_AA64ZFR0_FEATURES(sve2, AARCH64_SVE2)

static MrsFeature id_aa64zfr0[] = {
    ID_AA64ZFR0_SET_FEATURE(svef64mm, "F64MM"),
    ID_AA64ZFR0_SET_FEATURE(svef32mm, "F32MM"),
    ID_AA64ZFR0_SET_FEATURE(svei8mm, "I8MM"),
    ID_AA64ZFR0_SET_FEATURE(svesm4, "SM4"),
    ID_AA64ZFR0_SET_FEATURE(svesha3, "SHA3"),
    ID_AA64ZFR0_SET_FEATURE(svebf16, "BF16"),
    ID_AA64ZFR0_SET_FEATURE(sveebf16, "BF16+EBF"),
    ID_AA64ZFR0_SET_FEATURE(svebitper, "BitPerm"),
    ID_AA64ZFR0_SET_FEATURE(sveaes, "AES"),
    ID_AA64ZFR0_SET_FEATURE(sve_aes_pmull, "AES+PMULL"),
    ID_AA64ZFR0_SET_FEATURE(sve2, "SVE2"),
    MRS_FEATURE_END,
};

static const MrsField mrs_fields[] = {
    MRS_FIELD(ID_AA64ISAR0_KEY, id_aa64isar0),
    MRS_FIELD(ID_AA64ISAR1_KEY, id_aa64isar1),
    MRS_FIELD(ID_AA64ISAR2_KEY, id_aa64isar2),
    MRS_FIELD(ID_AA64PFR0_KEY, id_aa64pfr0),
    MRS_FIELD(ID_AA64PFR1_KEY, id_aa64pfr1),
    MRS_FIELD(ID_AA64ZFR0_KEY, id_aa64zfr0),
    MRS_FIELD_END,
};

static bool HasFeature(StringView csv, const char* feature) {
  return CpuFeatures_StringView_HasWord(csv, feature, ',');
}

static bool DetectAarch64Features(StringView csv, MrsFeature* mrs_features,
                                  Aarch64Features* features) {
  for (int i = 0; mrs_features[i].name != NULL; ++i) {
    MrsFeature mrs_feature = mrs_features[i];
    if (!HasFeature(csv, mrs_feature.name)) continue;
    for (int j = 0; mrs_feature.features_enum[j] != AARCH64_FEAT_END; ++j) {
      int feat_idx = mrs_feature.features_enum[j];
      kSetters[feat_idx](features, true);
    }
  }
}

static bool HandleAarch64Line(const LineResult result,
                              Aarch64Features* features) {
  const StringView line = result.line;
  for (int i = 0; mrs_fields[i].key != NULL; ++i) {
    MrsField mrs_field = mrs_fields[i];
    if (!CpuFeatures_StringView_StartsWith(line, str(mrs_field.key))) continue;
    StringView csv = result.line;
    int index = CpuFeatures_StringView_IndexOfChar(csv, '<');
    if (index >= 0) {
      csv = CpuFeatures_StringView_PopFront(csv, index + 1);
    }
    if (csv.size > 0 && CpuFeatures_StringView_Back(csv) == '>') {
      csv = CpuFeatures_StringView_PopBack(csv, 1);
    }
    DetectAarch64Features(csv, mrs_field.features, features);
  }
  return !result.eof;
}

static void DetectFeaturesFromOs(Aarch64Features* features) {
  // Handling FreeBSD platform through parsing /var/run/dmesg.boot.
  const int fd = CpuFeatures_OpenFile("/var/run/dmesg.boot");
  if (fd >= 0) {
    StackLineReader reader;
    StackLineReader_Initialize(&reader, fd);
    for (;;) {
      if (!HandleAarch64Line(StackLineReader_NextLine(&reader), features))
        break;
    }
    CpuFeatures_CloseFile(fd);
  }
}

Aarch64Info GetAarch64Info(void) {
  Aarch64Info info = kEmptyAarch64Info;
  DetectFeaturesFromOs(&info.features);
  return info;
}

#endif  // CPU_FEATURES_OS_FREEBSD
#endif  // CPU_FEATURES_ARCH_AARCH64
