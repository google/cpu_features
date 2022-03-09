workspace(name = "com_google_cpufeatures")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_googletest",
    sha256 = "1cff5915c9dfbf8241d811e95230833c4f34a6d56b7b8c960f4c828f60429a38",
    strip_prefix = "googletest-c9461a9b55ba954df0489bab6420eb297bed846b",
    urls = ["https://github.com/google/googletest/archive/c9461a9b55ba954df0489bab6420eb297bed846b.zip"],
)

http_archive(
    name = "bazel_skylib",
    sha256 = "c6966ec828da198c5d9adbaa94c05e3a1c7f21bd012a0b29ba8ddbccb2c93b0d",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.1.1/bazel-skylib-1.1.1.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.1.1/bazel-skylib-1.1.1.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()
