#include "../include/Simulation.hpp"
#include "../include/Constants.hpp"

#include "imgui.h"

#include <chrono>
#include <iostream>
#include <thread>

Simulation::Simulation()
    : m_camera(glm::vec3(Constants::WORLD_WIDTH / 2.0f,
                         Constants::WORLD_HEIGHT / 2.0f, 3000.0f)),
      m_window(1920, 1080, "Physics Engine", &m_camera, true),
      m_grid(Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT,
             Constants::WORLD_DEPTH,
             Constants::USE_3D ? Constants::CELL_SIZE_3D
                               : Constants::CELL_SIZE_2D) {
  m_gui.init(m_window.getGlfwWindow());
  restart();
}

Simulation::~Simulation() { m_gui.shutdown(); }

void Simulation::restart() {
  m_objects.clear();
  for (size_t i = 0; i < NUM_OBJECTS; ++i) {
    m_objects.emplace_back(std::make_unique<PhysicsObject>(
        USE_3D, OBJECT_DEFAULT_RADIUS,
        OBJECT_DEFAULT_RADIUS * OBJECT_DEFAULT_RADIUS));
  }
}

void Simulation::run() {
  GLuint fbo, fbo_texture, rbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &fbo_texture);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1920, 1080, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         fbo_texture, 0);

  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1920, 1080);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cerr << "Framebuffer incomplete!" << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  auto last_time = std::chrono::high_resolution_clock::now();
  int display_w = 1920, display_h = 1080;

  while (!m_window.shouldClose()) {
    auto current_time = std::chrono::high_resolution_clock::now();
    float frame_delta_time =
        std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;

    m_window.processInput(frame_delta_time);

    const float SUB_DELTA_TIME =
        Constants::FIXED_DELTA_TIME / Constants::PHYSICS_ITERATIONS;
    for (int iter = 0; iter < Constants::PHYSICS_ITERATIONS; ++iter) {
      for (auto &obj_ptr : m_objects) {
        obj_ptr->update(SUB_DELTA_TIME, USE_3D, GRAVITY);
      }
      m_grid.clear();
      for (const auto &obj_ptr : m_objects) {
        m_grid.insert(obj_ptr, USE_3D);
      }

      const unsigned int num_threads = std::thread::hardware_concurrency();
      std::vector<std::thread> threads;
      size_t chunk_size = NUM_OBJECTS / num_threads;
      size_t current_start_idx = 0;
      for (unsigned int t = 0; t < num_threads; ++t) {
        size_t end_idx = current_start_idx + chunk_size +
                         (t < (NUM_OBJECTS % num_threads) ? 1 : 0);
        threads.emplace_back(checkCollisionsForChunk, std::ref(m_objects),
                             std::ref(m_grid), current_start_idx, end_idx,
                             USE_3D, COEFFICIENT_OF_RESTITUTION);
        current_start_idx = end_idx;
      }
      for (std::thread &t : threads) {
        if (t.joinable())
          t.join();
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, display_w, display_h);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection =
        m_camera.getProjectionMatrix((float)display_w / (float)display_h);

    if (USE_3D) {
      m_renderer.renderContainerBox(view, projection);
      m_renderer.renderFloor(view, projection);
    }

    for (const auto &obj_ptr : m_objects) {
      m_renderer.renderObject(obj_ptr->position(), obj_ptr->radius(),
                              obj_ptr->color(), (float)display_w,
                              (float)display_h, USE_3D, view, projection);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_gui.render(*this, fbo_texture, display_w, display_h);

    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    m_window.swapBuffersAndPollEvents();
  }

  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &fbo_texture);
  glDeleteRenderbuffers(1, &rbo);
}

void Simulation::checkCollisionsForChunk(
    const std::vector<std::unique_ptr<PhysicsObject>> &objects,
    SpatialGrid &grid, size_t start_idx, size_t end_idx, bool is_3d,
    float restitution) {
  for (size_t i = start_idx; i < end_idx; ++i) {
    auto potential_colliders = grid.getPotentialColliders(objects[i], is_3d);
    for (PhysicsObject *other_object : potential_colliders) {
      if (objects[i].get() < other_object) {
        collision(*objects[i], *other_object, is_3d, restitution);
      }
    }
  }
}
