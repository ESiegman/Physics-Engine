#pragma once

#include "Camera.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window {
public:
  Window(int width, int height, const char *title, Camera *camera, bool is3D);
  ~Window();

  bool shouldClose();
  void processInput(float deltaTime);
  void swapBuffersAndPollEvents();

  GLFWwindow *getGlfwWindow() const { return m_glfwWindow; }

private:
  GLFWwindow *m_glfwWindow;
  Camera *m_camera;
  bool m_is3D;
  bool m_cameraLocked = false;
  int m_lastCtrlState = GLFW_RELEASE;

  double m_mouseX = 0.0;
  double m_mouseY = 0.0;

  void setupCallbacks();

  static void mouse_callback(GLFWwindow *window, double xpos, double ypos);
};
