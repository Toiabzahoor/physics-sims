#pragma once
#include <cmath>

namespace Constants {
    constexpr double c = 2.99792458e8;         
    constexpr double G = 6.67430e-11;         
    constexpr double M_sun = 1.98847e30;
    
    constexpr double rho_nuc = 2.8e14;         //nuclear saturation density in g/cm
    constexpr double rho_nuc_si = rho_nuc * 1000.0; //in kgm

    constexpr double km_to_m = 1000.0;

}