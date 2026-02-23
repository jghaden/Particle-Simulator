/**
  ******************************************************************************
  * @file    Simulation.hpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    19 JAN 2025
  * @brief   Header for Simulation.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __SIMULATION_HPP
#define __SIMULATION_HPP

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

#include "Engine.hpp"
#include "Particle.hpp"
#include "ParticleData.hpp"

/* Exported types ----------------------------------------------------------- */

typedef unsigned int          QuadNum;
typedef glm::dvec2            Vec2D;
typedef std::vector<Particle> Particles;

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

/* Exported constants ------------------------------------------------------- */

constexpr bool   ENABLE_BOUNDING_BOX      = true;           // Flag to toggle whether or not to keep particles within viewport
constexpr int    MAX_NUM_PARTICLES        = 50'000;         // Max number of particles that can be in the simulation
constexpr int    NUM_TEMPLATE_PARTICLES   = 2'000;          // Number of particles to create for templates
constexpr double COLLISION_DAMPING        = 0.0;            // Collision response damping
constexpr double DAMPING_FACTOR           = 1.0;            // Velocity damping factor
constexpr double MATH_PI_CONSTANT         = 3.1415926536;   // Mathmatical constant - pi
constexpr double MAX_PARTICLE_COLOR_SPEED = 10.0;           // Upper bounds for speed to determine how to color a particle
constexpr double MIN_INTERACTION_DISTANCE = 0.001;          // Minimum distance to prevent collapse
constexpr double GRAVITATIONAL_CONSTANT   = 6.6743e-11;     // Physics constant - G
constexpr double PARTICLE_RADIUS          = 0.0055;         // Radius of each particle for collision detection
constexpr double REPULSION_FACTOR         = 1.00;           // Basic repulsion force to apply when particles collide
constexpr double SOFTENING                = 0.01;           // Softening factor to prevent extreme forces
constexpr double TIME_STEP                = 1e-3;           // Time in seconds to step through the simulation

/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Forward declarations ----------------------------------------------------- */

struct QuadtreeNodePool;

/* Class definition --------------------------------------------------------- */

class Simulation
{
public:
    /* Public member variables -------------------------------------------------- */
    /* Public member functions -------------------------------------------------- */

    Simulation(Engine* engine, SimulationTemplate simulationTemplate = SimulationTemplate::Empty);
    ~Simulation();

    void Init();
    void InitTemplateParticles();

    void AddParticle(glm::dvec2 position);
    void AddParticles(glm::dvec2 position);
    void RemoveAllParticles();
    void RemoveParticle(glm::dvec2 position);
    void Update();
    void UpdateParticles();

    /* Getters ------------------------------------------------------------------ */

    int GetParticleBrushSize() const;
    size_t GetMaxParticleCount() const;
    size_t GetParticleCount() const;
    double GetNewParticleMass() const;
    double GetSimulationTime() const;
    double GetTimeStep() const;
    double GetTotalMass() const;
    glm::vec2 GetNewParticleVelocity() const;
    SimulationTemplate GetSimulationTemplate() const;
    ParticleData* GetParticleData() const;
    Engine* GetEngine() const;

    /* Setters ------------------------------------------------------------------ */

    void SetMaxParticleCount(size_t count);
    void SetNewParticleMass(double mass);
    void SetNewParticleVelocity(glm::vec2 velocity);
    void SetParticleData(ParticleData* particleData);
    void SetParticleBrushSize(int size);
    void SetSimulationTemplate(SimulationTemplate simulationTemplate = SimulationTemplate::Empty);
    void SetTimeStep(double timeStep);
private:
    /* Private member variables ------------------------------------------------- */

    int                particleBrushSize;
    size_t             maxParticleCount;
    double             newParticleMass;
    double             simulationTime;
    double             timeStep;
    double             totalMass;
    glm::vec2          newParticleVelocity;
    SimulationTemplate simulationTemplate;
    ParticleData*      particleData;
    Engine*            engine;
    QuadtreeNodePool*  nodePool;

    /* Private member functions ------------------------------------------------- */
    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */
};



#endif /* __SIMULATION_HPP */

/******************************** END OF FILE *********************************/
