#pragma once

#include <cmath>
#include <mutex>
#include <random>
#include <utility>
#include <vector>

class Circle {
private:
  float m_rad;
  std::pair<float, float> m_pos;
  float m_mass;
  std::pair<float, float> m_vel;
  std::mutex m_mutex;

public:
  Circle();
  Circle(float radius, float x, float y, float mass, float xvec, float yvec);

  void update(float dt);

  template <typename Iterator>
  void calcUpdate(Iterator listBegin, Iterator listEnd) {
    for (Iterator it = listBegin; it != listEnd; ++it) {
      collision(*this, *it);
    }
  }

  void preventBorderCollision();

  void updatePos(float dx, float dy) {
    m_pos.first += dx;
    m_pos.second += dy;
  }

  void updateVel(float vecx, float vecy) { m_vel = {vecx, vecy}; }

  const std::pair<float, float> &position() const { return m_pos; }
  float radius() const { return m_rad; }
  float mass() const { return m_mass; }
  const std::pair<float, float> &velocity() const { return m_vel; }

  friend void collision(Circle &c1, Circle &c2);
};

void collision(Circle &c1, Circle &c2);
