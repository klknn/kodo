#pragma once

#include <memory>

#include "imgui.h"
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>

namespace kodo {

class Gui {
 public:
  // Initializes GUI resources and libs.
  static std::unique_ptr<Gui> Init();

  // Finizelizes GUI resources and libs.
  virtual ~Gui();

  // Checks if GUI is closed during event loop.
  bool Close();

  void Begin();

  void End();

  // Render only using ImGui core funcs without backends.
  void RenderCore();

  void* GetHandle();

 private:
  Gui() {}  // Use Init() with error handing.

  // Our state
  bool show_demo_window_ = true;
  bool show_another_window_ = false;
  ImVec4 clear_color_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  GLFWwindow* window_;
  void* plugin_parent_window_;
};

}  // namespace kodo
