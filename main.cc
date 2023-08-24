#include <filesystem>
#include <memory>

#include "absl/cleanup/cleanup.h"
#include "absl/flags/declare.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/check.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "gui.h"
#include "kodo.pb.h"
#include "portaudio.h"

ABSL_FLAG(bool, gui, true, "Will launch GUI.");
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
    printf("Name                        = %s\n", deviceInfo->name);
    printf("Host API                    = %s\n",
           Pa_GetHostApiInfo(deviceInfo->hostApi)->name);
    printf("Max inputs = %d", deviceInfo->maxInputChannels);
    printf(", Max outputs = %d\n", deviceInfo->maxOutputChannels);

    printf("Default low input latency   = %8.4f\n",
           deviceInfo->defaultLowInputLatency);
    printf("Default low output latency  = %8.4f\n",
           deviceInfo->defaultLowOutputLatency);
    printf("Default high input latency  = %8.4f\n",
           deviceInfo->defaultHighInputLatency);
    printf("Default high output latency = %8.4f\n",
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
  project.set_name("untitled");
  project.set_author("unknown");
  LOG(INFO) << "Project proto\n" << project.Utf8DebugString();

  PaError err = Pa_Initialize();
  absl::Cleanup pa_cleanup = []() { Pa_Terminate(); };
  if (err != paNoError) {
    LOG(ERROR) << "Pa_Initialize failed: " << Pa_GetErrorText(err);
    return err;
  }
  absl::Status status = ListAudioDevices();
  LOG_IF(ERROR, !status.ok()) << status;

  if (!absl::GetFlag(FLAGS_gui)) {
    LOG(INFO) << "Skip GUI by --gui=false.";
    return 0;
  }

  // Launch GUI.
  std::unique_ptr<kodo::Gui> gui = kodo::Gui::Init();
  while (!gui->Close()) {
    gui->Render();
  }
}
