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