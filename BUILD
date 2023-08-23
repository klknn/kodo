cc_library(
    name = "gui",
    hdrs = ["gui.h"],
    srcs = ["gui.cc"],
    deps = [
        "@absl//absl/log",
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
        "@absl//absl/flags:parse",
        "@absl//absl/log",
        "@absl//absl/log:flags",
        "@absl//absl/log:initialize",
    ],
)
