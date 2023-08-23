#include <memory>

#include "absl/flags/declare.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"

#include "gui.h"

ABSL_DECLARE_FLAG(int, stderrthreshold);  // To override in main().

int main(int argc, char** argv) {
  // Process command line flags. https://abseil.io/docs/cpp/guides/flags.
  absl::SetFlag(&FLAGS_stderrthreshold, 0);
  absl::ParseCommandLine(argc, argv);
  absl::InitializeLog();
  LOG(INFO) << "Start: " << argv[0];
  for (int i = 1; i < argc; ++i) {
    LOG(INFO) << "  option: " << argv[i];
  }

  // Launch GUI.
  std::unique_ptr<kodoh::Gui> gui = kodoh::Gui::Init();
  while (!gui->Close()) {
    gui->Render();
  }
}
