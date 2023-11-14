load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//kodo:__subpackages__"])

cc_library(
    name = "iapplication",
    hdrs = ["iapplication.h"],
)

cc_library(
    name = "iplatform",
    hdrs = ["iplatform.h"],
    deps = [
        ":iapplication",
        ":iwindow",
    ]
)

cc_library(
    name = "iwindow",
    hdrs = ["iwindow.h"],
    deps = [
        "@vst3sdk//:pluginterfaces"
    ]
)
