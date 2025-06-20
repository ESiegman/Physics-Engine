#include "../include/GUI.hpp"
#include "../include/Simulation.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

void GUI::init(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  ImGui::StyleColorsDark();
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
  }

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
}

void GUI::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void GUI::render(Simulation &sim, GLuint sceneTexture, int display_w,
                 int display_h) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos);
  ImGui::SetNextWindowSize(viewport->Size);
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                  ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoBringToFrontOnFocus |
                  ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("DockSpaceHost", nullptr, window_flags);
  ImGui::PopStyleVar(2);
  ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

  if (m_firstTime) {
    m_firstTime = false;
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

    ImGuiID dock_main_id = dockspace_id;
    ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(
        dock_main_id, ImGuiDir_Left, 0.25f, nullptr, &dock_main_id);

    ImGui::DockBuilderDockWindow("Settings", dock_left_id);
    ImGui::DockBuilderDockWindow("Scene", dock_main_id);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::End();

  ImGui::Begin("Settings");
  if (ImGui::Button("Restart Simulation")) {
    sim.restart();
  }
  ImGui::Separator();

  ImGui::Text("Simulation Parameters");
  int num_objects = sim.NUM_OBJECTS;
  if (ImGui::InputInt("Number of Objects", &num_objects, 100, 1000)) {
    if (num_objects < 1)
      num_objects = 1;
    sim.NUM_OBJECTS = num_objects;
  }

  ImGui::SliderFloat("Gravity", &sim.GRAVITY, -2000.0f, 0.0f);
  ImGui::SliderFloat("Bounciness", &sim.COEFFICIENT_OF_RESTITUTION, 0.0f, 1.0f);

  ImGui::Separator();
  ImGui::Text("Default Object Properties");
  ImGui::SliderFloat("Radius", &sim.OBJECT_DEFAULT_RADIUS, 1.0f, 50.0f);

  ImGui::End();

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Scene");
  ImVec2 scene_view_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)(uintptr_t)sceneTexture, scene_view_size,
               ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::Render();
}
