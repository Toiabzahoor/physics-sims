#pragma once
#include <vector>
#include <functional>

namespace RK4 {
    // Defines the signature.
    // takes r and y to return a derivative
    using DerivativeFunc = std::function<std::vector<double>(double, const std::vector<double>&)>;

    // does one rk4 step and returns next y
    std::vector<double> step(double r, const std::vector<double>& y, double dr, const DerivativeFunc& derivs);
}