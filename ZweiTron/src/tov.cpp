#include "../include/tov.hpp"
#include "../include/constants.hpp"
#include <cmath>

TOVSystem::TOVSystem(const EquationOfState& eos_in) : eos(eos_in) {}

std::vector<double> TOVSystem::getDerivatives(double r, const std::vector<double>& y) const {
    double P = y[0];
    double M = y[1];

    //removed the hacky singularity fix.
    double epsilon = (P > 0.0) ? eos.returnEnergyDensity(P) : 0.0;
    
    double G = Constants::G;
    double c2 = std::pow(Constants::c, 2);

    // Mass derivative (dM/dr)
    double dM_dr = 4.0 * M_PI * std::pow(r, 2) * (epsilon / c2);

    // Pressure derivative (dP/dr)
    double term1 = epsilon + P;
    double term2 = M + (4.0 * M_PI * std::pow(r, 3) * P / c2);
    
    // The general relativity metric correction factor
    double gr_correction = 1.0 - ((2.0 * G * M) / (r * c2));
    
    if (gr_correction <= 0.0 || r <= 0.0) {
        return {0.0, 0.0}; 
    }

    double term3 = std::pow(r, 2) * gr_correction;
    double dP_dr = -(G / c2) * (term1 * term2) / term3;

    return {dP_dr, dM_dr};
}