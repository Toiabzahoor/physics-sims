#include "../include/rk4.hpp"

namespace RK4 {
    std::vector<double> step(double r, const std::vector<double>& y, double dr, const DerivativeFunc& derivs) {
        size_t n = y.size();
        std::vector<double> y_next(n);
        std::vector<double> k1(n), k2(n), k3(n), k4(n), temp(n);

        // slope at the beginning 
        k1 = derivs(r, y);
        for (size_t i = 0; i < n; ++i) {
            k1[i] *= dr;
            temp[i] = y[i] + 0.5 * k1[i];
        }

        // slope at the mid
        k2 = derivs(r + 0.5 * dr, temp);
        for (size_t i = 0; i < n; ++i) {
            k2[i] *= dr;
            temp[i] = y[i] + 0.5 * k2[i];
        }

        // slope at the midpoint again
        k3 = derivs(r + 0.5 * dr, temp);
        for (size_t i = 0; i < n; ++i) {
            k3[i] *= dr;
            temp[i] = y[i] + k3[i];
        }

        // slope at end
        k4 = derivs(r + dr, temp);
        for (size_t i = 0; i < n; ++i) {
            k4[i] *= dr;
            
            // Calculating the final weighted avg for the next step
            y_next[i] = y[i] + (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) / 6.0;
        }

        return y_next;
    }
}