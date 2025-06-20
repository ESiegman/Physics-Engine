#include "../include/Renderer.hpp"
#include "../include/Constants.hpp"
#include <GL/glew.h>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

const float PI = 3.14159265359f;

void generateUVSphere(std::vector<glm::vec3> &vertices,
                      std::vector<unsigned int> &indices, float radius,
                      unsigned int sectorCount, unsigned int stackCount) {
  vertices.clear();
  indices.clear();

  float x, y, z, xy;
  float sectorStep = 2 * PI / sectorCount;
  float stackStep = PI / stackCount;
  float sectorAngle, stackAngle;

  for (unsigned int i = 0; i <= stackCount; ++i) {
    stackAngle = PI / 2 - i * stackStep;
    xy = radius * cosf(stackAngle);
    z = radius * sinf(stackAngle);

    for (unsigned int j = 0; j <= sectorCount; ++j) {
      sectorAngle = j * sectorStep;
      x = xy * cosf(sectorAngle);
      y = xy * sinf(sectorAngle);
      vertices.push_back(glm::vec3(x, y, z));
    }
  }

  unsigned int k1, k2;
  for (unsigned int i = 0; i < stackCount; ++i) {
    k1 = i * (sectorCount + 1);
    k2 = k1 + sectorCount + 1;

    for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
      if (i != 0) {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }
      if (i != (stackCount - 1)) {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }
}

Renderer::Renderer(const SimulationConstants& constants)
  : m_constants(constants) {
  m_objectShader = new Shader("shaders/object.vert", "shaders/object.frag");
  m_lineShader = new Shader("shaders/line.vert", "shaders/line.frag");
  m_floorShader = new Shader("shaders/floor.vert", "shaders/floor.frag");

  glGenVertexArrays(1, &m_quadVAO);
  glGenBuffers(1, &m_quadVBO);
  glBindVertexArray(m_quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
  float quadVertices[] = {-0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f,
                          0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f};
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  std::vector<glm::vec3> sphereVertices;
  std::vector<unsigned int> sphereIndices;
  generateUVSphere(sphereVertices, sphereIndices, 1.0f, 36, 18);
  m_sphereIndexCount = sphereIndices.size();

  glGenVertexArrays(1, &m_sphereVAO);
  glGenBuffers(1, &m_sphereVBO);
  glGenBuffers(1, &m_sphereEBO);
  glBindVertexArray(m_sphereVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_sphereVBO);
  glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(glm::vec3),
               sphereVertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphereEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sphereIndices.size() * sizeof(unsigned int),
               sphereIndices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glEnableVertexAttribArray(0);

  float w = m_constants.WORLD_WIDTH;
  float h = m_constants.WORLD_HEIGHT;
  float d = m_constants.WORLD_DEPTH;

  std::vector<glm::vec3> boxVertices = {
      glm::vec3(0, 0, 0), glm::vec3(w, 0, 0), glm::vec3(0, 0, 0),
      glm::vec3(0, h, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, d),
      glm::vec3(w, 0, 0), glm::vec3(w, h, 0), glm::vec3(w, 0, 0),
      glm::vec3(w, 0, d), glm::vec3(0, h, 0), glm::vec3(w, h, 0),
      glm::vec3(0, h, 0), glm::vec3(0, h, d), glm::vec3(0, 0, d),
      glm::vec3(w, 0, d), glm::vec3(0, 0, d), glm::vec3(0, h, d),
      glm::vec3(w, h, d), glm::vec3(0, h, d), glm::vec3(w, h, d),
      glm::vec3(w, 0, d), glm::vec3(w, h, d), glm::vec3(w, h, 0)};

  glGenVertexArrays(1, &m_boxVAO);
  glGenBuffers(1, &m_boxVBO);
  glBindVertexArray(m_boxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_boxVBO);
  glBufferData(GL_ARRAY_BUFFER, boxVertices.size() * sizeof(glm::vec3),
               &boxVertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glEnableVertexAttribArray(0);

  float floorSize = 100000.0f;
  std::vector<glm::vec3> floorVertices = {
      glm::vec3(-floorSize, 0.0f, -floorSize),
      glm::vec3(floorSize, 0.0f, -floorSize),
      glm::vec3(floorSize, 0.0f, floorSize),
      glm::vec3(-floorSize, 0.0f, -floorSize),
      glm::vec3(floorSize, 0.0f, floorSize),
      glm::vec3(-floorSize, 0.0f, floorSize)};

  glGenVertexArrays(1, &m_floorVAO);
  glGenBuffers(1, &m_floorVBO);
  glBindVertexArray(m_floorVAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_floorVBO);
  glBufferData(GL_ARRAY_BUFFER, floorVertices.size() * sizeof(glm::vec3),
               &floorVertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

Renderer::~Renderer() {
  delete m_objectShader;
  delete m_lineShader;
  delete m_floorShader;

  glDeleteVertexArrays(1, &m_quadVAO);
  glDeleteBuffers(1, &m_quadVBO);

  glDeleteVertexArrays(1, &m_boxVAO);
  glDeleteBuffers(1, &m_boxVBO);

  glDeleteVertexArrays(1, &m_floorVAO);
  glDeleteBuffers(1, &m_floorVBO);

  glDeleteVertexArrays(1, &m_sphereVAO);
  glDeleteBuffers(1, &m_sphereVBO);
  glDeleteBuffers(1, &m_sphereEBO);
}

void Renderer::renderObject(glm::vec3 center, float radius, glm::vec3 color,
                            float windowWidth, float windowHeight, bool is3D,
                            const glm::mat4 &view,
                            const glm::mat4 &projection) {
  m_objectShader->use();
  m_objectShader->setMat4("projection", projection);
  m_objectShader->setMat4("view", view);
  m_objectShader->setVec3("u_color", color);

  if (is3D) {
    m_objectShader->setBool("u_isSphere", true);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, center);
    model = glm::scale(model, glm::vec3(radius));

    m_objectShader->setMat4("model", model);

    glBindVertexArray(m_sphereVAO);
    glDrawElements(GL_TRIANGLES, m_sphereIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
  } else {
    m_objectShader->setBool("u_isSphere", false);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(center.x, center.y, 0.0f));
    model = glm::scale(model, glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));

    m_objectShader->setMat4("model", model);

    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);
  }
}

void Renderer::renderContainerBox(const glm::mat4 &view,
                                  const glm::mat4 &projection) {
  m_lineShader->use();
  m_lineShader->setMat4("projection", projection);
  m_lineShader->setMat4("view", view);
  m_lineShader->setMat4("model", glm::mat4(1.0f));
  m_lineShader->setVec3("u_color", glm::vec3(0.4f, 0.4f, 0.4f));

  glLineWidth(2.0f);
  glBindVertexArray(m_boxVAO);
  glDrawArrays(GL_LINES, 0, 24);
  glBindVertexArray(0);
}

void Renderer::renderFloor(const glm::mat4 &view, const glm::mat4 &projection) {
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(1.0f, 1.0f);

  m_floorShader->use();
  m_floorShader->setMat4("projection", projection);
  m_floorShader->setMat4("view", view);
  m_floorShader->setMat4("model", glm::mat4(1.0f));
  m_floorShader->setVec3("u_color", glm::vec3(0.2f, 0.2f, 0.2f));

  glBindVertexArray(m_floorVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);

  glDisable(GL_POLYGON_OFFSET_FILL);
}
