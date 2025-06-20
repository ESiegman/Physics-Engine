#pragma once

#include "PhysicsObject.hpp"
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>

struct TupleHasher {
  size_t operator()(const std::tuple<int, int, int> &k) const {
    return std::get<0>(k) ^ std::get<1>(k) ^ std::get<2>(k);
  }
};

class SpatialGrid {
public:
  SpatialGrid(float width, float height, float depth, float cellSize);
  void insert(const std::unique_ptr<PhysicsObject> &object, bool is3D);
  std::vector<PhysicsObject *>
  getPotentialColliders(const std::unique_ptr<PhysicsObject> &object,
                        bool is3D);
  void clear();

private:
  std::tuple<int, int, int> getCellCoords(const glm::vec3 &pos);
  float m_cellSize;
  std::unordered_map<std::tuple<int, int, int>, std::vector<PhysicsObject *>,
                     TupleHasher>
      m_grid;
};
