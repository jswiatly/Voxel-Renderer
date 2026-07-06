#include "scene/Sky.hpp"

#include <glm/gtc/constants.hpp>
#include <cmath>

glm::vec4 getSkyColor(float timeOfDay) {
    for (size_t i{}; i + 1 < SKY_KEYFRAMES.size(); ++i) {
        const auto& a = SKY_KEYFRAMES[i];
        const auto& b = SKY_KEYFRAMES[i + 1];
        if (timeOfDay >= a.timeOfDay && timeOfDay <= b.timeOfDay) {
            float t = (timeOfDay - a.timeOfDay) / (b.timeOfDay - a.timeOfDay);
            return glm::mix(a.color, b.color, t);
        }
    }
    return SKY_KEYFRAMES.back().color;
}

glm::vec3 getSunDirection(float timeOfDay) {
    float theta = (timeOfDay - 0.25f) * glm::two_pi<float>();
    return glm::normalize(glm::vec3(std::cos(theta), std::sin(theta), 0.5f));
}

glm::vec3 getSunColor(float timeOfDay) {
    float sunY = getSunDirection(timeOfDay).y;
    float height = glm::clamp(sunY, 0.0f, 1.0f);
    glm::vec3 horizon = glm::vec3(1.0f, 0.5f, 0.25f);
    glm::vec3 zenith = glm::vec3(1.0f, 0.97f, 0.9f);
    glm::vec3 day = glm::mix(horizon, zenith, height);

    float daylight = glm::smoothstep(-0.2f, 0.0f, sunY);
    glm::vec3 night = glm::vec3(0.02f, 0.03f, 0.06f);
    return glm::mix(night, day, daylight);
}
