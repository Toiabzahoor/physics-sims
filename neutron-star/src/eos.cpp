#include "../include/eos.hpp"
#include "../include/constants.hpp"
#include <cmath>


PolytropicEoS::PolytropicEoS(double k_val, double gamma_val) 
    : K(k_val), Gamma(gamma_val) {}

double PolytropicEoS::returnPressure(double density) const {
    return K * std::pow(density, Gamma);
}

double PolytropicEoS::returnDensity(double pressure) const {
    return std::pow(pressure / K, 1.0 / Gamma);
}

double PolytropicEoS::returnEnergyDensity(double pressure) const {
    double rho = returnDensity(pressure);
    
   // E density
    double rest_mass_energy = rho * std::pow(Constants::c, 2);
    double internal_energy = pressure / (Gamma - 1.0);
    
    return rest_mass_energy + internal_energy;
}