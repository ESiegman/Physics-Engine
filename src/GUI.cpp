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
    ImGuiID dock_left_top_id = ImGui::DockBuilderSplitNode(
        dock_left_id, ImGuiDir_Up, 0.75f, nullptr, &dock_left_id);
    ImGuiID dock_left_bottom_id = dock_left_id;

    ImGui::DockBuilderDockWindow("Settings", dock_left_top_id);
    ImGui::DockBuilderDockWindow("Camera Controls", dock_left_bottom_id);
    ImGui::DockBuilderDockWindow("Scene", dock_main_id);
    ImGui::DockBuilderFinish(dockspace_id);
  }

  ImGui::End();

  ImGui::Begin("Settings");
  if (ImGui::Button("Restart Simulation")) {
    sim.restart();
  }
  ImGui::Separator();
  if (ImGui::Button("Open Camera Controls")) {
    m_showCameraControlsWindow = !m_showCameraControlsWindow;
  }
  ImGui::Separator();

  ImGui::Text("Simulation Parameters (Restart Require for Change in Num)");
  int num_objects_val = sim.m_constants.NUM_OBJECTS;
  if (ImGui::InputInt("Number of Objects", &num_objects_val, 100, 1000)) {
    if (num_objects_val < 1)
      num_objects_val = 1;
    sim.m_constants.NUM_OBJECTS = num_objects_val;
    sim.restart();
  }
  ImGui::SliderFloat("Gravity", &sim.m_constants.GRAVITY, -2000.0f, 0.0f);
  ImGui::SliderFloat("Bounciness", &sim.m_constants.COEFFICIENT_OF_RESTITUTION,
                     0.0f, 1.0f);
  ImGui::SliderFloat("Vertical Damping", &sim.m_constants.VERTICAL_DAMPING,
                     0.0f, 1.0f);
  ImGui::Checkbox("Use 3D", &sim.m_constants.USE_3D);

  ImGui::Separator();
  ImGui::Text("World Dimensions (Restart Required for Full Effect)");
  ImGui::InputFloat("World Width", &sim.m_constants.WORLD_WIDTH);
  ImGui::InputFloat("World Height", &sim.m_constants.WORLD_HEIGHT);
  ImGui::InputFloat("World Depth", &sim.m_constants.WORLD_DEPTH);

  ImGui::Separator();
  ImGui::Text("Physics Engine Settings");
  ImGui::InputFloat("Fixed Delta Time", &sim.m_constants.FIXED_DELTA_TIME);
  ImGui::InputInt("Physics Iterations", &sim.m_constants.PHYSICS_ITERATIONS);

  ImGui::Separator();
  ImGui::Text("Default Object Properties (Restart Required)");
  ImGui::SliderFloat("Radius", &sim.m_constants.OBJECT_DEFAULT_RADIUS, 1.0f,
                     50.0f);
  ImGui::InputFloat("Mass", &sim.m_constants.OBJECT_DEFAULT_MASS);
  ImGui::SliderFloat("Min Start Velocity", &sim.m_constants.OBJECT_MIN_VEL,
                     -2000.0f, 0.0f);
  ImGui::SliderFloat("Max Start Velocity", &sim.m_constants.OBJECT_MAX_VEL,
                     0.0f, 2000.0f);

  ImGui::Separator();
  ImGui::Text("Spatial Grid Settings (Restart Required)");
  ImGui::InputFloat("Cell Size 2D", &sim.m_constants.CELL_SIZE_2D);
  ImGui::InputFloat("Cell Size 3D", &sim.m_constants.CELL_SIZE_3D);

  ImGui::Separator();
  ImGui::Text("Camera Settings");
  ImGui::SliderFloat("Movement Speed", &sim.m_constants.CAMERA_MOVEMENT_SPEED,
                     100.0f, 5000.0f);
  ImGui::SliderFloat("Mouse Sensitivity",
                     &sim.m_constants.CAMERA_MOUSE_SENSITIVITY, 0.01f, 1.0f);
  ImGui::SliderFloat("FOV", &sim.m_constants.CAMERA_FOV, 30.0f, 90.0f);

  ImGui::End();

  if (m_showCameraControlsWindow) {
    ImGui::Begin("Camera Controls", &m_showCameraControlsWindow);
    ImGui::Text("Movement Controls:");
    ImGui::BulletText("W: Move Forward");
    ImGui::BulletText("S: Move Backward");
    ImGui::BulletText("A: Move Left");
    ImGui::BulletText("D: Move Right");
    ImGui::BulletText("Space: Move Up");
    ImGui::BulletText("Left Shift: Move Down");
    ImGui::BulletText("Mouse Movement: Look Around (when not locked)");
    ImGui::BulletText("Left Control: Toggle Camera Lock");
    ImGui::BulletText("ESC: Close Window");

    ImGui::End();
  }

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("Scene");
  ImVec2 scene_view_size = ImGui::GetContentRegionAvail();
  ImGui::Image((ImTextureID)(uintptr_t)sceneTexture, scene_view_size,
               ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::Render();
}
