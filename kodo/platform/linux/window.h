#pragma once

#include <functional>

#include "kodo/platform/iwindow.h"
#include "pluginterfaces/base/ftypes.h"
#include "pluginterfaces/base/funknown.h"

struct _XDisplay;
typedef struct _XDisplay Display;

namespace kodo {

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
  Steinberg::tresult queryInterface(const Steinberg::TUID iid,
                                    void** obj) override;

  void onIdle();

 private:
  Size getSize() const;

  struct Impl;
  std::unique_ptr<Impl> impl;
};

}  // namespace kodo
