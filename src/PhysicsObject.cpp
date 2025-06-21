#include "../include/PhysicsObject.hpp"
#include <glm/gtc/random.hpp>
#include <random>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>

PhysicsObject::PhysicsObject(const SimulationConstants &constants, bool is3D,
                             float radius, float mass)
    : m_constants(constants) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> x_rand(0, m_constants.WORLD_WIDTH);
  std::uniform_real_distribution<float> y_rand(0, m_constants.WORLD_HEIGHT);
  std::uniform_real_distribution<float> z_rand(0, m_constants.WORLD_DEPTH);
  std::uniform_real_distribution<float> vel_rand(m_constants.OBJECT_MIN_VEL,
                                                 m_constants.OBJECT_MAX_VEL);

  m_pos = glm::vec3(x_rand(gen), y_rand(gen), is3D ? z_rand(gen) : 0.0f);
  m_rad = radius;
  m_mass = mass;
  m_vel = glm::vec3(vel_rand(gen), vel_rand(gen), is3D ? vel_rand(gen) : 0.0f);

  m_color = glm::vec3(1.0f, 1.0f, 1.0f);
}

void PhysicsObject::update(float dt) {
  m_vel.y += m_constants.GRAVITY * dt;
  m_pos += m_vel * dt;
  preventBorderCollision(m_constants.USE_3D);
}

void PhysicsObject::preventBorderCollision(bool is3D) {

  if (m_pos.x + m_rad > m_constants.WORLD_WIDTH) {
    m_pos.x = m_constants.WORLD_WIDTH - m_rad;
    m_vel.x *= -m_constants.VERTICAL_DAMPING;
  } else if (m_pos.x - m_rad < 0) {
    m_pos.x = m_rad;
    m_vel.x *= -m_constants.VERTICAL_DAMPING;
  }
  if (m_pos.y + m_rad > m_constants.WORLD_HEIGHT) {
    m_pos.y = m_constants.WORLD_HEIGHT - m_rad;
    m_vel.y *= -m_constants.VERTICAL_DAMPING;
  } else if (m_pos.y - m_rad < 0) {
    m_pos.y = m_rad;
    m_vel.y *= -m_constants.VERTICAL_DAMPING;
  }

  if (is3D) {
    if (m_pos.z + m_rad > m_constants.WORLD_DEPTH) {
      m_pos.z = m_constants.WORLD_DEPTH - m_rad;
      m_vel.z *= -m_constants.VERTICAL_DAMPING;
    } else if (m_pos.z - m_rad < 0) {
      m_pos.z = m_rad;
      m_vel.z *= -m_constants.VERTICAL_DAMPING;
    }
  }
}

void collision(PhysicsObject &o1, PhysicsObject &o2,
               const SimulationConstants &constants) {
  glm::vec3 deltaPos = o2.position() - o1.position();
  if (!constants.USE_3D) {
    deltaPos.z = 0;
  }

  float distanceSq = glm::dot(deltaPos, deltaPos);
  float sumRadii = o1.radius() + o2.radius();
  float sumRadiiSq = sumRadii * sumRadii;

  if (distanceSq > sumRadiiSq) {
    return;
  }

  float distance = std::sqrt(distanceSq);

  if (distance < 1e-6f) {
    return;
  }

  glm::vec3 normal = deltaPos / distance;
  glm::vec3 rel_vel = o2.velocity() - o1.velocity();
  float vel_along_normal = glm::dot(rel_vel, normal);

  if (vel_along_normal > 0) {
    return;
  }

  std::lock(o1.m_mutex, o2.m_mutex);
  std::lock_guard<std::mutex> lock1(o1.m_mutex, std::adopt_lock);
  std::lock_guard<std::mutex> lock2(o2.m_mutex, std::adopt_lock);

  float overlap = sumRadii - distance;
  if (overlap > 0) {
    float total_inv_mass = (1.0f / o1.mass()) + (1.0f / o2.mass());
    float c1_correction_ratio = (1.0f / o1.mass()) / total_inv_mass;
    float c2_correction_ratio = (1.0f / o2.mass()) / total_inv_mass;
    o1.updatePos(-normal * overlap * c1_correction_ratio);
    o2.updatePos(normal * overlap * c2_correction_ratio);
  }

  float j =
      (-(1.0f + constants.COEFFICIENT_OF_RESTITUTION) * vel_along_normal) /
      ((1.0f / o1.mass()) + (1.0f / o2.mass()));
  glm::vec3 impulse = j * normal;

  o1.updateVel(o1.velocity() - impulse / o1.mass());
  o2.updateVel(o2.velocity() + impulse / o2.mass());
}
