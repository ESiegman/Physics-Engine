#include "../include/Camera.hpp"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch,
               const SimulationConstants &constants)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
      MovementSpeed(constants.CAMERA_MOVEMENT_SPEED),
      MouseSensitivity(constants.CAMERA_MOUSE_SENSITIVITY),
      Fov(constants.CAMERA_FOV), m_constants(constants) {
  Position = position;
  WorldUp = up;
  Yaw = yaw;
  Pitch = pitch;
  updateCameraVectors();
}
glm::mat4 Camera::getViewMatrix() {
  return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio) {
  return glm::perspective(glm::radians(Fov), aspectRatio, 10.0f, 150000.0f);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
  float velocity = MovementSpeed * deltaTime;
  if (direction == CameraMovement::FORWARD)
    Position += Front * velocity;
  if (direction == CameraMovement::BACKWARD)
    Position -= Front * velocity;
  if (direction == CameraMovement::LEFT)
    Position -= Right * velocity;
  if (direction == CameraMovement::RIGHT)
    Position += Right * velocity;
  if (direction == CameraMovement::UP)
    Position += WorldUp * velocity;
  if (direction == CameraMovement::DOWN)
    Position -= WorldUp * velocity;
}

void Camera::processMouseMovement(float xoffset, float yoffset,
                                  bool constrainPitch) {
  xoffset *= MouseSensitivity;
  yoffset *= MouseSensitivity;

  Yaw += xoffset;
  Pitch += yoffset;

  if (constrainPitch) {
    if (Pitch > 89.0f)
      Pitch = 89.0f;
    if (Pitch < -89.0f)
      Pitch = -89.0f;
  }

  updateCameraVectors();
}

void Camera::rotateBasedOnScreenPos(float xpos, float ypos, int screenWidth,
                                    int screenHeight, float deltaTime) {
  float centerX = screenWidth / 2.0f;
  float centerY = screenHeight / 2.0f;

  float x_diff = xpos - centerX;
  float y_diff = centerY - ypos;

  float dead_zone = 1.0f;
  if (glm::abs(x_diff) < dead_zone)
    x_diff = 0;
  if (glm::abs(y_diff) < dead_zone)
    y_diff = 0;

  float rotation_speed = 0.03f;
  Yaw += x_diff * rotation_speed * deltaTime;
  Pitch += y_diff * rotation_speed * deltaTime;

  if (Pitch > 89.0f)
    Pitch = 89.0f;
  if (Pitch < -89.0f)
    Pitch = -89.0f;

  updateCameraVectors();
}

void Camera::updateCameraVectors() {
  glm::vec3 front;
  front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  front.y = sin(glm::radians(Pitch));
  front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  Front = glm::normalize(front);

  Right = glm::normalize(glm::cross(Front, WorldUp));
  Up = glm::normalize(glm::cross(Right, Front));
}
