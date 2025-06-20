#include "../include/SpatialGrid.hpp"

SpatialGrid::SpatialGrid(float width, float height, float depth,
                         float cellSize)
    : m_cellSize(cellSize) {}

void SpatialGrid::insert(const std::unique_ptr<PhysicsObject> &object,
                         bool is3D) {
  auto coords = getCellCoords(object->position());
  if (!is3D) {
    std::get<2>(coords) = 0;
  }
  m_grid[coords].push_back(object.get());
}

std::vector<PhysicsObject *> SpatialGrid::getPotentialColliders(
    const std::unique_ptr<PhysicsObject> &object, bool is3D) {
  std::vector<PhysicsObject *> potentialColliders;
  auto centerCoords = getCellCoords(object->position());

  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      for (int z = (is3D ? -1 : 0); z <= (is3D ? 1 : 0); ++z) {
        std::tuple<int, int, int> cell = {
            std::get<0>(centerCoords) + x, std::get<1>(centerCoords) + y,
            std::get<2>(centerCoords) + z};
        if (m_grid.count(cell)) {
          potentialColliders.insert(potentialColliders.end(),
                                    m_grid.at(cell).begin(),
                                    m_grid.at(cell).end());
        }
      }
    }
  }
  return potentialColliders;
}

void SpatialGrid::clear() { m_grid.clear(); }

std::tuple<int, int, int> SpatialGrid::getCellCoords(const glm::vec3 &pos) {
  int cellX = static_cast<int>(pos.x / m_cellSize);
  int cellY = static_cast<int>(pos.y / m_cellSize);
  int cellZ = static_cast<int>(pos.z / m_cellSize);
  return {cellX, cellY, cellZ};
}
