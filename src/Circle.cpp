#include "../include/Circle.hpp"
#include <random>

const float GRAVITY = -980.0f;

Circle::Circle() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> xrand(0, 1920);
  std::uniform_int_distribution<> yrand(0, 1080);
  std::uniform_real_distribution<float> vel_rand(-1000.0f, 1000.0f);
  m_pos = {static_cast<float>(xrand(gen)), static_cast<float>(yrand(gen))};
  m_rad = 2.50f;
  m_mass = 25.0f;
  m_vel = {vel_rand(gen), vel_rand(gen)};
}

Circle::Circle(float radius, float x, float y, float mass, float xvec,
               float yvec) {
  m_rad = radius;
  m_pos = {x, y};
  m_mass = mass;
  m_vel = {xvec, yvec};
}

void Circle::update(float dt) {
  m_vel.second += GRAVITY * dt;
  m_pos.first += m_vel.first * dt;
  m_pos.second += m_vel.second * dt;
  preventBorderCollision();
}

void Circle::preventBorderCollision() {
  float worldWidth = 1920;
  float worldHeight = 1080;
  float x = m_pos.first;
  float y = m_pos.second;
  float vx = m_vel.first;
  float vy = m_vel.second;
  float damping = 0.8f;

  if (x + m_rad > worldWidth) {
    m_pos.first = worldWidth - m_rad;
    m_vel.first = -vx * damping;
  } else if (x - m_rad < 0) {
    m_pos.first = m_rad;
    m_vel.first = -vx * damping;
  }
  if (y + m_rad > worldHeight) {
    m_pos.second = worldHeight - m_rad;
    m_vel.second = -vy * damping;
  } else if (y - m_rad < 0) {
    m_pos.second = m_rad;
    m_vel.second = -vy * damping;
  }
}

void collision(Circle &c1, Circle &c2) {
  float dx = c2.position().first - c1.position().first;
  float dy = c2.position().second - c1.position().second;
  float distance_sq = dx * dx + dy * dy;
  float sumRadii = c1.radius() + c2.radius();

  if (distance_sq > sumRadii * sumRadii) {
    return;
  }

  std::lock(c1.m_mutex, c2.m_mutex);
  std::lock_guard<std::mutex> lock1(c1.m_mutex, std::adopt_lock);
  std::lock_guard<std::mutex> lock2(c2.m_mutex, std::adopt_lock);

  float distance = std::sqrt(distance_sq);

  if (distance < 1e-6f) {
    return;
  }

  float normal_x = dx / distance;
  float normal_y = dy / distance;

  float rel_vel_x = c2.velocity().first - c1.velocity().first;
  float rel_vel_y = c2.velocity().second - c1.velocity().second;
  float vel_along_normal = rel_vel_x * normal_x + rel_vel_y * normal_y;

  if (vel_along_normal > 0) {
    return;
  }

  float overlap = sumRadii - distance;
  if (overlap > 0) {
    float total_inv_mass = (1.0f / c1.mass()) + (1.0f / c2.mass());
    float c1_correction_ratio = (1.0f / c1.mass()) / total_inv_mass;
    float c2_correction_ratio = (1.0f / c2.mass()) / total_inv_mass;

    c1.updatePos(-normal_x * overlap * c1_correction_ratio,
                 -normal_y * overlap * c1_correction_ratio);
    c2.updatePos(normal_x * overlap * c2_correction_ratio,
                 normal_y * overlap * c2_correction_ratio);
  }

  float e = 0.95f;
  float j = (-(1.0f + e) * vel_along_normal) /
            ((1.0f / c1.mass()) + (1.0f / c2.mass()));
  float impulse_x = j * normal_x;
  float impulse_y = j * normal_y;

  c1.updateVel(c1.velocity().first - impulse_x / c1.mass(),
               c1.velocity().second - impulse_y / c1.mass());
  c2.updateVel(c2.velocity().first + impulse_x / c2.mass(),
               c2.velocity().second + impulse_y / c2.mass());
}
