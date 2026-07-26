#pragma once
#include <vector>
#include "eos.hpp"

class TOVSystem {
private:
    const EquationOfState& eos;

public:
    //injecting eos
    explicit TOVSystem(const EquationOfState& eos_in);

    //matching the signature of rk4::derivativefunc
    std::vector<double> getDerivatives(double r, const std::vector<double>& y) const;
};