#pragma once

#include "Camera.hpp"
#include "GUI.hpp"
#include "PhysicsObject.hpp"
#include "Renderer.hpp"
#include "SpatialGrid.hpp"
#include "Window.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class GUI;

class Simulation {
public:
  bool USE_3D = true;
  size_t NUM_OBJECTS = 2000;
  float GRAVITY = -980.0f;
  float OBJECT_DEFAULT_RADIUS = 10.0f;
  float COEFFICIENT_OF_RESTITUTION = 0.95f;

  Simulation();
  ~Simulation();

  void run();
  void restart();

private:
  friend class GUI;

  Camera m_camera;
  Window m_window;
  Renderer m_renderer;
  std::vector<std::unique_ptr<PhysicsObject>> m_objects;
  SpatialGrid m_grid;
  GUI m_gui;

  static void checkCollisionsForChunk(
      const std::vector<std::unique_ptr<PhysicsObject>> &objects,
      SpatialGrid &grid, size_t start_idx, size_t end_idx, bool is_3d,
      float restitution);
};
