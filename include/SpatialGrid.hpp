#pragma once

#include "Constants.hpp"
#include "PhysicsObject.hpp"

#include <memory>
#include <vector>

class SpatialGrid {
public:
  SpatialGrid(float width, float height, float depth, float cellSize);
  void insert(const std::unique_ptr<PhysicsObject> &object, bool is3D);

  template <typename TCallback>
  void processPotentialColliders(const std::unique_ptr<PhysicsObject> &object,
                                 bool is3D, TCallback callback) {
    glm::ivec3 centerCoords = getCellCoords(object->position());

    int z_start = is3D ? -1 : 0;
    int z_end = is3D ? 1 : 0;

    for (int x_offset = -1; x_offset <= 1; ++x_offset) {
      for (int y_offset = -1; y_offset <= 1; ++y_offset) {
        for (int z_offset = z_start; z_offset <= z_end; ++z_offset) {
          glm::ivec3 neighborCoords = {centerCoords.x + x_offset,
                                       centerCoords.y + y_offset,
                                       centerCoords.z + z_offset};

          if (isValidCell(neighborCoords)) {
            int index = get1DIndex(neighborCoords);
            const std::vector<PhysicsObject *> &cellObjects = m_grid[index];
            for (PhysicsObject *other_object : cellObjects) {
              callback(other_object);
            }
          }
        }
      }
    }
  }

  void clear();

  const std::vector<PhysicsObject *> &
  getInternalCellObjects(glm::ivec3 coords) {
    if (!isValidCell(coords)) {
      static const std::vector<PhysicsObject *> emptyVec;
      return emptyVec;
    }
    return m_grid[get1DIndex(coords)];
  }

private:
  glm::ivec3 getCellCoords(const glm::vec3 &pos);
  int get1DIndex(const glm::ivec3 &coords);
  bool isValidCell(const glm::ivec3 &coords);

  float m_cellSize;
  int m_cellsX, m_cellsY, m_cellsZ;
  std::vector<std::vector<PhysicsObject *>> m_grid;
  std::vector<int> m_populatedCellIndices;
  std::vector<int> m_dirtyCellIndices;
  std::vector<char> m_isCellDirty;
};
