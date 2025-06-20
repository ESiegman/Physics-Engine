#pragma once

#include "Camera.hpp"
#include "Constants.hpp"
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
  SimulationConstants m_constants;

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
      SpatialGrid &grid, size_t start_idx, size_t end_idx,
      const SimulationConstants &constants);
};
