#pragma once

#include <GL/glew.h>
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
  GLuint ID;

  Shader(const char *vertexPath, const char *fragmentPath);

  void use();
  void setBool(const std::string &name, bool value) const;
  void setFloat(const std::string &name, float value) const;
  void setVec2(const std::string &name, const glm::vec2 &value) const;
  void setVec3(const std::string &name, const glm::vec3 &value) const;
  void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
  void checkCompileErrors(GLuint shader, std::string type);
};
