#include "../include/Window.hpp"
#include <iostream>

Window::Window(int width, int height, const char *title, Camera *camera,
               bool is3D)
    : m_camera(camera), m_is3D(is3D) {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  m_glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
  if (!m_glfwWindow) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(m_glfwWindow);

  glewInit();
  glEnable(GL_DEPTH_TEST);

  setupCallbacks();
}

Window::~Window() {
  glfwDestroyWindow(m_glfwWindow);
  glfwTerminate();
}

void Window::setupCallbacks() {
  glfwSetWindowUserPointer(m_glfwWindow, this);

  if (m_is3D) {
    glfwSetInputMode(m_glfwWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetCursorPosCallback(m_glfwWindow, mouse_callback);
  }
}

void Window::mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  Window *thisWindow = static_cast<Window *>(glfwGetWindowUserPointer(window));
  if (thisWindow) {
    thisWindow->m_mouseX = xpos;
    thisWindow->m_mouseY = ypos;
  }
}

bool Window::shouldClose() { return glfwWindowShouldClose(m_glfwWindow); }

void Window::processInput(float deltaTime) {
  if (glfwGetKey(m_glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(m_glfwWindow, true);

  int currentCtrlState = glfwGetKey(m_glfwWindow, GLFW_KEY_LEFT_CONTROL);
  if (currentCtrlState == GLFW_PRESS && m_lastCtrlState == GLFW_RELEASE) {
    m_cameraLocked = !m_cameraLocked;
  }
  m_lastCtrlState = currentCtrlState;

  if (m_is3D && m_camera && !m_cameraLocked) {
    if (glfwGetKey(m_glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
      m_camera->processKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(m_glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
      m_camera->processKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(m_glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
      m_camera->processKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(m_glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
      m_camera->processKeyboard(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(m_glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
      m_camera->processKeyboard(CameraMovement::UP, deltaTime);
    if (glfwGetKey(m_glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      m_camera->processKeyboard(CameraMovement::DOWN, deltaTime);

    int width, height;
    glfwGetWindowSize(m_glfwWindow, &width, &height);
    m_camera->rotateBasedOnScreenPos(m_mouseX, m_mouseY, width, height,
                                     deltaTime);
  }
}

void Window::swapBuffersAndPollEvents() {
  glfwSwapBuffers(m_glfwWindow);
  glfwPollEvents();
}
