#include "../include/SpatialGrid.hpp"

#include <cmath>
#include <iostream>

const int RESERVE_PER_CELL = 20;

SpatialGrid::SpatialGrid(float width, float height, float depth, float cellSize)
    : m_cellSize(cellSize) {
  m_cellsX = static_cast<int>(std::ceil(width / m_cellSize));
  m_cellsY = static_cast<int>(std::ceil(height / m_cellSize));
  m_cellsZ = static_cast<int>(std::ceil(depth / m_cellSize));

  if (m_cellsX == 0)
    m_cellsX = 1;
  if (m_cellsY == 0)
    m_cellsY = 1;
  if (m_cellsZ == 0)
    m_cellsZ = 1;

  size_t totalCells = static_cast<size_t>(m_cellsX * m_cellsY * m_cellsZ);
  m_grid.resize(totalCells);
  m_isCellDirty.resize(totalCells, false);

  for (size_t i = 0; i < totalCells; ++i) {
    m_grid[i].reserve(RESERVE_PER_CELL);
  }
}

glm::ivec3 SpatialGrid::getCellCoords(const glm::vec3 &pos) {
  int cellX = static_cast<int>(pos.x / m_cellSize);
  int cellY = static_cast<int>(pos.y / m_cellSize);
  int cellZ = static_cast<int>(pos.z / m_cellSize);
  return glm::ivec3(cellX, cellY, cellZ);
}

int SpatialGrid::get1DIndex(const glm::ivec3 &coords) {
  return coords.x + coords.y * m_cellsX + coords.z * m_cellsX * m_cellsY;
}

bool SpatialGrid::isValidCell(const glm::ivec3 &coords) {
  return coords.x >= 0 && coords.x < m_cellsX && coords.y >= 0 &&
         coords.y < m_cellsY && coords.z >= 0 && coords.z < m_cellsZ;
}

void SpatialGrid::insert(const std::unique_ptr<PhysicsObject> &object,
                         bool is3D) {
  glm::ivec3 coords = getCellCoords(object->position());
  if (!is3D) {
    coords.z = 0;
  }

  if (isValidCell(coords)) {
    int index = get1DIndex(coords);
    if (m_isCellDirty[index] == 0) {
      m_isCellDirty[index] = 1;
      m_dirtyCellIndices.push_back(index);
    }
    m_grid[index].push_back(object.get());
  }
}

void SpatialGrid::clear() {
  for (int index : m_dirtyCellIndices) {
    m_grid[index].clear();
    m_isCellDirty[index] = 0;
  }
  m_dirtyCellIndices.clear();
}
