load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_proto_library", "cc_library")
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

proto_library(
    name = "kodo_proto",
    srcs = ["kodo.proto"],
)

cc_proto_library(
    name = "kodo_cc_proto",
    deps = [":kodo_proto"],
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
        "@imguizmo//:ImCurveEdit",
        "@imguizmo//:ImSequencer",
    ],
)

cc_library(
    name = "plugin_vst3",
    hdrs = ["plugin_vst3.h"],
    srcs = ["plugin_vst3.cc"],
    deps = [
        ":gui",
        "@imgui//:core",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/log:log",
        "@vst3sdk//:pluginterfaces",
        "@vst3sdk//:public_sdk",
    ],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    # TODO(klknn): Test this in linux.
    # features = ["fully_static_link"],
    deps = [
        ":gui",
        ":plugin_vst3",
        ":kodo_cc_proto",
        "@com_google_absl//absl/cleanup",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/log:log",
        "@com_google_absl//absl/log:flags",
        "@com_google_absl//absl/log:initialize",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings:str_format",
        "@portaudio//:portaudio",
    ],
)

refresh_compile_commands(
    name = "compdb",
)
