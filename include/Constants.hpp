#pragma once

#include <cstddef>

namespace Constants {
constexpr bool USE_3D = true;

constexpr float WORLD_WIDTH = 1920.0f;
constexpr float WORLD_HEIGHT = 1080.0f;
constexpr float WORLD_DEPTH = 1080.0f;

constexpr size_t NUM_OBJECTS = 2000;
constexpr float CELL_SIZE_3D = 50.0f;
constexpr float CELL_SIZE_2D = 20.0f;
constexpr float FIXED_DELTA_TIME = 0.01f;
constexpr int PHYSICS_ITERATIONS = 10;

constexpr float GRAVITY = -980.0f;
constexpr float OBJECT_DEFAULT_RADIUS = 10.0f;
constexpr float OBJECT_DEFAULT_MASS = 25.0f;
constexpr float OBJECT_MIN_VEL = -500.0f;
constexpr float OBJECT_MAX_VEL = 500.0f;
constexpr float COEFFICIENT_OF_RESTITUTION = 0.95f;

constexpr float CAMERA_MOVEMENT_SPEED = 1500.0f;
constexpr float CAMERA_MOUSE_SENSITIVITY = 0.1f;
constexpr float CAMERA_FOV = 45.0f;
} // namespace Constants
