#include <algorithm>
#include <filesystem>
#include <string>

#include "absl/cleanup/cleanup.h"
#include "absl/flags/declare.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "gui.h"
#include "kodo.pb.h"
#include "plugin_vst3.h"
#include "portaudio.h"

ABSL_FLAG(bool, gui, true, "Will launch GUI.");
ABSL_FLAG(std::string, test_vst3, "", "Test the given VST3 on launch.");
ABSL_DECLARE_FLAG(int, stderrthreshold);  // To override in main().

// From https://github.com/PortAudio/portaudio/blob/master/examples/pa_devs.c
absl::Status ListAudioDevices() {
  LOG(INFO) << "PortAudio version: " << Pa_GetVersion();
  LOG(INFO) << "Version text: " << Pa_GetVersionInfo()->versionText;

  PaDeviceIndex n_devices = Pa_GetDeviceCount();
  if (n_devices < 0) {
    LOG(ERROR) << Pa_GetErrorText(n_devices);
    return absl::FailedPreconditionError("Pa_GetDeviceCount failed.");
  }
  LOG(INFO) << "Number of devices: " << n_devices;

  for (int i = 0; i < n_devices; ++i) {
    if (i == Pa_GetDefaultInputDevice()) {
      LOG(INFO) << "Device: " << i << " (default input)";
    } else if (i == Pa_GetDefaultOutputDevice()) {
      LOG(INFO) << "Device: " << i << " (default output)";
    } else {
      LOG(INFO) << "Device: " << i;
    }
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
    LOG(INFO) << absl::StrFormat("Name                        = %s",
                                 deviceInfo->name);
    LOG(INFO) << absl::StrFormat("Host API                    = %s",
                                 Pa_GetHostApiInfo(deviceInfo->hostApi)->name);
    LOG(INFO) << absl::StrFormat("Max inputs = %d",
                                 deviceInfo->maxInputChannels);
    LOG(INFO) << absl::StrFormat(", Max outputs = %d",
                                 deviceInfo->maxOutputChannels);

    LOG(INFO) << absl::StrFormat("Default low input latency   = %8.4f",
                                 deviceInfo->defaultLowInputLatency);
    LOG(INFO) << absl::StrFormat("Default low output latency  = %8.4f",
                                 deviceInfo->defaultLowOutputLatency);
    LOG(INFO) << absl::StrFormat("Default high input latency  = %8.4f",
                                 deviceInfo->defaultHighInputLatency);
    LOG(INFO) << absl::StrFormat("Default high output latency = %8.4f",
                                 deviceInfo->defaultHighOutputLatency);
  }
  return absl::OkStatus();
}

int main(int argc, char** argv) {
  // Process command line flags. https://abseil.io/docs/cpp/guides/flags.
  absl::SetFlag(&FLAGS_stderrthreshold, 0);
  absl::ParseCommandLine(argc, argv);

  // Log startup info.
  absl::InitializeLog();
  LOG(INFO) << "Current path:" << std::filesystem::current_path();
  for (int i = 0; i < argc; ++i) {
    LOG(INFO) << "arg[" << i << "]: " << argv[i];
  }

  kodo::Project project;
  project.set_name("<untitled>");
  project.set_author("<unknown>");
  LOG(INFO) << "Project proto\n" << project.Utf8DebugString();

  PaError err = Pa_Initialize();
  absl::Cleanup pa_cleanup = []() { Pa_Terminate(); };
  if (err != paNoError) {
    LOG(ERROR) << "Pa_Initialize failed: " << Pa_GetErrorText(err);
    return err;
  }

  if (absl::Status status = ListAudioDevices(); !status.ok()) {
    LOG(ERROR) << status;
  }

  kodo::ControllerPtr controller(nullptr, nullptr);
  auto vst3_module = kodo::LoadVst3Plugin(absl::GetFlag(FLAGS_test_vst3));
  if (vst3_module.ok()) {
    auto controller_or = kodo::GetEditController(*vst3_module);
    if (controller_or.ok()) {
      controller = std::move(*controller_or);
    } else {
      LOG(ERROR) << controller_or.status();
    }
  } else {
    LOG(ERROR) << vst3_module.status();
  }

  if (!absl::GetFlag(FLAGS_gui)) {
    LOG(INFO) << "Skip GUI by --gui=false.";
    return 0;
  }

  // Launch GUI.
  std::unique_ptr<kodo::Gui> gui = kodo::Gui::Init();
  while (!gui->Close()) {
    gui->Begin();

    ImGui::NewFrame();
    if (controller) {
      kodo::OpenEditor(*controller, gui->GetHandle());
    }
    gui->RenderCore();
    ImGui::Render();
    gui->End();
  }
}
