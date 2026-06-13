#include "Time.hpp"

namespace vkr {

Time::Time() {
    reset();
}

void Time::reset() {
    m_lastFrameTime = Clock::now();
    m_deltaTime = 0.0f;
    m_realTimeSeconds = 0.0;
    m_gameTimeSeconds = 0.0;
}

void Time::update() {
    const TimePoint now = Clock::now();
    const double delta = std::chrono::duration<double>(now - m_lastFrameTime).count();
    m_deltaTime = static_cast<float>(delta);
    m_realTimeSeconds += delta;
    m_gameTimeSeconds += delta * GAME_TIME_SCALE;
    m_lastFrameTime = now;
}
} // namespace vkr
