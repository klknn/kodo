#pragma once

#include <memory>
#include <string>

#include "absl/status/status.h"
#include "absl/status/statusor.h"

namespace kodo {

struct NativeWindow {
  std::string type;
  void* ptr = nullptr;
};

class Window {
 public:
  virtual ~Window() {}

  virtual void Close() {}

  virtual absl::Status Show() { return absl::UnimplementedError(""); }

  virtual absl::Status Resize(int width, int height) {
    return absl::UnimplementedError("");
  };

  virtual absl::StatusOr<NativeWindow> Native() {
    return absl::UnimplementedError("");
  }
};

absl::StatusOr<std::unique_ptr<Window>> InitWindow(int width, int height);

}  // namespace kodo
