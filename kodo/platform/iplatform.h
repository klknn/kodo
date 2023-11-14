#pragma once

#include <functional>
#include <memory>

#include "kodo/platform/iapplication.h"
#include "kodo/platform/iwindow.h"

namespace kodo {

class IPlatform {
 public:
  virtual ~IPlatform() noexcept = default;

  virtual void setApplication(std::unique_ptr<IApplication>&& app) = 0;

  virtual std::shared_ptr<IWindow> createWindow(
      const std::string& title, Size size, bool resizeable,
      const WindowControllerPtr& controller) = 0;
  virtual void quit() = 0;
  virtual void kill(int resultCode, const std::string& reason) = 0;

  static IPlatform& instance();
};

}  // namespace kodo
