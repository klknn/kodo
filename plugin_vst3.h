#pragma once

#include <memory>
#include <string_view>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace kodo {

class Plugin {
 public:
  virtual ~Plugin() {}
  virtual absl::Status Render(void* window_handle) = 0;
};

class PluginModule {
 public:
  virtual ~PluginModule() {}
  virtual absl::StatusOr<std::unique_ptr<Plugin>> Load(int index) = 0;
};

absl::StatusOr<std::unique_ptr<PluginModule>> Vst3Module(std::string_view path);

}  // namespace kodo
