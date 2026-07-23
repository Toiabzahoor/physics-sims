#include "../include/tov.hpp"
#include "../include/constants.hpp"
#include <cmath>

TOVSystem::TOVSystem(const EquationOfState& eos_in) : eos(eos_in) {}

std::vector<double> TOVSystem::getDerivatives(double r, const std::vector<double>& y) const {
    double P = y[0];
    double M = y[1];

    // The surface of the star where pressure hits zero. 
    // overshooting shall be frozen
    if (P <= 0.0) {
        return {0.0, 0.0};
    }

    // To prevent division by zero at the exact center (r = 0)
    if (r <= 1e-10) {
        return {0.0, 0.0};
    }

    double epsilon = eos.returnEnergyDensity(P);
    
    double G = Constants::G;
    double c2 = std::pow(Constants::c, 2);

    // Mass derivative (dM/dr)
    double dM_dr = 4.0 * M_PI * std::pow(r, 2) * (epsilon / c2);

    // Pressure derivative (dP/dr)
    double term1 = epsilon + P;
    double term2 = M + (4.0 * M_PI * std::pow(r, 3) * P / c2);
    
    // The general relativity metric correction factor
    double gr_correction = 1.0 - ((2.0 * G * M) / (r * c2));
    double term3 = std::pow(r, 2) * gr_correction;

    double dP_dr = -(G / c2) * (term1 * term2) / term3;

    return {dP_dr, dM_dr};
}