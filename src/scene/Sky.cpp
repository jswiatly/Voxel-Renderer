#include "scene/Sky.hpp"

glm::vec4 getSkyColor(float timeOfDay){
    for (size_t i{}; i + 1 < SKY_KEYFRAMES.size(); ++i){
        const auto& a = SKY_KEYFRAMES[i];
        const auto& b = SKY_KEYFRAMES[i + 1];
        if (timeOfDay >= a.timeOfDay && timeOfDay <= b.timeOfDay){
            float t = (timeOfDay - a.timeOfDay) / (b.timeOfDay - a.timeOfDay);
            return glm::mix(a.color, b.color, t);
        }
    }
    return SKY_KEYFRAMES.back().color;
}