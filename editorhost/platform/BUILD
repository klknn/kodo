load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//editorhost:__subpackages__"])

cc_library(
    name = "appinit",
    hdrs = ["appinit.h"],
    deps = [
        ":iapplication",
        ":iplatform",
    ],
)

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
)
