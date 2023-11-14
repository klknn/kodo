#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "kodo/editorhost.h"
#include "kodo/platform/iapplication.h"
#include "kodo/platform/iplatform.h"
#include "kodo/platform/linux/runloop.h"
#include "kodo/platform/linux/window.h"

using namespace kodo;

namespace Steinberg {
namespace Vst {
namespace EditorHost {
namespace {

class Platform : public IPlatform {
 public:
  static Platform& instance() {
    static Platform gInstance;
    return gInstance;
  }

  void setApplication(std::unique_ptr<kodo::IApplication>&& app) override {
    application = std::move(app);
  }

  WindowPtr createWindow(const std::string& title, Size size, bool resizeable,
                         const WindowControllerPtr& controller) override {
    auto window =
        X11Window::make(title, size, resizeable, controller, xDisplay,
                        [this](X11Window* window) { onWindowClosed(window); });
    if (window) windows.push_back(std::static_pointer_cast<X11Window>(window));
    return window;
  }

  void quit() override {
    static bool recursiveGuard = false;
    if (recursiveGuard) return;
    recursiveGuard = true;

    closeAllWindows();

    if (application) application->terminate();

    RunLoop::instance().stop();

    recursiveGuard = false;
  }

  void kill(int resultCode, const std::string& reason) override {
    std::cout << reason << "\n";
    exit(resultCode);
  }

  void run(const std::vector<std::string>& cmdArgs) {
    // Connect to X server
    std::string displayName(getenv("DISPLAY"));
    if (displayName.empty()) displayName = ":0.0";

    if ((xDisplay = XOpenDisplay(displayName.data())) == nullptr) {
      return;
    }

    RunLoop::instance().setDisplay(xDisplay);

    application->init(cmdArgs);

    RunLoop::instance().start();

    XCloseDisplay(xDisplay);
  }

 private:
  void onWindowClosed(X11Window* window) {
    for (auto it = windows.begin(); it != windows.end(); ++it) {
      if (it->get() == window) {
        windows.erase(it);
        break;
      }
    }
  }

  void closeAllWindows() {
    for (auto& w : windows) {
      w->close();
    }
  }

  std::unique_ptr<IApplication> application;
  Display* xDisplay{nullptr};
  std::vector<std::shared_ptr<X11Window>> windows;
};

}  // namespace

IPlatform& IPlatform::instance() { return Platform::instance(); }

}  // namespace EditorHost
}  // namespace Vst
}  // namespace Steinberg

int main(int argc, char* argv[]) {
  std::vector<std::string> cmdArgs;
  for (int i = 1; i < argc; ++i) cmdArgs.push_back(argv[i]);

  Steinberg::Vst::EditorHost::Platform::instance().setApplication(
      std::make_unique<Steinberg::Vst::EditorHost::App>());

  Steinberg::Vst::EditorHost::Platform::instance().run(cmdArgs);

  return 0;
}
