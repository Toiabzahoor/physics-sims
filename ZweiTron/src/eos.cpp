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
    double rest_mass_energy = rho * std::pow(Constants::c, 2);
    double internal_energy = pressure / (Gamma - 1.0);
    return rest_mass_energy + internal_energy;
}

PiecewisePolytropicEoS::PiecewisePolytropicEoS(double k_crust, double gamma_crust, double gamma_core, double rho_trans)
    : K_crust(k_crust), Gamma_crust(gamma_crust), Gamma_core(gamma_core), rho_transition(rho_trans) {
    double P_transition = K_crust * std::pow(rho_transition, Gamma_crust);
    K_core = P_transition / std::pow(rho_transition, Gamma_core);
    
    double c2 = std::pow(Constants::c, 2);
    //ensures enthalpy remains continous
    a_core = (P_transition / (rho_transition * c2)) * (1.0 / (Gamma_crust - 1.0) - 1.0 / (Gamma_core - 1.0));
}

double PiecewisePolytropicEoS::returnPressure(double density) const {
    if (density <= rho_transition) {
        return K_crust * std::pow(density, Gamma_crust); // Crust
    } else {
        return K_core * std::pow(density, Gamma_core);   // Core
    }
}

double PiecewisePolytropicEoS::returnDensity(double pressure) const {
    double P_transition = K_crust * std::pow(rho_transition, Gamma_crust);
    if (pressure <= P_transition) {
        return std::pow(pressure / K_crust, 1.0 / Gamma_crust);
    } else {
        return std::pow(pressure / K_core, 1.0 / Gamma_core);
    }
}

double PiecewisePolytropicEoS::returnEnergyDensity(double pressure) const {
    double rho = returnDensity(pressure);
    double rest_mass_energy = rho * std::pow(Constants::c, 2);
    double internal_energy = 0.0;
    double P_transition = K_crust * std::pow(rho_transition, Gamma_crust);
    
    if (pressure <= P_transition) {
        internal_energy = pressure / (Gamma_crust - 1.0);
    } else {
        //fixed a thermodynamics violation
        // integrating E. density 
        internal_energy = (pressure / (Gamma_core - 1.0)) + (a_core * rest_mass_energy);
    }
    
    return rest_mass_energy + internal_energy;
}