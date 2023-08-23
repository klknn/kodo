# kodoh

[![ubuntu](https://github.com/klknn/kodoh/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/klknn/kodoh/actions/workflows/ubuntu.yml)

## Setup

1. Install bazelisk prebuilt binary as bazel
   https://github.com/bazelbuild/bazelisk/releases
2. If you use debian-like OS, GLFW requires X11 deps `apt install xorg-dev`.
   See https://www.glfw.org/docs/3.3/compile.html
3. Run this command to launch GUI: `bazel run :main`

## History

- 2023/08/23: Init.
