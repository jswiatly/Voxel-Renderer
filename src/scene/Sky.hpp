#pragma once

#include <array>
#include <glm/glm.hpp>

struct SkyKeyFrame {
    float timeOfDay;
    glm::vec4 color;
};

inline const std::array<SkyKeyFrame, 7> SKY_KEYFRAMES = {{
    {0.00f, {0.02f, 0.02f, 0.08f, 1.0f}},
    {0.25f, {0.15f, 0.10f, 0.25f, 1.0f}},
    {0.30f, {0.95f, 0.55f, 0.30f, 1.0f}},
    {0.50f, {0.45f, 0.72f, 0.95f, 1.0f}},
    {0.75f, {0.95f, 0.45f, 0.25f, 1.0f}},
    {0.80f, {0.20f, 0.15f, 0.35f, 1.0f}},
    {1.00f, {0.02f, 0.02f, 0.08f, 1.0f}},
}};

glm::vec4 getSkyColor(float timeOfDay);
glm::vec3 getSunDirection(float timeOfDay);
glm::vec3 getSunColor(float timeOfDay);