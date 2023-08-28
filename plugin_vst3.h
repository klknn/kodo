#pragma once

#include <memory>
#include <string_view>

#include "absl/status/statusor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/hosting/module.h"

namespace kodo {

// Tests plugin for debugging.
absl::StatusOr<VST3::Hosting::Module::Ptr> LoadVst3Plugin(
    std::string_view path);

using ControllerDel = void (*)(Steinberg::Vst::IEditController*);
using ControllerPtr =
    std::unique_ptr<Steinberg::Vst::IEditController, ControllerDel>;

absl::StatusOr<ControllerPtr> GetEditController(
    VST3::Hosting::Module::Ptr module);

absl::Status OpenEditor(Steinberg::Vst::IEditController& controller,
                        void* handle);

}  // namespace kodo
