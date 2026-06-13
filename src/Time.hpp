#pragma once

#include <chrono>

namespace vkr {

class Time {
  public:
    Time();

    void update();

    void reset();
    // pos += velocity * dt.
    float getDeltaTime() const {
        return m_deltaTime;
    }
    double getRealTimeSeconds() const {
        return m_realTimeSeconds;
    }
    double getGameTimeSeconds() const {
        return m_gameTimeSeconds;
    }
    double getGameTimeMinutes() const {
        return m_gameTimeSeconds / 60.0;
    }
    static constexpr double GAME_TIME_SCALE = 60.0;

  private:
    using Clock = std::chrono::steady_clock;
    using TimePoint = Clock::time_point;
    TimePoint m_lastFrameTime;
    float m_deltaTime = 0.0f;
    double m_realTimeSeconds = 0.0;
    double m_gameTimeSeconds = 0.0;
};

} // namespace vkr
