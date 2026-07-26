# Astrophysics Simulator: Neutron Stars → Black Holes

An interactive real-time simulation that lets you **dial up the density of a star** and watch it transform from a normal star into a rapidly spinning neutron star, then collapse past the TOV limit into a black hole with an accretion disk.

Built with C++ and raylib (OpenGL 3.3+), with general relativity baked into both the physics solver and the visual rendering.

---

## What You Can Do

Press **W** and **S** to increase or decrease the central density of the star (in units of nuclear saturation density — that's ~2.8×10¹⁴ g/cm³). The simulation solves the full general relativistic structure equations in real time and updates the visuals accordingly.

- **Low density**: A normal-ish star
- **~1× nuclear density**: A typical neutron star — ~1.4 solar masses, ~12 km radius, spinning at millisecond periods, with relativistic polar jets
- **Above ~1.5× nuclear density** (mass > 2.1 M☉): Gravitational collapse — the star shrinks, the Schwarzschild radius appears, and the scene transitions to a black hole with a glowing accretion disk

---

## The Physics, Explained

### 1. The TOV Equations (How Stars Hold Themselves Up)

Inside any star, gravity pulls inward and pressure pushes outward. In general relativity, this balance is described by the **Tolman–Oppenheimer–Volkoff (TOV) equations**:

```
dP/dr = - (G / c²) × (energy_density + pressure) × (mass + 4πr³ × pressure / c²) / (r² × (1 - 2GM / (rc²)))
dM/dr = 4πr² × energy_density / c²
```

The first equation says: the pressure gradient depends on three things — how much mass-energy is inside (gravity's source), the pressure itself (which also gravitates in GR!), and a **metric correction** term `(1 - 2GM/rc²)` that accounts for how spacetime curvature amplifies gravity. This correction is what makes neutron stars different from Newtonian stars — as you approach the Schwarzschild radius, this term goes to zero, and the pressure gradient blows up.

The code integrates these equations outward from the center using a **4th-order Runge-Kutta** method, stepping until the pressure drops to zero (the star's surface). The initial conditions near r=0 use a Taylor expansion to avoid the coordinate singularity.

### 2. The Equation of State (What Neutron Star Matter Is Like)

To solve the TOV equations, you need a relationship between pressure, density, and energy — the **equation of state (EoS)**. This is the big unknown in neutron star physics.

This simulation uses a **piecewise polytropic EoS**:

- **Crust** (outer layers, lower density): `P = K_crust × ρ^Γ_crust` — simulates the solid crystalline lattice of neutron-rich nuclei embedded in a relativistic electron gas
- **Core** (inner region, higher density): `P = K_core × ρ^Γ_core` — simulates uniform nuclear matter dominated by neutron degeneracy pressure

The two regimes are stitched together at a transition density with **continuous enthalpy** — this prevents a thermodynamic discontinuity that would otherwise violate the first law of thermodynamics (the code comments this as "fixed a thermodynamics violation").

The energy density used in the TOV equations includes:
```
ε = ρc² + P / (Γ - 1)
```
This is rest-mass energy plus internal energy from compression, related through the adiabatic index Γ.

### 3. The TOV Limit (~2.1 M☉)

Neutron degeneracy pressure can only support so much mass. Beyond about **2.1 solar masses**, no stable neutron star solution exists — gravity overwhelms the strong nuclear force and degeneracy pressure. This is the **Tolman–Oppenheimer–Volkoff limit**, hard-coded as the collapse threshold.

When the user pushes past this limit, the simulation triggers a black hole transition.

### 4. Black Hole Physics

The black hole is rendered with a **full-screen GLSL shader that ray-marches through curved spacetime**:

**Ray tracing**: For each pixel, a ray is cast from the camera. At each step, the ray is deflected by a pseudo-relativistic gravity term:
```
acceleration = -1.5 × R_s × |angular_momentum|² / r⁵ × r_hat
```
This reproduces the correct **photon sphere at 1.5 R_s** — the radius where light orbits in a circle around the black hole.

**The black hole shadow**: Rays that pass within ~1.5 R_s get bent into the event horizon and never return, creating the dark central shadow. Rays that graze the photon sphere produce a bright ring (the "photon ring"), which is rendered as a Gaussian peak at 1.5 R_s.

**The accretion disk**: When a ray intersects the equatorial plane (y=0), it samples a procedurally generated disk with:
- **Temperature gradient**: White-hot near the inner edge (~2.5 R_s, just outside the ISCO at 3 R_s), cooling to red at the outer edge (14 R_s)
- **Doppler beaming**: Material orbiting at relativistic speeds appears brighter when moving toward the observer. The boost follows δ³ where δ is the relativistic Doppler factor
- **Gravitational redshift**: Light loses energy climbing out of the black hole's gravity well, shifting toward red by a factor sqrt(1 - 1.5/r)
- **Spiral density waves**: Three-armed spiral pattern from instabilities in the disk

**Particle disk**: 30,000 individual particles supplement the shader disk, showing Keplerian orbits (v ∝ 1/√r) with particles plunging inward inside 2 R_s (the plunge region beyond the ISCO).

### 5. Neutron Star Visuals

The neutron star surface is rendered with a custom shader that simulates:

- **Limb darkening**: The star appears dimmer near its edge because we're seeing through more tenuous outer layers at grazing angles
- **Gravitational redshift**: As mass approaches the TOV limit, the surface color shifts to red — the light is losing energy climbing out of a deeper gravitational well
- **Magnetic hotspots**: Bright bluish-white spots at the magnetic poles (aligned with the rotation axis), pulsing at 10× the spin frequency — simulating pulsar emission where charged particles are accelerated along magnetic field lines and emit X-rays/radio

Three additive glow layers pulse at different frequencies to simulate the magnetosphere and atmospheric scattering.

### 6. Relativistic Jets

When in neutron star mode, **bipolar polar jets** shoot out along the rotation axis at ~0.6c (simulated):

- **Collimation**: A pinch force squeezes particles toward the axis, mimicking magnetic field confinement
- **Doppler beaming**: Approaching jet appears brighter (δ^1.2 boost), receding jet dimmer
- **Color**: Inner core is white-hot, outer sheath is blue — simulating temperature stratification in the jet

---

## Controls

| Key | Action |
|-----|--------|
| W / S | Increase / decrease central density |
| Arrow keys | Rotate camera |
| Mouse drag | Orbit camera |
| Scroll | Zoom in/out |
| Space | Pause / unpause |
| Escape | Return to menu |

---

## Project Structure

```
neutron-star/
├── app/main.cpp          — Application loop, menu, user input
├── src/
│   ├── tov.cpp           — TOV equations (general relativistic hydrostatic equilibrium)
│   ├── eos.cpp           — Equations of state (polytropic + piecewise)
│   ├── rk4.cpp           — 4th-order Runge-Kutta integrator
│   ├── star_physics.cpp  — Star rotation, precession, TOV solver caller
│   ├── blackhole.cpp     — Black hole shader and accretion disk particles
│   ├── jets.cpp          — Relativistic polar jet particle system
│   └── renderer.cpp      — 3D rendering, shader management, UI
├── include/              — Headers
└── CMakeLists.txt        — Build system (auto-fetches raylib 5.0)
```

---

## Build

Requires CMake 3.14+ and a C++17 compiler. raylib is fetched automatically:

```bash
cd neutron-star
cmake -B build
cmake --build build
./build/tov_sim
```

---

## Summary

This simulation puts together **three layers of physics** that normally live in separate domains:

1. **Stellar structure**: Real-time integration of the TOV equations with a realistic nuclear EoS
2. **Black hole visualization**: Ray-traced curved spacetime with Doppler beaming, gravitational redshift, and a photon ring
3. **Neutron star phenomenology**: Pulsar hotspots, relativistic jets, and the density-driven collapse transition

The result is that you can *feel* the physics — see the star red-shift as it approaches the TOV limit, watch the accretion disk flicker with Doppler-boosted particles, and observe the moment gravity wins.