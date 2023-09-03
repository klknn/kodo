#include "plugin_vst3.h"

#include <stdint.h>

#include <algorithm>
#include <iomanip>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>

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

struct FRelease {
  void operator()(Steinberg::FUnknown* p) { p->release(); }
};

template <class T>
concept FChild = std::is_base_of_v<Steinberg::FUnknown, T>;

template <FChild T>
using FUniquePtr = std::unique_ptr<T, FRelease>;

absl::StatusCode TResultToStatus(Steinberg::tresult code) {
  switch (code) {
    case Steinberg::kNoInterface:
      return absl::StatusCode::kNotFound;
    case Steinberg::kResultOk:
      return absl::StatusCode::kOk;
    case Steinberg::kResultFalse:
      return absl::StatusCode::kUnknown;
    case Steinberg::kInvalidArgument:
      return absl::StatusCode::kInvalidArgument;
    case Steinberg::kNotImplemented:
      return absl::StatusCode::kUnimplemented;
    case Steinberg::kInternalError:
      return absl::StatusCode::kInternal;
    case Steinberg::kNotInitialized:
      return absl::StatusCode::kFailedPrecondition;
    case Steinberg::kOutOfMemory:
      return absl::StatusCode::kResourceExhausted;
    default:
      return absl::StatusCode::kUnknown;
  }
}

template <FChild T>
absl::StatusOr<FUniquePtr<T>> Query(Steinberg::FUnknown* p) {
  T* ret;
  Steinberg::tresult res = p->queryInterface(T::iid, &ret);
  if (res == Steinberg::kResultOk) {
    return FUnknownPtr<T>(ret);
  }
  return absl::Status(TResultToStatus(res), "Failed to queryInterface.");
}

class ImPlugFrame : public Steinberg::IPlugFrame {
 public:
  explicit ImPlugFrame(const Steinberg::IPtr<Steinberg::IPlugView>& plug_view)
      : plug_view_(plug_view) {}

  absl::Status Render(void* handle) {
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    // absl::Cleanup pop_style = []() { ImGui::PopStyleVar(); };

    Steinberg::ViewRect curr_rect;
    plug_view_->getSize(&curr_rect);
    ImVec2 curr_size;
    curr_size.x = curr_rect.getWidth();
    curr_size.y = curr_rect.getHeight();
    ImGui::SetNextWindowContentSize(curr_size);
    // ImVec2 curr_pos;
    // curr_pos.x = curr_rect.left;
    // curr_pos.y = curr_rect.top;
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

    if (absl::Status status = Attach(handle); !status.ok()) {
      return status;
    }

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
#if defined _WIN32
    // Win32.
    if (plug_view_->isPlatformTypeSupported(Steinberg::kPlatformTypeHWND)) {
      LOG_FIRST_N(INFO, 1) << "Try HWND platform";
      if (plug_view_->attached(handle, Steinberg::kPlatformTypeHWND) !=
          Steinberg::kResultOk) {
        return absl::InvalidArgumentError(
            "cannot call attached(handle, HWND).");
      }
      return absl::OkStatus();
    }
#elif defined __APPLE__
    // OSX Cocoa.
    if (plug_view_->isPlatformTypeSupported(Steinberg::kPlatformTypeNSView) ==
        Steinberg::kResultOk) {
      LOG_FIRST_N(INFO, 1) << "NSView platform";
      if (plug_view_->attached(contentView(handle),
                               Steinberg::kPlatformTypeNSView) !=
          Steinberg::kResultOk) {
        return absl::InvalidArgumentError(
            "cannot call attached(handle, NSView).");
      }
      return absl::OkStatus();
    }
    if (plug_view_->isPlatformTypeSupported(Steinberg::kPlatformTypeHIView) ==
        Steinberg::kResultOk) {
      LOG_FIRST_N(INFO, 1) << "HIView platform";
      if (plug_view_->attached(handle, Steinberg::kPlatformTypeHIView) !=
          Steinberg::kResultOk) {
        return absl::InvalidArgumentError(
            "cannot call attached(handle, HIView).");
      }
      return absl::OkStatus();
    }
#elif defined __linux__
    // Linux X11.
    if (plug_view_->isPlatformTypeSupported(
            Steinberg::kPlatformTypeX11EmbedWindowID) == Steinberg::kResultOk) {
      LOG_FIRST_N(INFO, 1) << "X11 platform";
      if (plug_view_->attached(handle,
                               Steinberg::kPlatformTypeX11EmbedWindowID) !=
          Steinberg::kResultOk) {
        return absl::InvalidArgumentError(
            "cannot call attached(handle, X11EmbedWindow).");
      }
      return absl::OkStatus();
    }
#else
#error "Unknown platform."
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

// ImRect ViewRectToImRect(Steinberg::ViewRect r) {
//   ImRect result{};
//   result.Min.x = r.left;
//   result.Min.y = r.top;
//   result.Max.x = r.right;
//   result.Max.y = r.bottom;
//   return result;
// }

class Vst3Plugin : public Plugin {
 public:
  static absl::StatusOr<std::unique_ptr<Vst3Plugin>> Init(
      VST3::Hosting::Module::Ptr module, int index) {
    std::unique_ptr<Vst3Plugin> ret(new Vst3Plugin);
    ret->module_ = module;

    VST3::Hosting::PluginFactory factory = module->getFactory();
    LOG(INFO) << "Found #class=" << factory.classCount();
    if (factory.classCount() <= index) {
      return absl::NotFoundError(absl::StrCat(
          "index=", index,
          " is out-of-range to class count=", factory.classCount()));
    }

    ret->factory_ = factory.get();

    // if (auto f = Query<Steinberg::IPluginFactory3>(factory.get()); f.ok()) {
    //   f.value()->getClassInfoUnicode(index, )
    // }

    ret->class_info_ = factory.classInfos()[index];
    LOG(INFO) << "Class: " << ret->class_info_.name();
    Steinberg::IPtr<Steinberg::Vst::PlugProvider> provider = Steinberg::owned(
        new Steinberg::Vst::PlugProvider(factory, ret->class_info_, true));
    if (!provider) {
      return absl::NotFoundError("Could not create plugin provider.");
    }
    if (!provider->initialize()) {
      return absl::FailedPreconditionError("PlugProvider::initialize() failed");
    }
    Steinberg::Vst::IEditController* controller = provider->getController();
    if (!controller) {
      return absl::NotFoundError(
          "No EditController found (needed for allowing editor)");
    }
    LOG(INFO) << "#Params=" << controller->getParameterCount();
    ret->controller_ = controller;

    return ret;
  }

  ~Vst3Plugin() override { controller_->release(); }

  absl::Status Render(void* window_handle) override {
    Steinberg::IPtr<Steinberg::IPlugView> view = Steinberg::owned(
        controller_->createView(Steinberg::Vst::ViewType::kEditor));
    if (!view) {
      return absl::FailedPreconditionError("Could not create window.");
    }
    ImPlugFrame frame(view);
    return frame.Render(window_handle);
  }

 private:
  Vst3Plugin() {}

  Steinberg::IPtr<Steinberg::IPluginFactory> factory_;
  VST3::Hosting::ClassInfo class_info_;
  Steinberg::Vst::IEditController* controller_;
  VST3::Hosting::Module::Ptr module_;  // Not to exceed lifetime beyond module.
};

class Vst3PluginModule : public PluginModule {
 public:
  static absl::StatusOr<std::unique_ptr<Vst3PluginModule>> Init(
      std::string_view path) {
    std::unique_ptr<Vst3PluginModule> ret(new Vst3PluginModule);

    std::string error;
    ret->module_ = VST3::Hosting::Module::create(std::string{path}, error);
    if (ret->module_ == nullptr) {
      return absl::NotFoundError(absl::StrCat(error, " in file ", path));
    }

    LOG(INFO) << "Loaded module: " << std::quoted(ret->module_->getName())
              << " from path " << path;
    return ret;
  }

  absl::StatusOr<std::unique_ptr<Plugin>> Load(int index) override {
    return Vst3Plugin::Init(module_, index);
  }

 private:
  Vst3PluginModule() {}  // Use Init().

  VST3::Hosting::Module::Ptr module_;
};

}  // namespace

absl::StatusOr<std::unique_ptr<PluginModule>> Vst3Module(
    std::string_view path) {
  return Vst3PluginModule::Init(path);
}

}  // namespace kodo
