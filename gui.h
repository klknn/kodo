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

  // Wraps RenderCore with pre/post processings in backends.
  void Render();

 private:
  Gui() {}  // Use Init() with error handing.

  // Render only using ImGui core funcs without backends.
  void RenderCore();

  // Our state
  bool show_demo_window_ = true;
  bool show_another_window_ = false;
  ImVec4 clear_color_ = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  GLFWwindow* window_;
};

}  // namespace kodo
