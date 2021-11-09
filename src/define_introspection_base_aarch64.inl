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
  LINE(AARCH64_MTE, mte, , , )
#define INTROSPECTION_PREFIX Aarch64
#define INTROSPECTION_ENUM_PREFIX AARCH64
#include "define_introspection.inl"
