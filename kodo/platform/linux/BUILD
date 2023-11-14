load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library", "cc_library")

cc_library(
    name = "window",
    hdrs = ["window.h"],
    srcs = ["window.cc"],
    deps = [
        ":runloop",
        "//kodo/platform:iwindow",
        "@vst3sdk//:public_sdk",
        "@linux_system_libs//:x11",
    ]
)

cc_library(
    name = "runloop",
    hdrs = ["runloop.h"],
    srcs = ["runloop.cc"],
)

cc_binary(
    name = "linux",
    srcs = ["platform.cc"],
    deps = [
        ":window",
        ":runloop",
        "//kodo:editorhost",
        "//kodo/platform:iplatform",
    ],
)
