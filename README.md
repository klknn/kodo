# kodo

[![ubuntu](https://github.com/klknn/kodo/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/klknn/kodo/actions/workflows/ubuntu.yml)
[![windows](https://github.com/klknn/kodo/actions/workflows/windows.yml/badge.svg)](https://github.com/klknn/kodo/actions/workflows/windows.yml)
[![macos](https://github.com/klknn/kodo/actions/workflows/macos.yml/badge.svg)](https://github.com/klknn/kodo/actions/workflows/macos.yml)

Experimental digital audio workstation software.

## Setup

1. Install bazelisk prebuilt binary as bazel
   https://github.com/bazelbuild/bazelisk/releases
2. If you use debian-like OS:
   - GLFW requires X11 deps `apt install xorg-dev`.
     See https://www.glfw.org/docs/3.3/compile.html
   - Portaudio requires ALSA `apt install libasound-dev`
     See https://portaudio.com/docs/v19-doxydocs/compile_linux.html
   - See also [CI config](.github/workflows/ubuntu..yml) for details.
3. Run this command to launch it: `bazel run //:main --config={OS}`, where OS can be one of windows, macos, and linux.

## Roadmap

- [x] Create proto to save/load DAW projects.
- [x] Lookup audio devices.
- [ ] Create minimal GUI of sequencer, mixier, and piano roll.
- [ ] Load audio files and play audio.
- [ ] Record audio.
- [ ] Host VST3 synth plugins and play midi.
- [ ] Lookup midi devices.
- [ ] Record midi.
- [ ] Host VST3 effect plugins in the mixier.

## References

- Portaudio tutorial https://portaudio.com/docs/v19-doxydocs/tutorial_start.html
- RtMidi tutorial http://www.music.mcgill.ca/~gary/rtmidi/
- Imgui useful extensions https://github.com/ocornut/imgui/wiki/Useful-Extensions
- VST3 SDK doc https://steinbergmedia.github.io/vst3_doc/
- Minimal vst3 host app https://github.com/hotwatermorning/Terra/
  - Author's notes in Japanese https://hwm.hatenablog.com/entry/2014/12/01/233320
- Open exchange format for DAW projects https://github.com/bitwig/dawproject
- Proto2 language guide https://protobuf.dev/programming-guides/proto2/

## History

- 2023/08/23: Init.

## License

Copyright klknn since 2023.
Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt
