#pragma once

#include <functional>
#include <memory>

#include "kodo/platform/iapplication.h"
#include "kodo/platform/iwindow.h"

//------------------------------------------------------------------------
namespace Steinberg {
namespace Vst {
namespace EditorHost {

//------------------------------------------------------------------------
class IPlatform {
 public:
  virtual ~IPlatform() noexcept = default;

  virtual void setApplication(std::unique_ptr<kodo::IApplication>&& app) = 0;

  virtual WindowPtr createWindow(const std::string& title, Size size,
                                 bool resizeable,
                                 const WindowControllerPtr& controller) = 0;
  virtual void quit() = 0;
  virtual void kill(int resultCode, const std::string& reason) = 0;

  static IPlatform& instance();
};

//------------------------------------------------------------------------
}  // namespace EditorHost
}  // namespace Vst
}  // namespace Steinberg
