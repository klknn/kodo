load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")

proto_library(
    name = "kodoh_proto",
    srcs = ["kodoh.proto"],
)

cc_proto_library(
    name = "kodoh_cc_proto",
    deps = [":kodoh_proto"],
)

cc_library(
    name = "gui",
    hdrs = ["gui.h"],
    srcs = ["gui.cc"],
    deps = [
        "@com_google_absl//absl/log:log",
        "@imgui//:core",
        "@imgui//:backends_glfw",
        "@imgui//:backends_opengl3",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [
        ":gui",
        ":kodoh_cc_proto",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:log",
        "@com_google_absl//absl/log:flags",
        "@com_google_absl//absl/log:initialize",
    ],
)
