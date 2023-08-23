#include <filesystem>
#include <memory>

#include "absl/flags/declare.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/log/initialize.h"
#include "absl/log/log.h"
#include "gui.h"
#include "kodoh.pb.h"

ABSL_DECLARE_FLAG(int, stderrthreshold);  // To override in main().

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

  kodoh::Project project;
  project.set_name("untitled");
  project.set_author("unknown");
  LOG(INFO) << "Project proto\n" << project.Utf8DebugString();

  // Launch GUI.
  std::unique_ptr<kodoh::Gui> gui = kodoh::Gui::Init();
  while (!gui->Close()) {
    gui->Render();
  }
}
