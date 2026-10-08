#include "tools/Benchmark.hpp"

#include <chrono>
#include <iostream>
#include <algorithm>
#include <numeric>

Benchmark::Benchmark(std::string name, int iterations) : m_name(std::move(name)), m_iterations(iterations) {}

void Benchmark::run(const TestFunction& function) {
    using Clock = std::chrono::steady_clock;

    std::vector<double> times;
    times.reserve(m_iterations);

    for (int i = 0; i < m_iterations; ++i) {
        const auto start = Clock::now();

        function();

        const auto end = Clock::now();

        const double elapsed = std::chrono::duration<double, std::milli>(end - start).count();

        times.push_back(elapsed);
    }

    const double min = *std::min_element(times.begin(), times.end());

    const double max = *std::max_element(times.begin(), times.end());

    const double average = std::accumulate(times.begin(), times.end(), 0.0) / times.size();

    std::cout << '\n';
    std::cout << "=== Benchmark: " << m_name << " ===\n";
    std::cout << "Iterations: " << m_iterations << '\n';
    std::cout << "Min:        " << min << " ms\n";
    std::cout << "Average:    " << average << " ms\n";
    std::cout << "Max:        " << max << " ms\n";
}