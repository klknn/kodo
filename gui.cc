// Dear ImGui: standalone example application for GLFW + OpenGL 3, using
// programmable pipeline (GLFW is a cross-platform general purpose library for
// handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation,
// etc.) If you are new to Dear ImGui, read documentation from the docs/ folder
// + read the top of imgui.cpp. Read online:
// https://github.com/ocornut/imgui/tree/master/docs
#include "gui.h"

#include <stdio.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <vector>

#include "ImCurveEdit.h"
#include "ImSequencer.h"
#include "absl/log/log.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>  // Will drag system OpenGL headers

namespace kodo {

namespace {

void glfw_error_callback(int error, const char* description) {
  LOG(ERROR) << "GLFW Error " << error << ": " << description;
}

}  // namespace

std::unique_ptr<Gui> Gui::Init() {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit()) return nullptr;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char* glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

  // Create window with graphics context
  GLFWwindow* window = glfwCreateWindow(
      1280, 720, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
  if (window == nullptr) return nullptr;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype
  // for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at
  // runtime from the "fonts/" folder. See Makefile.emscripten for details.
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

  std::unique_ptr<Gui> ret(new Gui);
  ret->window_ = window;
  return ret;
}

Gui::~Gui() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window_);
  glfwTerminate();
}

bool Gui::Close() { return glfwWindowShouldClose(window_); }

void Gui::Render() {
  // Poll and handle events (inputs, window resize, etc.)
  // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
  // tell if dear imgui wants to use your inputs.
  // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
  // your main application, or clear/overwrite your copy of the mouse data.
  // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
  // data to your main application, or clear/overwrite your copy of the
  // keyboard data. Generally you may always pass all inputs to dear imgui,
  // and hide them from your application based on those two flags.
  glfwPollEvents();

  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();

  RenderCore();

  int display_w, display_h;
  glfwGetFramebufferSize(window_, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  glClearColor(clear_color_.x * clear_color_.w, clear_color_.y * clear_color_.w,
               clear_color_.z * clear_color_.w, clear_color_.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  glfwSwapBuffers(window_);
}

namespace {

// See https://github.com/ocornut/imgui#usage.
void RenderMyFirstTool() {
  ImGui::Begin("My First Tool", nullptr, ImGuiWindowFlags_MenuBar);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Open..", "Ctrl+O")) {
        LOG(INFO) << "Open clicked.";
      }
      if (ImGui::MenuItem("Save", "Ctrl+S")) {
        LOG(INFO) << "Save clicked.";
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  static float my_color;
  if (ImGui::ColorEdit4("Color", &my_color)) {
    LOG(INFO) << "Color: " << my_color;
  }

  // Generate samples and plot them.
  std::array<float, 100> samples;
  for (int i = 0; i < samples.size(); ++i) {
    samples[i] = std::sin(i * 0.2 + ImGui::GetTime() * 1.5);
  }
  ImGui::PlotLines("Samples", samples.data(), samples.size());

  // Display contents in a scrolling region
  ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
  ImGui::BeginChild("Scrolling");
  for (int n = 0; n < 50; n++) {
    ImGui::Text("%04d: Some text", n);
  }
  ImGui::EndChild();
  ImGui::End();
}

static const char* SequencerItemTypeNames[] = {
    "Camera", "Music", "ScreenEffect", "FadeIn", "Animation"};

struct RampEdit : public ImCurveEdit::Delegate {
  RampEdit() {
    mPts[0][0] = ImVec2(-10.f, 0);
    mPts[0][1] = ImVec2(20.f, 0.6f);
    mPts[0][2] = ImVec2(25.f, 0.2f);
    mPts[0][3] = ImVec2(70.f, 0.4f);
    mPts[0][4] = ImVec2(120.f, 1.f);
    mPointCount[0] = 5;

    mPts[1][0] = ImVec2(-50.f, 0.2f);
    mPts[1][1] = ImVec2(33.f, 0.7f);
    mPts[1][2] = ImVec2(80.f, 0.2f);
    mPts[1][3] = ImVec2(82.f, 0.8f);
    mPointCount[1] = 4;

    mPts[2][0] = ImVec2(40.f, 0);
    mPts[2][1] = ImVec2(60.f, 0.1f);
    mPts[2][2] = ImVec2(90.f, 0.82f);
    mPts[2][3] = ImVec2(150.f, 0.24f);
    mPts[2][4] = ImVec2(200.f, 0.34f);
    mPts[2][5] = ImVec2(250.f, 0.12f);
    mPointCount[2] = 6;
    mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
    mMax = ImVec2(1.f, 1.f);
    mMin = ImVec2(0.f, 0.f);
  }
  size_t GetCurveCount() { return 3; }

  bool IsVisible(size_t curveIndex) { return mbVisible[curveIndex]; }
  size_t GetPointCount(size_t curveIndex) { return mPointCount[curveIndex]; }

  uint32_t GetCurveColor(size_t curveIndex) {
    uint32_t cols[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
    return cols[curveIndex];
  }
  ImVec2* GetPoints(size_t curveIndex) { return mPts[curveIndex]; }
  virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const {
    return ImCurveEdit::CurveSmooth;
  }
  virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) {
    mPts[curveIndex][pointIndex] = ImVec2(value.x, value.y);
    SortValues(curveIndex);
    for (size_t i = 0; i < GetPointCount(curveIndex); i++) {
      if (mPts[curveIndex][i].x == value.x) return (int)i;
    }
    return pointIndex;
  }
  virtual void AddPoint(size_t curveIndex, ImVec2 value) {
    if (mPointCount[curveIndex] >= 8) return;
    mPts[curveIndex][mPointCount[curveIndex]++] = value;
    SortValues(curveIndex);
  }
  virtual ImVec2& GetMax() { return mMax; }
  virtual ImVec2& GetMin() { return mMin; }
  virtual unsigned int GetBackgroundColor() { return 0; }
  ImVec2 mPts[3][8];
  size_t mPointCount[3];
  bool mbVisible[3];
  ImVec2 mMin;
  ImVec2 mMax;

 private:
  void SortValues(size_t curveIndex) {
    auto b = std::begin(mPts[curveIndex]);
    auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
    std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
  }
};

struct MySequence : public ImSequencer::SequenceInterface {
  // interface with sequencer

  int GetFrameMin() const override { return mFrameMin; }
  int GetFrameMax() const override { return mFrameMax; }
  int GetItemCount() const override { return (int)myItems.size(); }

  int GetItemTypeCount() const override {
    return sizeof(SequencerItemTypeNames) / sizeof(char*);
  }
  const char* GetItemTypeName(int typeIndex) const override {
    return SequencerItemTypeNames[typeIndex];
  }
  const char* GetItemLabel(int index) const override {
    static char tmps[512];
    snprintf(tmps, 512, "[%02d] %s", index,
             SequencerItemTypeNames[myItems[index].mType]);
    return tmps;
  }

  void Get(int index, int** start, int** end, int* type,
           unsigned int* color) override {
    MySequenceItem& item = myItems[index];
    if (color)
      *color =
          0xFFAA8080;  // same color for everyone, return color based on type
    if (start) *start = &item.mFrameStart;
    if (end) *end = &item.mFrameEnd;
    if (type) *type = item.mType;
  }
  void Add(int type) override {
    myItems.push_back(MySequenceItem{type, 0, 10, false});
  };
  void Del(int index) override { myItems.erase(myItems.begin() + index); }
  void Duplicate(int index) override { myItems.push_back(myItems[index]); }

  size_t GetCustomHeight(int index) override {
    return myItems[index].mExpanded ? 300 : 0;
  }

  void DoubleClick(int index) override {
    if (myItems[index].mExpanded) {
      myItems[index].mExpanded = false;
      return;
    }
    for (auto& item : myItems) item.mExpanded = false;
    myItems[index].mExpanded = !myItems[index].mExpanded;
  }

  void CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc,
                  const ImRect& legendRect, const ImRect& clippingRect,
                  const ImRect& legendClippingRect) override {
    static const char* labels[] = {"Translation", "Rotation", "Scale"};

    rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
    rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
    draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max,
                            true);
    for (int i = 0; i < 3; i++) {
      ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
      ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
      draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF,
                         labels[i]);
      if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) &&
          ImGui::IsMouseClicked(0))
        rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
    }
    draw_list->PopClipRect();

    ImGui::SetCursorScreenPos(rc.Min);
    ImVec2 diff{rc.Max.x - rc.Min.x, rc.Max.y - rc.Min.y};
    ImCurveEdit::Edit(rampEdit, diff, 137 + index, &clippingRect);
  }

  void CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc,
                         const ImRect& clippingRect) override {
    rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
    rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
    draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < rampEdit.mPointCount[i]; j++) {
        float p = rampEdit.mPts[i][j].x;
        if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
          continue;
        float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
        float x = ImLerp(rc.Min.x, rc.Max.x, r);
        draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4),
                           0xAA000000, 4.f);
      }
    }
    draw_list->PopClipRect();
  }

  struct MySequenceItem {
    int mType;
    int mFrameStart, mFrameEnd;
    bool mExpanded;
  };

  MySequence(int frameMin, int frameMax, std::vector<MySequenceItem> items)
      : mFrameMin(frameMin), mFrameMax(frameMax), myItems(std::move(items)) {}

  // my datas
  int mFrameMin = 0;
  int mFrameMax = 0;
  std::vector<MySequenceItem> myItems;
  RampEdit rampEdit;
};

// From https://github.com/CedricGuillemet/ImGuizmo/blob/master/example/main.cpp
void RenderSequencer() {
  // sequence with default values
  // static std::vector<MySequence::MySequenceItem> initItems;
  static MySequence mySequence{-100,
                               1000,
                               {{0, 10, 30, false},
                                {1, 20, 30, true},
                                {3, 12, 60, false},
                                {2, 61, 90, false},
                                {4, 90, 99, false}}};

  ImGui::SetNextWindowSize(ImVec2(940, 480), ImGuiCond_Appearing);
  ImGui::Begin("Sequencer");
  if (ImGui::CollapsingHeader("Sequencer")) {
    // let's create the sequencer
    static int selectedEntry = -1;
    static int firstFrame = 0;
    static bool expanded = true;
    static int currentFrame = 100;

    ImGui::PushItemWidth(130);
    ImGui::InputInt("Frame Min", &mySequence.mFrameMin);
    ImGui::SameLine();
    ImGui::InputInt("Frame ", &currentFrame);
    ImGui::SameLine();
    ImGui::InputInt("Frame Max", &mySequence.mFrameMax);
    ImGui::PopItemWidth();
    ImSequencer::Sequencer(
        &mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame,
        ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD |
            ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE |
            ImSequencer::SEQUENCER_CHANGE_FRAME);
    // add a UI to edit that particular item
    if (selectedEntry != -1) {
      const MySequence::MySequenceItem& item =
          mySequence.myItems[selectedEntry];
      ImGui::Text("I am a %s, please edit me",
                  SequencerItemTypeNames[item.mType]);
      // switch (type) ....
    }
  }
  ImGui::End();
}

}  // namespace

void Gui::RenderCore() {
  ImGui::NewFrame();
  RenderSequencer();
  RenderMyFirstTool();

  // 1. Show the big demo window (Most of the sample code is in
  // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
  // ImGui!).
  if (show_demo_window_) ImGui::ShowDemoWindow(&show_demo_window_);

  // 2. Show a simple window that we create ourselves. We use a Begin/End pair
  // to create a named window.
  {
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!");  // Create a window called "Hello, world!"
                                    // and append into it.

    ImGui::Text("This is some useful text.");  // Display some text (you can
                                               // use a format strings too)
    ImGui::Checkbox("Demo Window",
                    &show_demo_window_);  // Edit bools storing our window
                                          // open/close state
    ImGui::Checkbox("Another Window", &show_another_window_);

    ImGui::SliderFloat("float", &f, 0.0f,
                       1.0f);  // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3(
        "clear color",
        (float*)&clear_color_);  // Edit 3 floats representing a color

    if (ImGui::Button("Button"))  // Buttons return true when clicked (most
                                  // widgets return true when edited/activated)
      counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
  }

  // 3. Show another simple window.
  if (show_another_window_) {
    ImGui::Begin(
        "Another Window",
        &show_another_window_);  // Pass a pointer to our bool variable (the
                                 // window will have a closing button that
                                 // will clear the bool when clicked)
    ImGui::Text("Hello from another window!");
    if (ImGui::Button("Close Me")) show_another_window_ = false;
    ImGui::End();
  }

  // Rendering
  ImGui::Render();
}

}  // namespace kodo
