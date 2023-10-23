#include <memory>

#include "window.h"

namespace kodo {

namespace {

class LinuxWindow : public Window {
 public:
  LinuxWindow() {}
};

}  // namespace

absl::StatusOr<std::unique_ptr<Window>> InitWindow(int width, int height) {
  return std::make_unique<LinuxWindow>();
}

}  // namespace kodo
