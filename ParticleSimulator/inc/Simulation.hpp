/**
  ******************************************************************************
  * @file    Simulation.hpp
  * @author  Josh Haden
  * @version V0.0.1
  * @date    18 JAN 2025
  * @brief   Header for Simulation.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIMULATION_HPP
#define __SIMULATION_HPP

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

#include "Engine.hpp"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

const bool   BOUNDING_BOX = false;
const int    NUM_PARTICLES = 2'000;
const int    MAX_NUM_PARTICLES = 3'000;
const double TIME_STEP = 0.002;
const double MAX_PARTICLE_COLOR_SPEED = 10.0;
const double M_PI = 3.1415926536;
const double GRAVITATIONAL_CONSTANT = 6.6743e-11;
const double MIN_DISTANCE = 2.0;                    // Minimum distance to prevent collapse
const double SOFTENING = 1.0;                       // Softening factor to prevent extreme forces
const double DAMPING = 1.0;                         // Velocity damping factor
const double COLLISION_DAMPING = 0.75;              // Collision response damping
const double PARTICLE_RADIUS = 0.0055;              // Radius of each particle for collision detection
const double REPULSION_FACTOR = 1.05;

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Class definition --------------------------------------------------------- */

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
private:
    double totalMass;
public:
    Simulation(Engine* engine, SimulationTemplate simulationTemplate = SimulationTemplate::Empty);

    void SetParticles(Particles* particles);

    void initializeParticles();

    void update();
    void updateParticles();

    void addParticle(glm::dvec2 pos);
    void addParticles(glm::dvec2 pos);
    void removeParticle(glm::dvec2 pos);
    void removeAllParticles();

    size_t getParticleCount();
    size_t getMaxParticleCount();
    double getTotalMass();

    int particleBrushSize;
    double newParticleVelocity;
    double newParticleMass;
    double simulationTime;

    Engine* engine;
    Particles* particles;

    SimulationTemplate simulationTemplate;
};



#endif /* __SIMULATION_HPP */

/************************END OF FILE************************/
