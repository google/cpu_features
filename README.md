# cpu features

A cross platform C89 library to get cpu features at runtime.

### Checking features at runtime.

```c
#include "cpuinfo_x86.h"

static const X86Features features = GetX86Info().features;

void Compute(void) {
  if(features.aes && features.sse4_2) {
    // Run optimized code.
  } else {
    // Run standard code.
  }
}
```

### Caching for faster evaluation of complex checks.

Features are bit packed and some compilers generate poor code when combining
them. A simple solution is to read them once and store the result in a global
variable.

```c
#include "cpuinfo_arm.h"

static const ArmFeatures features = GetArmInfo().features;
static const bool has_aes_and_neon = features.aes && features.neon;

// use has_aes_and_neon.
```

### Checking compile time flags as well.

```c
#include "cpuinfo_x86.h"

static const X86Features features = GetX86Info().features;
static const bool has_avx = CPU_FEATURES_COMPILED_X86_AVX || features.avx;

// use has_avx.
```

`CPU_FEATURES_COMPILED_X86_AVX` is set to 1 if the compiler is instructed to use
AVX, 0 otherwise. This allows combining compile time and runtime knowledge.

### Use x86 microarchitecture to reject poor hardware implementations.

```c
#include "cpuinfo_x86.h"

static const X86Info info = GetX86Info();
static const X86Microarchitecture uarch = GetX86Microarchitecture(&info);
static const bool has_fast_avx = info.features.avx && uarch != INTEL_SNB;

// use has_fast_avx.
```

On x86, the first incarnation of a feature in a microarchitecture may not be
very efficient (e.g. AVX on Sandybridge). We provide a function to retrieve the
underlying microarchitecture so clients can decide whether they want to use it
or not.

## What does it currently support

|                             | x86 | ARM | aarch64 |  mips  |  POWER  |
|---------------------------- | :-: | :-: | :-----: | :----: | :-----: |
|Features From cpu            | yes | no* | no*     | no yet | not yet |
|Features From Linux          | no  | yes | yes     | yes    | not yet |
|Micro Architecture Detection | yes | no  | no      | no     | not yet |
|Windows support              | yes | no  | no      | no     | not yet |

-   **Features From Cpuid**: features are retrieved by using the cpuid
    instruction. (*) Unfortunately this instruction is privileged for some
    architectures; in this case we fall back to Linux.
-   **Features From Linux**: we gather data from several sources depending on
    what's available:
    +   from glibc's
        [getauxval](https://www.gnu.org/software/libc/manual/html_node/Auxiliary-Vector.html)
    +   by parsing `/proc/self/auxv`
    +   by parsing `/proc/cpuinfo`
-   **Micro Architecture Detection**: On x86 some features are not always
    implemented efficiently in hardware (e.g. AVX on Sandybridge). Exposing the
    microarchitecture allows the client to reject some microarchitectures.

## Design Rationale

-   Simple to use, API should be straightforward from the snippets above.
-   Unit tested.
-   Extensible. It should be easy to add missing features or architectures.
-   Compatible with old compilers and available on many architectures so it can
    be used widely. We target gnu89.
-   Works in sandboxed environment: some architectures rely on parsing files
    that may not be available in a sandboxed environment. It is useful when
    running integration tests in hermetic environments.
-   Thread safe, no allocation, no exception: suitable for implementation of
    fundamental libc functions like malloc, memcpy, memcmp...
