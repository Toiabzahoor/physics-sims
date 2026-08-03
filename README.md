# Astrophysics Simulator
Hey there! I am Toiab Zahoor, and this is my project ZweiTron. ZweiTron is an interactive neutron star and black hole simulator I made using C++ and raylib. It is highly optimized so that it can run on low-end laptops or PCs. Initially, my idea was only to make a neutron star simulation. But, a neutron star isn't that cool to look at because most of the stuff happening is subatomic, so I decided to add in a black hole simulator as well. Both are modeled by real equations, and I have tried my best to be as faithful to the physics as possible. Here is how I made it:
---
## How It Works
So here is what you can actually do in ZweiTron. You use the **W** and **S** keys to dial the central density of the star up or down. Because the engine solves the real general relativistic structure equations on the spot, it updates what you see in real time.
Depending on how high you push the density, you get to see three main stages:
*   **Low density:** It just looks like a normal star.
*   **~1x nuclear density:** It turns into a typical neutron star. It spins super fast and shoots out these relativistic polar jets.
*   **Above ~1.5x nuclear density:** This is where you cross the mass limit (greater than 2.1 solar masses). Gravity completely wins out, the star shrinks, the Schwarzschild radius is reached, and it collapses right into a black hole surrounded by a glowing accretion disk.
---
## The Physics, Explained
### 1. The TOV Equations (How it doesn't collapse immediately)
Inside any star, you have gravity pulling inward and pressure pushing outward. Because we are dealing with extreme mass, Newtonian physics doesn't work well, so I used the Tolman-Oppenheimer-Volkoff (TOV) equations.
Basically, this tells us that the pressure gradient depends on mass-energy, the pressure itself, and a metric correction term that handles the curvature of spacetime. To actually simulate this, the code solves these equations outward from the center of the star using a 4th-order Runge-Kutta integrator, stopping when the pressure hits zero (which is the surface).
### 2. The Equation of State 
To make the TOV equations work, you need to define how pressure and density relate to each other—the Equation of State (EoS). I went with a piecewise polytropic EoS:
*   **Crust:** Simulates a solid lattice of neutron-rich nuclei in the outer layers.
*   **Core:** Simulates uniform nuclear matter deeper inside.
I also stitched these two parts together with continuous enthalpy to make sure it doesn't violate the laws of thermodynamics I encountered without doing that.
### 3. The TOV Limit
Neutron degeneracy pressure can only hold up so much mass. Once you dial it past ~2.1 solar masses, TOV stops working. Gravity completely overwhelms the strong nuclear force. This is the TOV limit, and in the simulation, it's the exact threshold that triggers the collapse into a black hole.
### 4. Black Hole Rendering (Curved Spacetime in a Shader)
Once the star collapses, the rendering switches over to a full-screen GLSL shader that actually ray-marches through curved spacetime.
*   **Ray Tracing Gravity:** For every pixel, a ray gets cast from the camera, and its path gets bent by a pseudo-relativistic gravity term. This creates a realistic photon sphere at exactly 1.5 Schwarzschild radii (R_s)—the exact spot where light orbits in a circle.
*   **The Shadow & Photon Ring:** Rays that pass too close get swallowed by the event horizon, creating the black hole's shadow. Rays that just graze the photon sphere get bent around and form a super bright photon ring.
*   **The Accretion Disk:** The disk around the black hole is procedurally generated with a temperature gradient (white-hot near the inner edge, cooling to red on the outside). It includes gravitational redshift (light losing energy as it climbs out of the gravity well) and Doppler beaming (material moving toward you at relativistic speeds looks way brighter).
*   **Particle System:** To make it pop, I also added 30,000 individual particles on top of the shader to show Keplerian orbits, which actually plunge inward once they cross the innermost stable circular orbit (ISCO). (Although I faced many issues and now this is mostly a vestigial thing)
### 5. Neutron Star Visuals & Jets
For the neutron star itself, I wrote a custom shader to handle some cool relativistic effects:
*   **Limb Darkening & Redshift:** The edges of the star look dimmer, and as you increase the mass towards the TOV limit, the whole star visibly shifts to red because the light struggles more to escape the gravity well.
*   **Pulsar Hotspots:** There are bright bluish-white magnetic hotspots at the poles that pulse at 10x the spin frequency to simulate X-ray/radio emission. (Although I didn't work much because my laptop has integrated graphics and basically it would lag a lot if I went into detail.)
*   **Relativistic Jets:** When it's spinning fast, it shoots out bipolar jets at around 0.6c (just an approximation due to the low-end nature). They use a pinch force to simulate magnetic collimation, and just like the accretion disk, they use Doppler beaming so the jet pointing towards you is noticeably brighter than the one pointing away.
---
## Controls

| Key / Action | Result |
| :--- | :--- |
| **W / S** | Increase or decrease the central density of the star |
| **Arrow keys / Mouse drag** | Orbit and rotate the camera |
| **Scroll wheel** | Zoom in and out |
| **Spacebar** | Pause or unpause the physics |
| **Escape** | Head back to the main menu |

---
## Code Structure
If you want to poke around the source code, here is how I laid it out:
*   `app/main.cpp` - The main application loop and menu.
*   `src/tov.cpp` & `src/eos.cpp` - The heavy physics lifting (general relativistic equations and the equations of state).
*   `src/rk4.cpp` - The 4th-order Runge-Kutta math integrator.
*   `src/blackhole.cpp` & `src/jets.cpp` - The shaders and particle systems for the black hole and relativistic jets.
*   `src/renderer.cpp` - Handles all the 3D rendering and UI.
---
## How to Build It
ZweiTron is super easy to compile. You just need CMake (3.14+) and a C++17 compiler. The build system automatically fetches raylib 5.0 for you, so you don't have to worry about linking dependencies manually.
Just run these commands in your terminal:
```bash
cmake -B build
cmake --build build
./build/ZweiTron
```
thanks for checking out!!
