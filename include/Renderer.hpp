#pragma once

#include "Shader.hpp"
#include <glm/glm.hpp>

class Renderer {
public:
  Renderer();
  ~Renderer();

  void renderObject(glm::vec3 center, float radius, glm::vec3 color,
                    float windowWidth, float windowHeight, bool is3D,
                    const glm::mat4 &view, const glm::mat4 &projection);

  void renderContainerBox(const glm::mat4 &view, const glm::mat4 &projection);
  void renderFloor(const glm::mat4 &view, const glm::mat4 &projection);

private:
  GLuint m_quadVAO, m_quadVBO;
  Shader *m_objectShader;

  GLuint m_boxVAO, m_boxVBO;
  Shader *m_lineShader;

  GLuint m_floorVAO, m_floorVBO;
  Shader *m_floorShader;

  GLuint m_sphereVAO, m_sphereVBO, m_sphereEBO;
  unsigned int m_sphereIndexCount;
};
