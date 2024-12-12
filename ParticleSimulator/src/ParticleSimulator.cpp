#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

int main()
{
    Engine e;

    Simulation sim(&e);

    sim.engine->Init();
}