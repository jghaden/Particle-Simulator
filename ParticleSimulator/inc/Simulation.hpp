#pragma once

#include "PCH.hpp"

#include "Engine.hpp"

const int NUM_PARTICLES = 2'000;
//const double TIME_STEP = 0.00016;
const double TIME_STEP = 0.002;
//const float MAX_PARTICLE_COLOR_SPEED = 0.3f;
const double MAX_PARTICLE_COLOR_SPEED = 10.0;
const double M_PI = 3.1415926536;
const double GRAVITATIONAL_CONSTANT = 0.005;
//const float GRAVITATIONAL_CONSTANT = 6.6743e-11;
const double MIN_DISTANCE = 2.0;                // Minimum distance to prevent collapse
//const float SOFTENING = 2.75f;                   // Softening factor to prevent extreme forces
const double SOFTENING = 1.0;                   // Softening factor to prevent extreme forces
//const float DAMPING = 0.975f;                   // Velocity damping factor
const double DAMPING = 1.0;                   // Velocity damping factor
const double COLLISION_DAMPING = 0.75;          // Collision response damping
const double PARTICLE_RADIUS = 0.0055;            // Radius of each particle for collision detection
const double REPULSION_FACTOR = 1.05;

enum SimulationTemplate
{
    Empty,
    SquareFill,
    CircleFill,
    CircleOutline,
    EllipseOutline,
    RightTriangle,
    Wave,

    CircularOrbit,
    EllipticalOrbit,
    PlanetaryOrbit,
    BinaryStar
};

class Particle;

typedef std::vector<Particle> Particles;
typedef unsigned int QuadNum;
typedef glm::dvec2 Vec2D;

class Simulation
{
public:
    Simulation(Engine* engine, SimulationTemplate simulationTemplate = SimulationTemplate::CircleOutline);

    void SetParticles(Particles* particles);

    void initializeParticles();

    void update();
    void updateParticles(Particles &particles);

    void addParticle(Particles &particles, glm::dvec2 pos);
    void addParticles(Particles &particles, glm::dvec2 pos);
    void removeParticle(Particles &particles, glm::dvec2 pos);

    int particleBrushSize;
    double newParticleVelocity;
    double newParticleMass;
    double simulationTime;

    Engine* engine;
    Particles* particles;

    SimulationTemplate simulationTemplate;
};