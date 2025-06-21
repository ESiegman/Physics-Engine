#pragma once

#include "Camera.hpp"
#include "Constants.hpp"
#include "GUI.hpp"
#include "PhysicsObject.hpp"
#include "Shader.hpp"
#include "SpatialGrid.hpp"
#include "Window.hpp"

#include <cstddef>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct PointLight {
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
};

struct GpuGridCell {
  unsigned int objectStartIndex;
  unsigned int objectCount;
};

class Simulation {
public:
  SimulationConstants m_constants;

  Simulation();
  ~Simulation();

  void run();
  void restart();

  void notifyWorldDimensionsChanged();

private:
  friend class GUI;

  Camera m_camera;
  Window m_window;
  std::vector<std::unique_ptr<PhysicsObject>> m_objects;
  SpatialGrid m_grid;
  GUI m_gui;
  glm::ivec2 m_debugPixel = glm::ivec2(960, 540);
  bool m_worldDimensionsChanged = false;

  Shader *m_raytracingComputeShader;
  std::vector<PointLight> m_pointLights;

  static void checkCollisionsForChunk(
      const std::vector<std::unique_ptr<PhysicsObject>> &objects,
      SpatialGrid &grid, size_t start_idx, size_t end_idx,
      const SimulationConstants &constants);
};
