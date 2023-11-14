#pragma once

#include <memory>
#include <string>

#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/funknown.h"

namespace kodo {

using Coord = int32_t;

struct Point {
  Coord x;
  Coord y;
};

struct Size {
  Coord width;
  Coord height;
  auto operator<=>(const Size&) const = default;
};

struct Rect {
  Point origin;
  Size size;
};

struct NativePlatformWindow {
  std::string type;
  void* ptr;
};

class IWindow;

class IWindowController {
 public:
  virtual ~IWindowController() noexcept = default;

  virtual void onShow(IWindow& window) = 0;
  virtual void onClose(IWindow& window) = 0;
  virtual void onResize(IWindow& window, Size newSize) = 0;
  virtual void onContentScaleFactorChanged(IWindow& window,
                                           float newScaleFactor) = 0;
  virtual Size constrainSize(IWindow& window, Size requestedSize) = 0;
};

using WindowControllerPtr = std::shared_ptr<IWindowController>;

class IWindow {
 public:
  virtual ~IWindow() noexcept = default;

  virtual void show() = 0;
  virtual void close() = 0;
  virtual void resize(Size newSize) = 0;
  virtual Size getContentSize() = 0;

  virtual NativePlatformWindow getNativePlatformWindow() const = 0;

  virtual Steinberg::tresult queryInterface(const Steinberg::TUID iid,
                                            void** obj) = 0;
};

using WindowPtr = std::shared_ptr<IWindow>;

}  // namespace kodo
