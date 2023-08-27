#include "plugin_vst3.h"

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>

#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"

using ::VST3::Hosting::Module;

namespace kodo {

// https://www.utsbox.com/?p=3006
absl::StatusOr<std::shared_ptr<Module>> LoadVst3Plugin(std::string_view path) {
  std::string error;
  std::shared_ptr<Module> module =
      VST3::Hosting::Module::create(std::string{path}, error);
  if (!module) {
    return absl::NotFoundError(absl::StrCat(error, " in file ", path));
  }

  auto factory = module->getFactory();
  std::unique_ptr<Steinberg::Vst::PlugProvider> provider;
  for (auto& classInfo : factory.classInfos()) {
    provider = std::make_unique<Steinberg::Vst::PlugProvider>(factory,
                                                              classInfo, true);
    if (!provider->initialize()) {
      return absl::FailedPreconditionError(
          absl::StrCat("PlugProvider::initialize() failed for ", path));
    }
    break;
  }
  if (!provider) {
    return absl::NotFoundError(
        absl::StrCat("No VST3 Audio Module Class found in file ", path));
  }

  Steinberg::Vst::IEditController* controller = provider->getController();
  if (!controller) {
    return absl::NotFoundError(absl::StrCat(
        "No EditController found (needed for allowing editor) in file ", path));
  }
  controller->release();  // plugProvider does an addRef

  return module;
}

}  // namespace kodo
