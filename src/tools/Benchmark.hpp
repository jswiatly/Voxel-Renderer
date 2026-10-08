#pragma once

#include <string>
#include <vector>
#include <functional>

class Benchmark {
  public:
    using TestFunction = std::function<void()>;

    Benchmark(std::string name, int iterations);

    void run(const TestFunction& function);

  private:
    std::string m_name;
    int m_iterations;
};