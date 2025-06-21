#include "../include/Simulation.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <chrono>
#include <iostream>
#include <map>
#include <thread>

struct ShaderPhysicsObjectData {
  glm::vec3 position;
  float radius;
  glm::vec3 color;
  float padding1;
};

struct ShaderPointLightData {
  glm::vec3 position;
  float intensity;
  glm::vec3 color;
  float padding2;
};

struct GpuPhysicsObject {
  glm::vec3 position;
  float radius;
  glm::vec3 color;
  float reflectivity;
};

struct GpuPointLight {
  glm::vec3 position;
  float intensity;
  glm::vec3 color;
  float padding;
};

struct SceneData {
  glm::vec3 cameraPos;
  float padding1;
  glm::mat4 viewInverse;
  glm::mat4 projectionInverse;
  glm::vec3 worldBoundsMin;
  float padding2;
  glm::vec3 worldBoundsMax;
  float padding3;
  glm::vec3 physicsBoundsMin;
  float padding4;
  glm::vec3 physicsBoundsMax;
  float padding5;
  glm::ivec2 debugPixelCoord;
  int numLights;
  int numObjects;
};

void checkGLErrors(const char *label) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "OpenGL Error at " << label << ": " << err << std::endl;
  }
}

Simulation::Simulation()
    : m_camera(glm::vec3(m_constants.WORLD_WIDTH / 2.0f,
                         m_constants.WORLD_HEIGHT / 2.0f, 3000.0f),
               glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, m_constants),
      m_window(1920, 1080, "Physics Engine", &m_camera, m_constants.USE_3D),
      m_grid(m_constants.WORLD_WIDTH, m_constants.WORLD_HEIGHT,
             m_constants.WORLD_DEPTH,
             m_constants.USE_3D ? m_constants.CELL_SIZE_3D
                                : m_constants.CELL_SIZE_2D) {
  m_gui.init(m_window.getGlfwWindow());

  m_raytracingComputeShader =
      new Shader("shaders/raytracer.comp", ShaderType::COMPUTE_SHADER);

  if (m_raytracingComputeShader->ID == 0) {
    std::cerr << "----------------------------------------------------------"
              << std::endl;
    std::cerr
        << "FATAL DIAGNOSTIC: The compute shader program failed to be created."
        << std::endl;
    std::cerr << "Please scroll up in the console to look for an "
                 "'ERROR::SHADER_COMPILATION_ERROR' or "
                 "'ERROR::PROGRAM_LINKING_ERROR' message."
              << std::endl;
    std::cerr << "----------------------------------------------------------"
              << std::endl;
  }

  m_pointLights.push_back({glm::vec3(m_constants.WORLD_WIDTH / 2.0f,
                                     m_constants.WORLD_HEIGHT / 2.0f,
                                     m_constants.WORLD_DEPTH / 2.0f),
                           glm::vec3(1.0f, 1.0f, 1.0f), 1.0f});
  m_pointLights.push_back(
      {glm::vec3(0.0f, m_constants.WORLD_HEIGHT, 0.0f),
       glm::vec3(1.0f, 1.0f, 1.0f), 3.0f});

  restart();
}

Simulation::~Simulation() {
  m_gui.shutdown();
  delete m_raytracingComputeShader;
}

void Simulation::restart() {
  m_objects.clear();
  for (int i = 0; i < m_constants.NUM_OBJECTS; ++i) {
    m_objects.emplace_back(std::make_unique<PhysicsObject>(
        m_constants, m_constants.USE_3D, m_constants.OBJECT_DEFAULT_RADIUS,
        m_constants.OBJECT_DEFAULT_MASS));
  }
}

void Simulation::notifyWorldDimensionsChanged() {
  m_grid = SpatialGrid(m_constants.WORLD_WIDTH, m_constants.WORLD_HEIGHT,
                       m_constants.WORLD_DEPTH,
                       m_constants.USE_3D ? m_constants.CELL_SIZE_3D
                                          : m_constants.CELL_SIZE_2D);
}

void Simulation::run() {
  GLuint fbo, fbo_texture, rbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  glGenTextures(1, &fbo_texture);
  glBindTexture(GL_TEXTURE_2D, fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1920, 1080, 0, GL_RGBA,
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

  GLuint objectSSBO, lightSSBO, gridCellsSSBO, objectIndicesSSBO;
  glGenBuffers(1, &objectSSBO);
  glGenBuffers(1, &lightSSBO);
  glGenBuffers(1, &gridCellsSSBO);
  glGenBuffers(1, &objectIndicesSSBO);

  while (!m_window.shouldClose()) {

    auto current_time = std::chrono::high_resolution_clock::now();
    float frame_delta_time =
        std::chrono::duration<float>(current_time - last_time).count();
    last_time = current_time;

    m_window.processInput(frame_delta_time);

    const float SUB_DELTA_TIME =
        m_constants.FIXED_DELTA_TIME / m_constants.PHYSICS_ITERATIONS;
    for (int iter = 0; iter < m_constants.PHYSICS_ITERATIONS; ++iter) {
      for (auto &obj_ptr : m_objects) {
        obj_ptr->update(SUB_DELTA_TIME);
      }
      m_grid.clear();
      for (const auto &obj_ptr : m_objects) {
        m_grid.insert(obj_ptr, m_constants.USE_3D);
      }
      const unsigned int num_threads = std::thread::hardware_concurrency();
      std::vector<std::thread> threads;
      size_t chunk_size = m_objects.size() / num_threads;
      if (chunk_size == 0 && m_objects.size() > 0) {
        chunk_size = 1;
      }
      size_t current_start_idx = 0;
      for (unsigned int t = 0;
           t < num_threads && current_start_idx < m_objects.size(); ++t) {
        size_t end_idx =
            std::min(current_start_idx + chunk_size, m_objects.size());
        if (t == num_threads - 1) {
          end_idx = m_objects.size();
        }
        threads.emplace_back(checkCollisionsForChunk, std::ref(m_objects),
                             std::ref(m_grid), current_start_idx, end_idx,
                             m_constants);
        current_start_idx = end_idx;
      }
      for (std::thread &t : threads) {
        if (t.joinable())
          t.join();
      }
    }

    if (m_worldDimensionsChanged) {
      m_worldDimensionsChanged = false;
    }

    std::vector<GpuPhysicsObject> shaderObjects(m_objects.size());
    for (size_t i = 0; i < m_objects.size(); ++i) {
      shaderObjects[i].position = m_objects[i]->position();
      shaderObjects[i].radius = m_objects[i]->radius();
      shaderObjects[i].color = m_objects[i]->color();
      shaderObjects[i].reflectivity = 0.4f;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 shaderObjects.size() * sizeof(GpuPhysicsObject),
                 shaderObjects.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, objectSSBO);

    std::vector<GpuPointLight> shaderLights(m_pointLights.size());
    for (size_t i = 0; i < m_pointLights.size(); ++i) {
      shaderLights[i].position = m_pointLights[i].position;
      shaderLights[i].intensity = m_pointLights[i].intensity;
      shaderLights[i].color = m_pointLights[i].color;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lightSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 shaderLights.size() * sizeof(GpuPointLight),
                 shaderLights.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, lightSSBO);

    int cellsX = static_cast<int>(
        std::ceil(m_constants.WORLD_WIDTH / (m_constants.USE_3D
                                                 ? m_constants.CELL_SIZE_3D
                                                 : m_constants.CELL_SIZE_2D)));
    int cellsY = static_cast<int>(
        std::ceil(m_constants.WORLD_HEIGHT / (m_constants.USE_3D
                                                  ? m_constants.CELL_SIZE_3D
                                                  : m_constants.CELL_SIZE_2D)));
    int cellsZ = static_cast<int>(
        std::ceil(m_constants.WORLD_DEPTH / (m_constants.USE_3D
                                                 ? m_constants.CELL_SIZE_3D
                                                 : m_constants.CELL_SIZE_2D)));
    if (cellsX == 0)
      cellsX = 1;
    if (cellsY == 0)
      cellsY = 1;
    if (cellsZ == 0)
      cellsZ = 1;

    std::map<int, std::vector<unsigned int>> cellObjectsMap;
    float cellSize = m_constants.USE_3D ? m_constants.CELL_SIZE_3D
                                        : m_constants.CELL_SIZE_2D;
    for (size_t i = 0; i < m_objects.size(); ++i) {
      const auto &obj = m_objects[i];
      glm::vec3 min_bound = obj->position() - glm::vec3(obj->radius());
      glm::vec3 max_bound = obj->position() + glm::vec3(obj->radius());
      glm::ivec3 min_cell = glm::ivec3(floor(min_bound.x / cellSize),
                                       floor(min_bound.y / cellSize),
                                       floor(min_bound.z / cellSize));
      glm::ivec3 max_cell = glm::ivec3(floor(max_bound.x / cellSize),
                                       floor(max_bound.y / cellSize),
                                       floor(max_bound.z / cellSize));
      for (int x = min_cell.x; x <= max_cell.x; ++x) {
        for (int y = min_cell.y; y <= max_cell.y; ++y) {
          for (int z = (m_constants.USE_3D ? min_cell.z : 0);
               z <= (m_constants.USE_3D ? max_cell.z : 0); ++z) {
            glm::ivec3 cellCoords(x, y, z);
            if (cellCoords.x >= 0 && cellCoords.x < cellsX &&
                cellCoords.y >= 0 && cellCoords.y < cellsY &&
                cellCoords.z >= 0 && cellCoords.z < cellsZ) {
              int cellIndex = cellCoords.x + cellCoords.y * cellsX +
                              cellCoords.z * cellsX * cellsY;
              cellObjectsMap[cellIndex].push_back(i);
            }
          }
        }
      }
    }
    std::vector<GpuGridCell> gpuGridCells(cellsX * cellsY * cellsZ);
    std::vector<unsigned int> gpuObjectIndices;
    for (size_t i = 0; i < gpuGridCells.size(); ++i) {
      gpuGridCells[i].objectStartIndex = gpuObjectIndices.size();
      if (cellObjectsMap.count(i)) {
        const auto &objectsInCell = cellObjectsMap.at(i);
        gpuGridCells[i].objectCount = objectsInCell.size();
        gpuObjectIndices.insert(gpuObjectIndices.end(), objectsInCell.begin(),
                                objectsInCell.end());
      } else {
        gpuGridCells[i].objectCount = 0;
      }
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, gridCellsSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 gpuGridCells.size() * sizeof(GpuGridCell), gpuGridCells.data(),
                 GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, gridCellsSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, objectIndicesSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 gpuObjectIndices.size() * sizeof(unsigned int),
                 gpuObjectIndices.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, objectIndicesSSBO);

    m_raytracingComputeShader->use();

    glm::vec3 physicsCenter(m_constants.WORLD_WIDTH / 2.0f,
                            m_constants.WORLD_HEIGHT / 2.0f,
                            m_constants.WORLD_DEPTH / 2.0f);
    float renderMargin = 4000.0f;
    glm::vec3 renderHalfSize((m_constants.WORLD_WIDTH / 2.0f) + renderMargin,
                             (m_constants.WORLD_HEIGHT / 2.0f) + renderMargin,
                             (m_constants.WORLD_DEPTH / 2.0f) + renderMargin);
    glm::vec3 worldBoundsMin = physicsCenter - renderHalfSize;
    glm::vec3 worldBoundsMax = physicsCenter + renderHalfSize;

    m_raytracingComputeShader->setVec3("cameraPos", m_camera.Position);
    m_raytracingComputeShader->setMat4("viewInverse",
                                       glm::inverse(m_camera.getViewMatrix()));
    m_raytracingComputeShader->setMat4(
        "projectionInverse", glm::inverse(m_camera.getProjectionMatrix(
                                 (float)display_w / (float)display_h)));
    m_raytracingComputeShader->setInt("numObjects", m_objects.size());
    m_raytracingComputeShader->setInt("numLights", m_pointLights.size());
    m_raytracingComputeShader->setVec3("worldBoundsMin", worldBoundsMin);
    m_raytracingComputeShader->setVec3("worldBoundsMax", worldBoundsMax);
    m_raytracingComputeShader->setVec3("physicsBoundsMin", glm::vec3(0.0f));
    m_raytracingComputeShader->setVec3("physicsBoundsMax",
                                       glm::vec3(m_constants.WORLD_WIDTH,
                                                 m_constants.WORLD_HEIGHT,
                                                 m_constants.WORLD_DEPTH));
    m_raytracingComputeShader->setInt("gridCellsX", cellsX);
    m_raytracingComputeShader->setInt("gridCellsY", cellsY);
    m_raytracingComputeShader->setInt("gridCellsZ", cellsZ);
    m_raytracingComputeShader->setFloat("cellSize", cellSize);

    glBindImageTexture(0, fbo_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glDispatchCompute((GLuint)std::ceil((float)display_w / 8.0f),
                      (GLuint)std::ceil((float)display_h / 8.0f), 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_gui.render(*this, fbo_texture, display_w, display_h);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGuiIO &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }

    m_window.swapBuffersAndPollEvents();
  }

  glDeleteBuffers(1, &objectSSBO);
  glDeleteBuffers(1, &lightSSBO);
  glDeleteBuffers(1, &gridCellsSSBO);
  glDeleteBuffers(1, &objectIndicesSSBO);

  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &fbo_texture);
  glDeleteRenderbuffers(1, &rbo);
}

void Simulation::checkCollisionsForChunk(
    const std::vector<std::unique_ptr<PhysicsObject>> &objects,
    SpatialGrid &grid, size_t start_idx, size_t end_idx,
    const SimulationConstants &constants) {
  for (size_t i = start_idx; i < end_idx; ++i) {
    grid.processPotentialColliders(
        objects[i], constants.USE_3D, [&](PhysicsObject *other_object) {
          if (objects[i].get() < other_object) {
            collision(*objects[i], *other_object, constants);
          }
        });
  }
}
