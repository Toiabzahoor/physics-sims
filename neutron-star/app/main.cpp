#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include "../include/constants.hpp"
#include "../include/eos.hpp"
#include "../include/tov.hpp"
#include "../include/rk4.hpp"

int main() {
    // setting the eos and using test values for k and gamma
    double K = 0.025;   
    double Gamma = 2.0;
    PolytropicEoS eos(K, Gamma);

    // initialize tov
    TOVSystem tov(eos);

    // binding the tov derivative so rk4 can use it
    RK4::DerivativeFunc derivs = [&tov](double r, const std::vector<double>& y) {
        return tov.getDerivatives(r, y);
    };

    // starting at very small because starting from zero would introduce 0 in division
    double r = 1.0; // 1 meter from the center
    double dr = 10.0; // 10 meter steps
    
    // central density
    double central_density = Constants::rho_nuc_si * 2.0;
    double central_pressure = eos.returnPressure(central_density);
    
    // initial state vector: y = {Pressure, Mass}
    std::vector<double> y = {central_pressure, 0.0}; 

    //prepare output file
    std::ofstream outfile("data/star_profile.csv");
    outfile << "Radius_m,Pressure_Pa,Mass_kg\n";
    outfile << r << "," << y[0] << "," << y[1] << "\n";

    std::cout << "Integrating TOV equations outward..." << std::endl;

     //loop continues until the pressure drops to 0 
    while (y[0] > 0.0) {
        y = RK4::step(r, y, dr, derivs);
        r += dr;

        // Save data every 100 steps to keep the file size reasonable
        static int step_count = 0;
        if (step_count++ % 100 == 0 && y[0] > 0.0) {
            outfile << r << "," << y[0] << "," << y[1] << "\n";
        }
    }

    outfile.close();

    // 6. Final Results
    double final_radius_km = r / Constants::km_to_m;
    double final_mass_solar = y[1] / Constants::M_sun;

    std::cout << "Simulation Complete!" << std::endl;
    std::cout << "Star Radius: " << final_radius_km << " km" << std::endl;
    std::cout << "Star Mass:   " << final_mass_solar << " Solar Masses" << std::endl;

    return 0;
}