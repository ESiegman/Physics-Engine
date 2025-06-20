#include "../include/PhysicsObject.hpp"
#include "../include/Constants.hpp"
#include <glm/gtc/random.hpp>
#include <random>

PhysicsObject::PhysicsObject(bool is3D, float radius, float mass) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> x_rand(0, Constants::WORLD_WIDTH);
  std::uniform_real_distribution<float> y_rand(0, Constants::WORLD_HEIGHT);
  std::uniform_real_distribution<float> z_rand(0, Constants::WORLD_DEPTH);
  std::uniform_real_distribution<float> vel_rand(Constants::OBJECT_MIN_VEL, Constants::OBJECT_MAX_VEL);

  m_pos =
      glm::vec3(x_rand(gen), y_rand(gen), is3D ? z_rand(gen) : 0.0f);
  m_rad = radius;
  m_mass = mass;
  m_vel = glm::vec3(vel_rand(gen), vel_rand(gen),
                    is3D ? vel_rand(gen) : 0.0f);
  
  std::uniform_real_distribution<float> color_rand(0.0f, 1.0f);
  m_color = glm::vec3(color_rand(gen), color_rand(gen), color_rand(gen));
}

void PhysicsObject::update(float dt, bool is3D, float gravity) {
  m_vel.y += gravity * dt;
  m_pos += m_vel * dt;
  preventBorderCollision(is3D);
}

void PhysicsObject::preventBorderCollision(bool is3D) {
  float damping = 0.8f;

  if (m_pos.x + m_rad > Constants::WORLD_WIDTH) {
    m_pos.x = Constants::WORLD_WIDTH - m_rad;
    m_vel.x *= -damping;
  } else if (m_pos.x - m_rad < 0) {
    m_pos.x = m_rad;
    m_vel.x *= -damping;
  }
  if (m_pos.y + m_rad > Constants::WORLD_HEIGHT) {
    m_pos.y = Constants::WORLD_HEIGHT - m_rad;
    m_vel.y *= -damping;
  } else if (m_pos.y - m_rad < 0) {
    m_pos.y = m_rad;
    m_vel.y *= -damping;
  }

  if (is3D) {
    if (m_pos.z + m_rad > Constants::WORLD_DEPTH) {
      m_pos.z = Constants::WORLD_DEPTH - m_rad;
      m_vel.z *= -damping;
    } else if (m_pos.z - m_rad < 0) {
      m_pos.z = m_rad;
      m_vel.z *= -damping;
    }
  }
}

void collision(PhysicsObject &o1, PhysicsObject &o2, bool is3D, float restitution) {
  glm::vec3 pos1 = o1.position();
  glm::vec3 pos2 = o2.position();
  if (!is3D) {
    pos1.z = 0;
    pos2.z = 0;
  }

  float distance = glm::distance(pos1, pos2);
  float sumRadii = o1.radius() + o2.radius();

  if (distance > sumRadii || distance < 1e-6f) {
    return;
  }

  std::lock(o1.m_mutex, o2.m_mutex);
  std::lock_guard<std::mutex> lock1(o1.m_mutex, std::adopt_lock);
  std::lock_guard<std::mutex> lock2(o2.m_mutex, std::adopt_lock);

  glm::vec3 normal = glm::normalize(pos2 - pos1);
  glm::vec3 rel_vel = o2.velocity() - o1.velocity();
  float vel_along_normal = glm::dot(rel_vel, normal);

  if (vel_along_normal > 0) {
    return;
  }

  float overlap = sumRadii - distance;
  if (overlap > 0) {
    float total_inv_mass = (1.0f / o1.mass()) + (1.0f / o2.mass());
    float c1_correction_ratio = (1.0f / o1.mass()) / total_inv_mass;
    float c2_correction_ratio = (1.0f / o2.mass()) / total_inv_mass;
    o1.updatePos(-normal * overlap * c1_correction_ratio);
    o2.updatePos(normal * overlap * c2_correction_ratio);
  }

  float j = (-(1.0f + restitution) * vel_along_normal) /
            ((1.0f / o1.mass()) + (1.0f / o2.mass()));
  glm::vec3 impulse = j * normal;

  o1.updateVel(o1.velocity() - impulse / o1.mass());
  o2.updateVel(o2.velocity() + impulse / o2.mass());
}
