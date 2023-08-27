#include <string_view>

#include "absl/status/statusor.h"
#include "public.sdk/source/vst/hosting/module.h"

namespace kodo {

// Tests plugin for debugging.
absl::StatusOr<VST3::Hosting::Module::Ptr> LoadVst3Plugin(
    std::string_view path);

}  // namespace kodo
