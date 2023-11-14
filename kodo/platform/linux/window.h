#pragma once

#include <functional>

#include "kodo/platform/iwindow.h"

struct _XDisplay;
typedef struct _XDisplay Display;

namespace Steinberg {
namespace Vst {
namespace EditorHost {

class X11Window : public IWindow {
 public:
  using Ptr = std::shared_ptr<X11Window>;
  using WindowClosedFunc = std::function<void(X11Window*)>;

  static Ptr make(const std::string& name, Size size, bool resizeable,
                  const WindowControllerPtr& controller, Display* display,
                  const WindowClosedFunc& windowClosedFunc);

  X11Window();
  ~X11Window() override;

  bool init(const std::string& name, Size size, bool resizeable,
            const WindowControllerPtr& controller, Display* display,
            const WindowClosedFunc& windowClosedFunc);

  void show() override;
  void close() override;
  void resize(Size newSize) override;
  Size getContentSize() override;

  NativePlatformWindow getNativePlatformWindow() const override;
  tresult queryInterface(const TUID iid, void** obj) override;

  void onIdle();

 private:
  Size getSize() const;

  struct Impl;
  std::unique_ptr<Impl> impl;
};

}  // namespace EditorHost
}  // namespace Vst
}  // namespace Steinberg
