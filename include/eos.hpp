#pragma once

class EquationOfState {
public:
    virtual ~EquationOfState() = default;
    virtual double returnPressure(double density) const = 0;
    virtual double returnDensity(double pressure) const = 0;
    virtual double returnEnergyDensity(double pressure) const = 0;
};

class PolytropicEoS : public EquationOfState {
private:
    double K;
    double Gamma;
public:
    PolytropicEoS(double k_val, double gamma_val);
    double returnPressure(double density) const override;
    double returnDensity(double pressure) const override;
    double returnEnergyDensity(double pressure) const override;
};

// added piecewise for more accurate crust vs code modelling
class PiecewisePolytropicEoS : public EquationOfState {
private:
    double K_crust;
    double Gamma_crust;
    double Gamma_core;
    double rho_transition;
    double K_core; 
    double a_core; 
public:
    PiecewisePolytropicEoS(double k_crust, double gamma_crust, double gamma_core, double rho_trans);
    double returnPressure(double density) const override;
    double returnDensity(double pressure) const override;
    double returnEnergyDensity(double pressure) const override;
};