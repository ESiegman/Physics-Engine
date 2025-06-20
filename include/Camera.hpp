#pragma once

#include "Constants.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement { FORWARD, BACKWARD, LEFT, RIGHT, UP, DOWN };

class Camera {
public:
  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  glm::vec3 WorldUp;

  float Yaw;
  float Pitch;

  float MovementSpeed;
  float MouseSensitivity;
  float Fov;

  Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch,
         const SimulationConstants &constants);

  glm::mat4 getViewMatrix();
  glm::mat4 getProjectionMatrix(float aspectRatio);

  void processKeyboard(CameraMovement direction, float deltaTime);
  void processMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true);
  void rotateBasedOnScreenPos(float xpos, float ypos, int screenWidth,
                              int screenHeight, float deltaTime);

private:
  void updateCameraVectors();
  const SimulationConstants &m_constants;
};
