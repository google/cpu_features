# cpu_features, a cross platform C89 library to get cpu features at runtime.
package(
    default_copts = [
        "-DDISABLE_GOOGLE_GLOBAL_USING_DECLARATIONS",
        "-Wno-implicit-fallthrough",
        "-Ithird_party/cpu_features/include",
    ],
    default_visibility = ["//visibility:public"],
    features = [
        "-parse_headers",  # disabled because tests (C++) depends on C target compiled with -std=gnu89.
        "-layering_check",  # disabled because it depends on parse_headers.
    ],
)

licenses(["notice"])

# MOE:begin_strip
filegroup(
    name = "opensource_filegroup",
    srcs = [
        ".clang-format",
        ".travis.yml",
        "BUILD",
        "CMakeLists.txt",
        "CMakeLists.txt.in",
        "CONTRIBUTING.md",
        "LICENSE",
        "OWNERS",
        "README.md",
        "WORKSPACE",
        "appveyor.yml",
        "include/cpu_features_macros.h",
        "include/cpuinfo_aarch64.h",
        "include/cpuinfo_arm.h",
        "include/cpuinfo_mips.h",
        "include/cpuinfo_x86.h",
        "include/internal/bit_utils.h",
        "include/internal/cpuid_x86.h",
        "include/internal/filesystem.h",
        "include/internal/hwcaps.h",
        "include/internal/linux_features_aggregator.h",
        "include/internal/stack_line_reader.h",
        "include/internal/string_view.h",
        "src/cpuid_x86_clang.c",
        "src/cpuid_x86_gcc.c",
        "src/cpuid_x86_msvc.c",
        "src/cpuinfo_aarch64.c",
        "src/cpuinfo_arm.c",
        "src/cpuinfo_mips.c",
        "src/cpuinfo_x86.c",
        "src/filesystem.c",
        "src/hwcaps.c",
        "src/linux_features_aggregator.c",
        "src/list_cpu_features.cc",
        "src/stack_line_reader.c",
        "src/string_view.c",
        "test/CMakeLists.txt",
        "test/bit_utils_test.cc",
        "test/cpuinfo_aarch64_test.cc",
        "test/cpuinfo_arm_test.cc",
        "test/cpuinfo_mips_test.cc",
        "test/cpuinfo_x86_test.cc",
        "test/filesystem_for_testing.cc",
        "test/filesystem_for_testing.h",
        "test/hwcaps_for_testing.cc",
        "test/hwcaps_for_testing.h",
        "test/linux_features_aggregator_test.cc",
        "test/stack_line_reader_test.cc",
        "test/string_view_test.cc",
    ],
    visibility = ["//third_party/cpu_features:__subpackages__"],
)

# MOE:end_strip

exports_files(["LICENSE"])

vardef(
    "GNU89_FLAGS",
    "-std=gnu89 " +
    "-Wall " +
    "-Wdeclaration-after-statement " +
    "-Wextra " +
    "-Wmissing-declarations " +
    "-Wmissing-prototypes " +
    "-Wold-style-definition " +
    "-Wshadow " +
    "-Wsign-compare " +
    "-Wstrict-prototypes ",
)

cc_library(
    name = "cpu_features_macros",
    srcs = ["include/cpu_features_macros.h"],
    copts = [varref("GNU89_FLAGS")],
)

cc_library(
    name = "bit_utils",
    srcs = ["include/internal/bit_utils.h"],
    copts = [varref("GNU89_FLAGS")],
    deps = [":cpu_features_macros"],
)

cc_test(
    name = "bit_utils_test",
    srcs = ["test/bit_utils_test.cc"],
    deps = [
        ":bit_utils",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "string_view",
    srcs = ["src/string_view.c"],
    hdrs = ["include/internal/string_view.h"],
    copts = [varref("GNU89_FLAGS")],
    deps = [":cpu_features_macros"],
)

cc_test(
    name = "string_view_test",
    srcs = ["test/string_view_test.cc"],
    deps = [
        ":string_view",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "filesystem",
    srcs = [
        "include/internal/filesystem.h",
        "src/filesystem.c",
    ],
    copts = [
        varref("GNU89_FLAGS"),
    ],
    deps = [":cpu_features_macros"],
)

cc_library(
    name = "filesystem_for_testing",
    testonly = 1,
    srcs = [
        "include/internal/filesystem.h",
        "test/filesystem_for_testing.cc",
        "test/filesystem_for_testing.h",
    ],
    deps = [
        ":cpu_features_macros",
        "//base",
    ],
)

cc_library(
    name = "stack_line_reader",
    srcs = [
        "include/internal/stack_line_reader.h",
        "src/stack_line_reader.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    defines = ["STACK_LINE_READER_BUFFER_SIZE=1024"],
    deps = [
        ":cpu_features_macros",
        ":filesystem",
        ":string_view",
    ],
)

cc_library(
    name = "stack_line_reader_for_testing",
    testonly = 1,
    srcs = [
        "include/internal/stack_line_reader.h",
        "src/stack_line_reader.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    defines = ["STACK_LINE_READER_BUFFER_SIZE=1024"],
    deps = [
        ":cpu_features_macros",
        ":filesystem_for_testing",
        ":string_view",
    ],
)

cc_test(
    name = "stack_line_reader_test",
    srcs = [
        "include/internal/stack_line_reader.h",
        "src/stack_line_reader.c",
        "test/stack_line_reader_test.cc",
    ],
    defines = ["STACK_LINE_READER_BUFFER_SIZE=16"],
    deps = [
        ":cpu_features_macros",
        ":filesystem_for_testing",
        ":string_view",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "hwcaps",
    srcs = [
        "include/internal/hwcaps.h",
        "src/hwcaps.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    deps = [
        ":cpu_features_macros",
        ":filesystem",
    ],
)

cc_library(
    name = "hwcaps_for_testing",
    testonly = 1,
    srcs = ["test/hwcaps_for_testing.cc"],
    hdrs = [
        "include/internal/hwcaps.h",
        "test/hwcaps_for_testing.h",
    ],
    deps = [":cpu_features_macros"],
)

cc_library(
    name = "linux_features_aggregator",
    srcs = [
        "include/internal/linux_features_aggregator.h",
        "src/linux_features_aggregator.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    deps = [
        ":hwcaps",
        ":string_view",
    ],
)

cc_test(
    name = "linux_features_aggregator_test",
    srcs = ["test/linux_features_aggregator_test.cc"],
    deps = [
        ":linux_features_aggregator",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "cpuinfo_mips",
    srcs = [
        "include/cpuinfo_mips.h",
        "src/cpuinfo_mips.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    deps = [
        ":linux_features_aggregator",
        ":stack_line_reader",
        ":string_view",
    ],
)

cc_test(
    name = "cpuinfo_mips_test",
    srcs = [
        "include/cpuinfo_mips.h",
        "src/cpuinfo_mips.c",
        "test/cpuinfo_mips_test.cc",
    ],
    deps = [
        ":filesystem_for_testing",
        ":hwcaps_for_testing",
        ":linux_features_aggregator",
        ":stack_line_reader",
        ":string_view",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "cpuinfo_aarch64",
    srcs = [
        "include/cpuinfo_aarch64.h",
        "src/cpuinfo_aarch64.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    deps = [
        ":linux_features_aggregator",
        ":stack_line_reader",
        ":string_view",
    ],
)

cc_test(
    name = "cpuinfo_aarch64_test",
    srcs = [
        "include/cpuinfo_aarch64.h",
        "src/cpuinfo_aarch64.c",
        "test/cpuinfo_aarch64_test.cc",
    ],
    deps = [
        ":filesystem_for_testing",
        ":hwcaps_for_testing",
        ":linux_features_aggregator",
        ":stack_line_reader",
        ":string_view",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "cpuinfo_arm",
    srcs = [
        "include/cpuinfo_arm.h",
        "src/cpuinfo_arm.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    deps = [
        ":bit_utils",
        ":linux_features_aggregator",
        ":stack_line_reader",
        ":string_view",
    ],
)

cc_test(
    name = "cpuinfo_arm_test",
    srcs = [
        "include/cpuinfo_arm.h",
        "src/cpuinfo_arm.c",
        "test/cpuinfo_arm_test.cc",
    ],
    deps = [
        ":bit_utils",
        ":filesystem_for_testing",
        ":hwcaps_for_testing",
        ":linux_features_aggregator",
        ":stack_line_reader",
        ":string_view",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "cpuid_x86",
    srcs = [
        "include/internal/cpuid_x86.h",
        "src/cpuid_x86_clang.c",
        "src/cpuid_x86_gcc.c",
        "src/cpuid_x86_msvc.c",
    ],
    copts = [varref("GNU89_FLAGS")],
    deps = [":cpu_features_macros"],
)

cc_library(
    name = "cpuinfo_x86",
    srcs = ["src/cpuinfo_x86.c"],
    hdrs = ["include/cpuinfo_x86.h"],
    copts = [varref("GNU89_FLAGS")],
    deps = [
        ":bit_utils",
        ":cpu_features_macros",
        ":cpuid_x86",
    ],
)

cc_test(
    name = "cpuinfo_x86_test",
    srcs = [
        "include/cpuinfo_x86.h",
        "src/cpuinfo_x86.c",
        "test/cpuinfo_x86_test.cc",
    ],
    defines = ["CPU_FEATURES_TEST"],
    deps = [
        ":bit_utils",
        ":cpu_features_macros",
        ":cpuid_x86",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_binary(
    name = "list_cpu_features",
    srcs = ["src/list_cpu_features.cc"],
    deps = [
        ":cpu_features_macros",
        ":cpuinfo_aarch64",
        ":cpuinfo_arm",
        ":cpuinfo_mips",
        ":cpuinfo_x86",
    ],
)
