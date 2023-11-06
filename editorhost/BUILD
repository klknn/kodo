load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = [":__subpackages__"])

cc_library(
    name = "editorhost",
    hdrs = ["editorhost.h"],
    srcs = ["editorhost.cpp"],
    deps = [
        "//editorhost/platform:appinit",
        "//editorhost/platform:iapplication",
        "//editorhost/platform:iwindow",
        "@vst3sdk//:base",
        "@vst3sdk//:pluginterfaces",
        "@vst3sdk//:public_sdk",
    ],
)
