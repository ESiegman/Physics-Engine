#pragma once

#include "Constants.hpp"

#include <glm/glm.hpp>
#include <mutex>

class PhysicsObject {
public:
  PhysicsObject(const SimulationConstants &constants, bool is3D, float radius,
                float mass);

  void update(float dt);
  void preventBorderCollision(bool is3D);

  const glm::vec3 &position() const { return m_pos; }
  const glm::vec3 &velocity() const { return m_vel; }
  float radius() const { return m_rad; }
  float mass() const { return m_mass; }
  const glm::vec3 &color() const { return m_color; }

  void updatePos(const glm::vec3 &delta) { m_pos += delta; }
  void updateVel(const glm::vec3 &newVel) { m_vel = newVel; }

  mutable std::mutex m_mutex;

private:
  glm::vec3 m_pos;
  glm::vec3 m_vel;
  glm::vec3 m_color;
  float m_rad;
  float m_mass;
  const SimulationConstants &m_constants;
};

void collision(PhysicsObject &o1, PhysicsObject &o2,
               const SimulationConstants &constants);
