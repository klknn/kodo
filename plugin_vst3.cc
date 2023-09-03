#include "plugin_vst3.h"

#include <stdint.h>

#include <iomanip>
#include <memory>
#include <string>
#include <string_view>

#include "absl/cleanup/cleanup.h"
#include "absl/log/log.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "imgui_internal.h"
#include "pluginterfaces/gui/iplugview.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "public.sdk/source/vst/hosting/module.h"
#include "public.sdk/source/vst/hosting/plugprovider.h"

using ::VST3::Hosting::Module;

#ifdef __APPLE__

#include <objc/objc.h>

extern "C" {
// Sends a message with a simple return value to an instance of a class.
// https://developer.apple.com/documentation/objectivec/1456712-objc_msgsend/
id objc_msgSend(id, SEL, ...);
}

// The window’s content view, the highest accessible view object in the window’s
// view hierarchy.
// https://developer.apple.com/documentation/appkit/nswindow/1419160-contentview?language=objc
static void* contentView(void* nswindow) {
  SEL sel = sel_registerName("contentView:");
  return (void*)objc_msgSend((id)nswindow, sel);
}

#endif

namespace kodo {

namespace {

class ImPlugFrame : public Steinberg::IPlugFrame {
 public:
  explicit ImPlugFrame(const Steinberg::IPtr<Steinberg::IPlugView>& plug_view)
      : plug_view_(plug_view) {}

  absl::Status Render(void* handle) {
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    // absl::Cleanup pop_style = []() { ImGui::PopStyleVar(); };

    Steinberg::ViewRect curr_rect;
    plug_view_->getSize(&curr_rect);
    ImVec2 curr_size, curr_pos;
    curr_size.x = curr_rect.getWidth();
    curr_size.y = curr_rect.getHeight();
    // curr_pos.x = curr_rect.left;
    // curr_pos.y = curr_rect.top;

    ImGui::SetNextWindowContentSize(curr_size);
    // ImGui::SetNextWindowPos(curr_pos, 0, {0.5, 0.5});
    ImGui::Begin("PlugFrame");

    Steinberg::ViewRect new_rect;
    new_rect.left = ImGui::GetCursorPosX();
    new_rect.top = ImGui::GetCursorPosY();
    new_rect.right = ImGui::GetContentRegionMax().x;
    new_rect.bottom = ImGui::GetContentRegionMax().y;

    // LOG(INFO) << "Cursor X=" << ImGui::GetCursorPosX()
    //           << " Y=" << ImGui::GetCursorPosY();
    if (plug_view_->onSize(&new_rect) != Steinberg::kResultOk) {
      return absl::InvalidArgumentError("cannot call onSize(new_rect).");
    }
    if (plug_view_->setFrame(this) != Steinberg::kResultOk) {
      return absl::InvalidArgumentError("cannot call setFrame(this).");
    }

    Attach(handle);

    // ImVec2 min = ImGui::GetCursorPos();
    // ImVec2 max = ImGui::GetContentRegionMax();
    // // ImVec2 region_avail = ImGui::GetContentRegionAvail();
    // Steinberg::ViewRect new_rect;
    // new_rect.left = min.x;
    // new_rect.right = max.x;
    // new_rect.top = min.x;
    // new_rect.bottom = max.y;

    // resizeView(plug_view_, &new_rect);
    // plug_view_->onSize(&new_rect);

    ImGui::End();
    return absl::OkStatus();
  }

  // Called to inform the host about the resize of a given view.
  // Afterwards the host has to call IPlugView::onSize().
  Steinberg::tresult PLUGIN_API resizeView(
      Steinberg::IPlugView* view, Steinberg::ViewRect* newSize) override {
    if (newSize == nullptr || view == nullptr || view != plug_view_)
      return Steinberg::kInvalidArgument;

    using Steinberg::ViewRect;
    Steinberg::ViewRect r;
    if (plug_view_->getSize(&r) != Steinberg::kResultTrue)
      return Steinberg::kInternalError;
    // if (r == *newSize) return Steinberg::kResultTrue;

    // TODO
    return Steinberg::kResultOk;
  }

 private:
  // Attaches the plug-in editor with platform-specific window handle.
  absl::Status Attach(void* handle) {
    // Based on
    // https://github.com/hotwatermorning/Vst3HostDemo/blob/22ab87c3cf19992d7ef852e24b5900762c94c7f9/Vst3HostDemo/plugin/vst3/Vst3PluginImpl.cpp#L414
#ifdef _WIN32
    if (plug_view_->attached(handle, Steinberg::kPlatformTypeHWND) !=
        Steinberg::kResultOk) {
      return absl::InvalidArgumentError("cannot call attached(handle, HWND).");
    }
#elif defined __APPLE__
    if (plug_view_->isPlatformTypeSupported(Steinberg::kPlatformTypeNSView) ==
        Steinberg::kResultOk) {
      if (plug_view_->attached(contentView(handle),
                               Steinberg::kPlatformTypeNSView) !=
          Steinberg::kResultOk) {
        return absl::InvalidArgumentError(
            "cannot call attached(handle, NSView).");
      }
    }
    if (plug_view_->isPlatformTypeSupported(Steinberg::kPlatformTypeHIView) ==
        Steinberg::kResultOk) {
      if (plug_view_->attached(handle, Steinberg::kPlatformTypeHIView) !=
          Steinberg::kResultOk) {
        return absl::InvalidArgumentError(
            "cannot call attached(handle, HIView).");
      }
    }
#else
    // #error "Unknown platform."
#endif
    return absl::UnimplementedError("Unsupported platform.");
  }

  Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID _iid,
                                               void** obj) override {
    if (Steinberg::FUnknownPrivate::iidEqual(_iid,
                                             Steinberg::IPlugFrame::iid) ||
        Steinberg::FUnknownPrivate::iidEqual(_iid, Steinberg::FUnknown::iid)) {
      *obj = this;
      addRef();
      return Steinberg::kResultTrue;
    }
    return Steinberg::kNoInterface;
  }

  // we do not care here of the ref-counting. A plug-in call of release should
  // not destroy this class!
  uint32_t PLUGIN_API addRef() override { return 1000; }
  uint32_t PLUGIN_API release() override { return 1000; }

  Steinberg::IPtr<Steinberg::IPlugView> plug_view_;
};

ImRect ViewRectToImRect(Steinberg::ViewRect r) {
  ImRect result{};
  result.Min.x = r.left;
  result.Min.y = r.top;
  result.Max.x = r.right;
  result.Max.y = r.bottom;
  return result;
}

absl::Status ShowEditor(Steinberg::Vst::IEditController& contoller) {
  Steinberg::IPtr<Steinberg::IPlugView> view =
      Steinberg::owned(contoller.createView(Steinberg::Vst::ViewType::kEditor));
  if (!view) {
    return absl::FailedPreconditionError("Could not create window.");
  }

  Steinberg::ViewRect size;
  if (view->getSize(&size) != Steinberg::kResultOk) {
    return absl::FailedPreconditionError("Could not get view size.");
  }
  ImRect rect = ViewRectToImRect(size);

  ImPlugFrame frame(view);
  view->setFrame(&frame);

  return absl::OkStatus();
}

void ReleaseController(Steinberg::Vst::IEditController* p) { p->release(); }

}  // namespace

absl::Status OpenEditor(Steinberg::Vst::IEditController& controller,
                        void* handle) {
  Steinberg::IPtr<Steinberg::IPlugView> view = Steinberg::owned(
      controller.createView(Steinberg::Vst::ViewType::kEditor));
  if (!view) {
    return absl::FailedPreconditionError("Could not create window.");
  }

  ImPlugFrame frame(view);
  frame.Render(handle);
}

absl::StatusOr<ControllerPtr> GetEditController(
    VST3::Hosting::Module::Ptr module) {
  auto factory = module->getFactory();
  LOG(INFO) << "Found #class=" << factory.classCount();

  Steinberg::IPtr<Steinberg::Vst::PlugProvider> provider;
  for (auto& classInfo : factory.classInfos()) {
    LOG(INFO) << "Class: " << classInfo.name();
    provider = Steinberg::owned(
        new Steinberg::Vst::PlugProvider(factory, classInfo, true));
    if (!provider->initialize()) {
      return absl::FailedPreconditionError("PlugProvider::initialize() failed");
    }
    break;
  }
  if (!provider) {
    return absl::NotFoundError("No VST3 Audio Module Class found");
  }

  Steinberg::Vst::IEditController* controller = provider->getController();
  if (!controller) {
    return absl::NotFoundError(
        "No EditController found (needed for allowing editor)");
  }
  LOG(INFO) << "#Params=" << controller->getParameterCount();

  return ControllerPtr(controller, &ReleaseController);
}

// https://www.utsbox.com/?p=3006
absl::StatusOr<std::shared_ptr<Module>> LoadVst3Plugin(std::string_view path) {
  std::string error;
  std::shared_ptr<Module> module =
      VST3::Hosting::Module::create(std::string{path}, error);
  if (!module) {
    return absl::NotFoundError(absl::StrCat(error, " in file ", path));
  }

  LOG(INFO) << "Loaded module: " << std::quoted(module->getName())
            << " from path " << path;
  // if (absl::Status status = ShowEditor(*controller); !status.ok()) {
  //   return status;
  // }
  return module;
}

}  // namespace kodo
