#ifndef GUI_HPP
#define GUI_HPP

#include <string>
#include <vector>

#include <GL/glew.h>

class GLFWwindow;
class Simulation;

class GUI {
public:
  void init(GLFWwindow *window);
  void render(Simulation &sim, GLuint sceneTexture, int display_w,
              int display_h);
  void shutdown();

private:
  bool m_firstTime = true;
  bool m_showCameraControlsWindow = false;
};

#endif
