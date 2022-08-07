# cpu_features

A cross-platform C library to retrieve CPU features (such as available
instructions) at runtime.

# GitHub-CI Status

[comment]: <> (The following lines are generated by "scripts/generate_badges.d" that you can run online https://run.dlang.io/)

| Os | amd64 | AArch64 | ARM | MIPS | POWER | RISCV | s390x |
| :-- | --: | --: | --: | --: | --: | --: | --: |
| Linux | [![][i1a0]][l1a0]<br/>[![][i1a1]][l1a1] | [![][i1b0]][l1b0]<br/>![][d1] | [![][i1c0]][l1c0]<br/>![][d1] | [![][i1d0]][l1d0]<br/>![][d1] | [![][i1e0]][l1e0]<br/>![][d1] | [![][i1f0]][l1f0]<br/>![][d1] | [![][i1g0]][l1g0]<br/>![][d1] |
| FreeBSD | [![][i2a0]][l2a0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] |
| MacOS | [![][i3a0]][l3a0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] |
| Windows | [![][i4a0]][l4a0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] | ![][d0]<br/>![][d1] |

[d0]: https://img.shields.io/badge/CMake-N%2FA-lightgrey
[d1]: https://img.shields.io/badge/Bazel-N%2FA-lightgrey
[i1a0]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20Linux%20CMake/main?label=CMake
[i1a1]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20Linux%20Bazel/main?label=Bazel
[i1b0]: https://img.shields.io/github/workflow/status/google/cpu_features/AArch64%20Linux%20CMake/main?label=CMake
[i1c0]: https://img.shields.io/github/workflow/status/google/cpu_features/ARM%20Linux%20CMake/main?label=CMake
[i1d0]: https://img.shields.io/github/workflow/status/google/cpu_features/MIPS%20Linux%20CMake/main?label=CMake
[i1e0]: https://img.shields.io/github/workflow/status/google/cpu_features/POWER%20Linux%20CMake/main?label=CMake
[i1f0]: https://img.shields.io/github/workflow/status/google/cpu_features/RISCV%20Linux%20CMake/main?label=CMake
[i1g0]: https://img.shields.io/github/workflow/status/google/cpu_features/s390x%20Linux%20CMake/main?label=CMake
[i2a0]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20FreeBSD%20CMake/main?label=CMake
[i3a0]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20MacOS%20CMake/main?label=CMake
[i4a0]: https://img.shields.io/github/workflow/status/google/cpu_features/amd64%20Windows%20CMake/main?label=CMake
[l1a0]: https://github.com/google/cpu_features/actions/workflows/amd64_linux_cmake.yml
[l1a1]: https://github.com/google/cpu_features/actions/workflows/amd64_linux_bazel.yml
[l1b0]: https://github.com/google/cpu_features/actions/workflows/aarch64_linux_cmake.yml
[l1c0]: https://github.com/google/cpu_features/actions/workflows/arm_linux_cmake.yml
[l1d0]: https://github.com/google/cpu_features/actions/workflows/mips_linux_cmake.yml
[l1e0]: https://github.com/google/cpu_features/actions/workflows/power_linux_cmake.yml
[l1f0]: https://github.com/google/cpu_features/actions/workflows/riscv_linux_cmake.yml
[l1g0]: https://github.com/google/cpu_features/actions/workflows/s390x_linux_cmake.yml
[l2a0]: https://github.com/google/cpu_features/actions/workflows/amd64_freebsd_cmake.yml
[l3a0]: https://github.com/google/cpu_features/actions/workflows/amd64_macos_cmake.yml
[l4a0]: https://github.com/google/cpu_features/actions/workflows/amd64_windows_cmake.yml

## Table of Contents

- [Design Rationale](#rationale)
- [Code samples](#codesample)
- [Running sample code](#usagesample)
- [What's supported](#support)
- [Android NDK's drop in replacement](#ndk)
- [License](#license)
- [Build with cmake](#cmake)
- [Community Bindings](#bindings)

<a name="rationale"></a>
## Design Rationale

-   **Simple to use.** See the snippets below for examples.
-   **Extensible.** Easy to add missing features or architectures.
-   **Compatible with old compilers** and available on many architectures so it
    can be used widely. To ensure that cpu_features works on as many platforms
    as possible, we implemented it in a highly portable version of C: C99.
-   **Sandbox-compatible.** The library uses a variety of strategies to cope
    with sandboxed environments or when `cpuid` is unavailable. This is useful
    when running integration tests in hermetic environments.
-   **Thread safe, no memory allocation, and raises no exceptions.**
    cpu_features is suitable for implementing fundamental libc functions like
    `malloc`, `memcpy`, and `memcmp`.
-   **Unit tested.**

<a name="codesample"></a>
## Code samples

**Note:** For C++ code, the library functions are defined in the `cpu_features` namespace.

### Checking features at runtime

Here's a simple example that executes a codepath if the CPU supports both the
AES and the SSE4.2 instruction sets:

```c
#include "cpuinfo_x86.h"

// For C++, add `using namespace cpu_features;`
static const X86Features features = GetX86Info().features;

void Compute(void) {
  if (features.aes && features.sse4_2) {
    // Run optimized code.
  } else {
    // Run standard code.
  }
}
```

### Caching for faster evaluation of complex checks

If you wish, you can read all the features at once into a global variable, and
then query for the specific features you care about. Below, we store all the ARM
features and then check whether AES and NEON are supported.

```c
#include <stdbool.h>
#include "cpuinfo_arm.h"

// For C++, add `using namespace cpu_features;`
static const ArmFeatures features = GetArmInfo().features;
static const bool has_aes_and_neon = features.aes && features.neon;

// use has_aes_and_neon.
```

This is a good approach to take if you're checking for combinations of features
when using a compiler that is slow to extract individual bits from bit-packed
structures.

### Checking compile time flags

The following code determines whether the compiler was told to use the AVX
instruction set (e.g., `g++ -mavx`) and sets `has_avx` accordingly.

```c
#include <stdbool.h>
#include "cpuinfo_x86.h"

// For C++, add `using namespace cpu_features;`
static const X86Features features = GetX86Info().features;
static const bool has_avx = CPU_FEATURES_COMPILED_X86_AVX || features.avx;

// use has_avx.
```

`CPU_FEATURES_COMPILED_X86_AVX` is set to 1 if the compiler was instructed to
use AVX and 0 otherwise, combining compile time and runtime knowledge.

### Rejecting poor hardware implementations based on microarchitecture

On x86, the first incarnation of a feature in a microarchitecture might not be
the most efficient (e.g. AVX on Sandy Bridge). We provide a function to retrieve
the underlying microarchitecture so you can decide whether to use it.

Below, `has_fast_avx` is set to 1 if the CPU supports the AVX instruction
set&mdash;but only if it's not Sandy Bridge.

```c
#include <stdbool.h>
#include "cpuinfo_x86.h"

// For C++, add `using namespace cpu_features;`
static const X86Info info = GetX86Info();
static const X86Microarchitecture uarch = GetX86Microarchitecture(&info);
static const bool has_fast_avx = info.features.avx && uarch != INTEL_SNB;

// use has_fast_avx.
```

This feature is currently available only for x86 microarchitectures.

<a name="usagesample"></a>
### Running sample code

Building `cpu_features` (check [quickstart](#quickstart) below) brings a small executable to test the library.

```shell
 % ./build/list_cpu_features
arch            : x86
brand           :        Intel(R) Xeon(R) CPU E5-1650 0 @ 3.20GHz
family          :   6 (0x06)
model           :  45 (0x2D)
stepping        :   7 (0x07)
uarch           : INTEL_SNB
flags           : aes,avx,cx16,smx,sse4_1,sse4_2,ssse3
```

```shell
% ./build/list_cpu_features --json
{"arch":"x86","brand":"       Intel(R) Xeon(R) CPU E5-1650 0 @ 3.20GHz","family":6,"model":45,"stepping":7,"uarch":"INTEL_SNB","flags":["aes","avx","cx16","smx","sse4_1","sse4_2","ssse3"]}
```

<a name="support"></a>
## What's supported

|         | x86³ | AArch64 | ARM     | MIPS⁴   | s390x   | POWER   | RISC-V  |
|---------|:----:|:-------:|:-------:|:-------:|:-------:|:-------:|:-------:|
| Linux   | yes² | yes¹    | yes¹    | yes¹    | yes¹    | yes¹    | yes¹    |
| FreeBSD | yes² | not yet | not yet | not yet | not yet | not yet | N/A     |
| MacOs   | yes² | not yet | N/A     | N/A     | no      | no      | N/A     |
| Windows | yes² | not yet | not yet | N/A     | N/A     | N/A     | N/A     |
| Android | yes² | yes¹    | yes¹    | yes¹    | N/A     | N/A     | N/A     |
| iOS     | N/A  | not yet | not yet | N/A     | N/A     | N/A     | N/A     |

1.  **Features revealed from Linux.** We gather data from several sources
    depending on availability:
    +   from glibc's
        [getauxval](https://www.gnu.org/software/libc/manual/html_node/Auxiliary-Vector.html)
    +   by parsing `/proc/self/auxv`
    +   by parsing `/proc/cpuinfo`
2.  **Features revealed from CPU.** features are retrieved by using the `cpuid`
    instruction.
3.  **Microarchitecture detection.** On x86 some features are not always
    implemented efficiently in hardware (e.g. AVX on Sandybridge). Exposing the
    microarchitecture allows the client to reject particular microarchitectures.
4.  All flavors of Mips are supported, little and big endian as well as 32/64
    bits.

<a name="ndk"></a>
## Android NDK's drop in replacement

[cpu_features](https://github.com/google/cpu_features) is now officially
supporting Android and offers a drop in replacement of for the NDK's [cpu-features.h](https://android.googlesource.com/platform/ndk/+/main/sources/android/cpufeatures/cpu-features.h)
, see [ndk_compat](ndk_compat) folder for details.

<a name="license"></a>
## License

The cpu_features library is licensed under the terms of the Apache license.
See [LICENSE](LICENSE) for more information.

<a name="cmake"></a>
## Build with CMake

Please check the [CMake build instructions](cmake/README.md).

<a name="quickstart"></a>
### Quickstart

- Run `list_cpu_features`
  ```sh
  cmake -S. -Bbuild -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release
  cmake --build build --config Release -j
  ./build/list_cpu_features --json
  ```

  _Note_: Use `--target ALL_BUILD` on the second line for `Visual Studio` and `XCode`.

- run tests
  ```sh
  cmake -S. -Bbuild -DBUILD_TESTING=ON -DCMAKE_BUILD_TYPE=Debug
  cmake --build build --config Debug -j
  cmake --build build --config Debug --target test
  ```

  _Note_: Use `--target RUN_TESTS` on the last line for `Visual Studio` and `--target RUN_TEST` for `XCode`.


- install `cpu_features`
  ```sh
  cmake --build build --config Release --target install -v
  ```

  _Note_: Use `--target INSTALL` for `Visual Studio`.

  _Note_: When using `Makefile` or `XCode` generator, you can use
  [`DESTDIR`](https://www.gnu.org/software/make/manual/html_node/DESTDIR.html)
  to install on a local repository.<br>
  e.g.
  ```sh
  cmake --build build --config Release --target install -v -- DESTDIR=install
  ```

<a name="bindings"></a>
## Community bindings

Links provided here are not affiliated with Google but are kindly provided by the OSS Community.

 - .Net
   - https://github.com/toor1245/cpu_features.NET
 - Python
   - https://github.com/Narasimha1997/py_cpu


_Send PR to showcase your wrapper here_
