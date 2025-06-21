#pragma once

#include "Camera.hpp"
#include "Constants.hpp"
#include "GUI.hpp"
#include "PhysicsObject.hpp"
#include "Shader.hpp"
#include "SpatialGrid.hpp"
#include "Window.hpp"

#include <cstddef>
#include <functional>
#include <future>
#include <glm/glm.hpp>
#include <memory>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

class ThreadPool {
public:
  ThreadPool(size_t threads) : stop(false) {
    if (threads == 0) {
      threads = 1;
    }
    for (size_t i = 0; i < threads; ++i) {
      workers.emplace_back([this] {
        for (;;) {
          std::function<void()> task;
          {
            std::unique_lock<std::mutex> lock(this->queue_mutex);
            this->condition.wait(
                lock, [this] { return this->stop || !this->tasks.empty(); });
            if (this->stop && this->tasks.empty())
              return;
            task = std::move(this->tasks.front());
            this->tasks.pop();
          }
          task();
        }
      });
    }
  }

  template <class F, class... Args>
  auto enqueue(F &&f, Args &&...args)
      -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      if (stop)
        throw std::runtime_error("enqueue on stopped ThreadPool");
      tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
  }

  size_t getNumThreads() const { return workers.size(); }

  ~ThreadPool() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex);
      stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers)
      worker.join();
  }

private:
  std::vector<std::thread> workers;
  std::queue<std::function<void()>> tasks;
  std::mutex queue_mutex;
  std::condition_variable condition;
  bool stop;
};

struct PointLight {
  glm::vec3 position;
  glm::vec3 color;
  float intensity;
};

struct GpuGridCell {
  unsigned int objectStartIndex;
  unsigned int objectCount;
};

class Simulation {
public:
  SimulationConstants m_constants;

  Simulation();
  ~Simulation();

  void run();
  void restart();

  void notifyWorldDimensionsChanged();

private:
  friend class GUI;

  Camera m_camera;
  Window m_window;
  std::vector<std::unique_ptr<PhysicsObject>> m_objects;
  SpatialGrid m_grid;
  GUI m_gui;
  glm::ivec2 m_debugPixel = glm::ivec2(960, 540);
  bool m_worldDimensionsChanged = false;

  bool m_pendingRestart = false;     // New flag
  bool m_pendingWorldResize = false; // New flag

  Shader *m_raytracingComputeShader;
  std::vector<PointLight> m_pointLights;

  static void checkCollisionsForChunk(
      const std::vector<std::unique_ptr<PhysicsObject>> &objects,
      SpatialGrid &grid, size_t start_idx, size_t end_idx,
      const SimulationConstants &constants);
  GLuint m_fbo;
  GLuint m_fboTexture;
  GLuint m_rbo;
  int m_currentDisplayW;
  int m_currentDisplayH;
  GLuint m_objectSSBO;
  GLuint m_lightSSBO;
  GLuint m_gridCellsSSBO;
  GLuint m_objectIndicesSSBO;

  std::unique_ptr<ThreadPool> m_threadPool;

  void resizeGpuBuffers();
};
