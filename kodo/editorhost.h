#pragma once

#include "kodo/platform/iapplication.h"
#include "kodo/platform/iwindow.h"
#include "public.sdk/source/vst/hosting/hostclasses.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"
#include "public.sdk/source/vst/utility/optional.h"

namespace kodo {

class WindowController;

class App : public kodo::IApplication {
 public:
  ~App() noexcept override;
  void init(const std::vector<std::string>& cmdArgs) override;
  void terminate() override;

 private:
  enum OpenFlags {
    kSetComponentHandler = 1 << 0,
    kSecondWindow = 1 << 1,
  };
  void openEditor(const std::string& path, VST3::Optional<VST3::UID> effectID,
                  uint32_t flags);
  void createViewAndShow(Steinberg::Vst::IEditController* controller);

  VST3::Hosting::Module::Ptr module{nullptr};
  Steinberg::IPtr<Steinberg::Vst::PlugProvider> plugProvider{nullptr};
  Steinberg::Vst::HostApplication pluginContext;
  std::shared_ptr<kodo::IWindow> window;
  std::shared_ptr<WindowController> windowController;
};

}  // namespace kodo
