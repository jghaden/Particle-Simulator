/**
  ******************************************************************************
  * @file    Simulation.cpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    19 JAN 2025
  * @brief
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

#include "Simulation.hpp"
#include "Particle.hpp"
#include "Quadtree.hpp"

/* Global variables --------------------------------------------------------- */
/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */



/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/******************************************************************************/


/**
  * @brief  Simulation constructor
  * @param  engine
  * @param  simulationTemplate
  * @retval None
  */
Simulation::Simulation(Engine* engine, SimulationTemplate simulationTemplate)
{
    this->engine              = engine;
    this->engine->SetSimulation(this);
    this->newParticleVelocity = glm::vec2(0.0);
    this->newParticleMass     = 1e8;
    this->maxParticleCount    = MAX_NUM_PARTICLES;
    this->particleData        = nullptr;
    this->particleBrushSize   = 5;
    this->simulationTemplate  = simulationTemplate;
    this->simulationTime      = 0.0;
    this->timeStep            = TIME_STEP;
    this->totalMass           = 0.0;
}


/**
  * @brief  Initialize simulation variables and call related setup functions
  * @param  None
  * @retval None
  */
void Simulation::Init()
{
    this->InitTemplateParticles();
}


/**
  * @brief  Initialize particles for template
  * @param  None
  * @retval None
  */
void Simulation::InitTemplateParticles()
{
    if (this->GetSimulationTemplate() != SimulationTemplate::Empty && this->GetSimulationTemplate() < SimulationTemplate::CircularOrbit)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        std::uniform_real_distribution<> dis_angle(0.0, 2.0 * MATH_PI_CONSTANT);
        std::uniform_real_distribution<> dis_radius(0.0, 1.0);

        float radius = 0.5;

        for (int i = 0; i < NUM_TEMPLATE_PARTICLES; ++i)
        {
            glm::dvec2 position;
            double angle = dis_angle(gen);              // Random angle between 0 and 2*pi
            double r = radius * sqrt(dis_radius(gen));  // Random radius adjusted for area

            switch (this->GetSimulationTemplate())
            {
                case SimulationTemplate::SquareFill: position = glm::dvec2(dis(gen) / 1.05, dis(gen) / 1.05); break;
                case SimulationTemplate::CircleFill: position = glm::dvec2(r * cos(angle), r * sin(angle)); break;
                case SimulationTemplate::CircleOutline: position = glm::dvec2(cos(angle) / 1.1, sin(angle) / 1.1); break;
                case SimulationTemplate::EllipseOutline: position = glm::dvec2(cos(angle) / 6.0, sin(angle) / 1.1); break;
                case SimulationTemplate::RightTriangle: position = glm::dvec2(r * cos(angle) * cos(angle) - 0.25, r * sin(angle) * sin(angle) - 0.25); break;
                case SimulationTemplate::Wave: position = glm::dvec2(cos(angle / 4 - 2.25) / 1.1, sin(angle * 4) / 1.1); break;
            }

            this->particleData->AddParticle(1e8, position, glm::dvec2(0.0));
        }
    }
    else
    {
        switch (this->simulationTemplate)
        {
            case SimulationTemplate::CircularOrbit:
            {
                this->particleData->AddParticle(1e6, glm::dvec2(0.0, 0.0), glm::dvec2(0.0));
                this->particleData->AddParticle(1, glm::dvec2(0, 0.25), glm::dvec2(5, 0.0));
                break;
            }
            case SimulationTemplate::EllipticalOrbit:
            {
                this->particleData->AddParticle(1e10, glm::dvec2(0.0, 0.0), glm::dvec2(0.0));
                this->particleData->AddParticle(1, glm::dvec2(0, 0.1), glm::dvec2(20, 0.0));
                break;
            }
            case SimulationTemplate::PlanetaryOrbit:
            {
                this->particleData->AddParticle(1e6, glm::dvec2(0.0, 0.0), glm::dvec2(0.0));
                this->particleData->AddParticle(1.0, glm::dvec2(0, 0.1), glm::dvec2(3.2, 0.0));
                this->particleData->AddParticle(1.0, glm::dvec2(0, 0.3), glm::dvec2(5.5, 0.0));
                this->particleData->AddParticle(1.0, glm::dvec2(0, 0.5), glm::dvec2(7.0, 0.0));
                this->particleData->AddParticle(1.0, glm::dvec2(0, 0.75), glm::dvec2(8.5, 0.0));
                break;
            }
            case SimulationTemplate::BinaryStar:
            {
                this->particleData->AddParticle(1e6, glm::dvec2(-0.25, 0.0), glm::dvec2(0.0, 5.0));
                this->particleData->AddParticle(1e6, glm::dvec2(0.25, 0.0), glm::dvec2(0.0, -5.0));
                break;
            }
        }
    }
}


/**
  * @brief  Add particle at a specified position
  * @param  position
  * @retval None
  */
void Simulation::AddParticle(glm::dvec2 position)
{
    double x = 2.0 * position.x / (double)WINDOW_WIDTH - 1.0;
    double y = 1.0 - 2.0 * position.y / (double)WINDOW_HEIGHT;

    if (this->GetParticleCount() < this->GetMaxParticleCount())
    {
        this->particleData->AddParticle(
            this->newParticleMass + 1.0,
            glm::dvec2(x, y),
            glm::dvec2(this->newParticleVelocity)
        );
    }
}


/**
  * @brief  Add multiple particles in a random distribution at a specified position
  * @param  position
  * @retval None
  */
void Simulation::AddParticles(glm::dvec2 position)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis_angle(0.0, 2.0 * MATH_PI_CONSTANT);
    std::uniform_real_distribution<> dis_radius(0.0, 1.0);

    double radius = 0.01 * particleBrushSize / 2.0;

    double x = 2.0 * position.x / (double)WINDOW_WIDTH - 1.0;
    double y = 1.0 - 2.0 * position.y / (double)WINDOW_HEIGHT;

    for (int i = 0; i < this->particleBrushSize; ++i)
    {
        double angle = dis_angle(gen); // Random angle between 0 and 2*pi
        double r = radius * sqrt(dis_radius(gen)); // Random radius adjusted for area

        glm::dvec2 particlePos = glm::dvec2(r * cos(angle) + x, r * sin(angle) + y);

        if (this->GetParticleCount() < this->GetMaxParticleCount())
        {
            this->particleData->AddParticle(
                this->newParticleMass + 1.0,
                particlePos,
                glm::dvec2(this->newParticleVelocity)
            );
        }
    }
}


/**
  * @brief  Remove all particles in simulation
  * @param  None
  * @retval None
  */
void Simulation::RemoveAllParticles()
{
    this->particleData->Clear();
}


/**
  * @brief  Remove particle at a specified position
  * @param  position
  * @retval None
  */
void Simulation::RemoveParticle(glm::dvec2 position)
{
    double x = 2.0 * position.x / (double)WINDOW_WIDTH - 1.0;
    double y = 1.0 - 2.0 * position.y / (double)WINDOW_HEIGHT;

    // Remove particles in reverse order to avoid index shifting issues
    for (int i = (int)this->particleData->Size() - 1; i >= 0; --i)
    {
        glm::dvec2 direction = this->particleData->positions[i] - glm::dvec2(x, y);
        double distance = glm::length(direction);

        if (distance < (this->particleBrushSize * PARTICLE_RADIUS / 2))
        {
            this->particleData->RemoveParticle(i);
        }
    }
}

/**
  * @brief  Update simulation
  * @param  None
  * @retval None
  */
void Simulation::Update()
{
    this->simulationTime += this->GetTimeStep();

    this->UpdateParticles();
}


/**
  * @brief  Calculate forces applied to particles using Barnes-Hut algorithm
  * @param  None
  * @retval None
  */
void Simulation::UpdateParticles()
{
    this->totalMass = 0;

    ParticleData& particles = *particleData;
    size_t numParticles = particles.Size();

    if (numParticles == 0) return;

    // Bounding box that encloses all particles
    double minX = -1.0, maxX = 1.0;
    double minY = -1.0, maxY = 1.0;

    for (size_t i = 0; i < numParticles; ++i)
    {
        minX = std::min(minX, particles.positions[i].x);
        maxX = std::max(maxX, particles.positions[i].x);
        minY = std::min(minY, particles.positions[i].y);
        maxY = std::max(maxY, particles.positions[i].y);
    }

    double centerX = (minX + maxX) * 0.5;
    double centerY = (minY + maxY) * 0.5;
    double halfSize = std::max(maxX - minX, maxY - minY) * 0.5;

    // Build quadtree
    QuadtreeNode root(centerX, centerY, halfSize + 1e-3);
    for (size_t i = 0; i < numParticles; ++i)
    {
        root.Insert(i, particles);
    }

    root.ComputeMassDistribution(particles);

    // Compute forces using Barnes-Hut, accumulate in each particle
    for (size_t i = 0; i < numParticles; ++i)
    {
        // Reset acceleration for this time step
        particles.accelerations[i] = glm::dvec2(0.0);

        glm::dvec2 bhForce = ComputeForceBarnesHut(i, particles, &root, THETA);
        // a = F / m
        particles.accelerations[i] = bhForce / particles.masses[i];
    }

    // Pre-allocate reusable vector for collision detection (optimization)
    std::vector<size_t> neighborsReusable;
    neighborsReusable.reserve(32);

    // Update velocities based on acceleration and handle collisions
    for (size_t i = 0; i < numParticles; i++)
    {
        // Bounding box to keep particles in view
        if (ENABLE_BOUNDING_BOX)
        {
            glm::dvec2& position = particles.positions[i];
            glm::dvec2& velocity = particles.velocities[i];

            for (int axis = 0; axis < 2; axis++)
            {
                if (std::abs(position[axis]) > 1.0)
                {
                    // Clamp position
                    position[axis] = glm::sign(position[axis]) * 1.0;
                    // Invert (dampen) velocity along that axis
                    velocity[axis] *= -0.9;
                }
            }
        }

        const glm::dvec2& posI = particles.positions[i];

        // Query a bounding box that roughly covers possible collisions.
        double range = 2.0 * PARTICLE_RADIUS;
        double xMin = posI.x - range;
        double xMax = posI.x + range;
        double yMin = posI.y - range;
        double yMax = posI.y + range;

        neighborsReusable.clear();
        root.QueryRange(xMin, yMin, xMax, yMax, neighborsReusable);

        // Check collisions only with these neighbors
        for (size_t j : neighborsReusable)
        {
            if (j == i)
                continue; // skip self

            glm::dvec2 direction = particles.positions[j] - particles.positions[i];
            double distance = glm::length(direction);
            if (distance < 2.0 * PARTICLE_RADIUS)
            {
                glm::dvec2 collisionNormal = glm::normalize(direction);
                glm::dvec2 relativeVelocity = particles.velocities[j] - particles.velocities[i];
                double separatingVelocity = glm::dot(relativeVelocity, collisionNormal);

                if (separatingVelocity < 0)
                {
                    double impulse = -(1 + COLLISION_DAMPING) * separatingVelocity /
                        ((1 / particles.masses[i]) + (1 / particles.masses[j]));

                    particles.velocities[i] -= (impulse / particles.masses[i]) * collisionNormal * REPULSION_FACTOR;
                    particles.velocities[j] += (impulse / particles.masses[j]) * collisionNormal * REPULSION_FACTOR;

                    // Separate overlapping particles
                    double overlap = 2 * PARTICLE_RADIUS - distance;
                    glm::dvec2 separationVector = overlap * 0.5 * collisionNormal;

                    particles.positions[i] -= separationVector;
                    particles.positions[j] += separationVector;
                }
            }
        }

        particles.UpdateParticle(i, this->GetTimeStep());

        this->totalMass = root.totalMass;
    }
}


/**
  * @brief  Get particle brush size used to add/remove particles
  * @param  None
  * @retval int
  */
int Simulation::GetParticleBrushSize() const
{
    return this->particleBrushSize;
}


/**
  * @brief  Get maximum number of particles that can be present in the simulation
  * @param  None
  * @retval size_t
  */
size_t Simulation::GetMaxParticleCount() const
{
    return this->maxParticleCount;
}


/**
  * @brief  Get current number of particles present in the simulation
  * @param  None
  * @retval size_t
  */
size_t Simulation::GetParticleCount() const
{
    return this->particleData->Size();
}


/**
  * @brief  Get mass that will be applied to new particles
  * @param  None
  * @retval double
  */
double Simulation::GetNewParticleMass() const
{
    return this->newParticleMass;
}


/**
  * @brief  Get velocity that will be applied to new particles
  * @param  None
  * @retval glm::vec2
  */
glm::vec2 Simulation::GetNewParticleVelocity() const
{
    return this->newParticleVelocity;
}


/**
  * @brief  Get current simulation time
  * @param  None
  * @retval double
  */
double Simulation::GetSimulationTime() const
{
    return this->simulationTime;
}


/**
  * @brief  Get current simulation time step
  * @param  None
  * @retval double
  */
double Simulation::GetTimeStep() const
{
    return this->timeStep;
}


/**
  * @brief  Get total mass of all the particles present in the simulation
  * @param  None
  * @retval double
  */
double Simulation::GetTotalMass() const
{
    return this->totalMass;
}


/**
  * @brief  Get template that is used when the simulation is initiated or restarted
  * @param  None
  * @retval SimulationTemplate
  */
SimulationTemplate Simulation::GetSimulationTemplate() const
{
    return this->simulationTemplate;
}


/**
  * @brief  Get pointer to particle data (SoA)
  * @param  None
  * @retval ParticleData*
  */
ParticleData* Simulation::GetParticleData() const
{
    return this->particleData;
}


/**
  * @brief  Get pointer to engine
  * @param  None
  * @retval Engine*
  */
Engine* Simulation::GetEngine() const
{
    return this->engine;
}


/**
  * @brief  Set maximum number of particles that can be present in the simulation
  * @param  count
  * @retval None
  */
void Simulation::SetMaxParticleCount(size_t count)
{
    this->maxParticleCount = count;
}


/**
  * @brief  Set mass that will be applied to new particles
  * @param  mass
  * @retval None
  */
void Simulation::SetNewParticleMass(double mass)
{
    this->newParticleMass = mass;
}


/**
  * @brief  Set velocity that will be applied to new particles
  * @param  velocity
  * @retval None
  */
void Simulation::SetNewParticleVelocity(glm::vec2 velocity)
{
    this->newParticleVelocity = velocity;
}


/**
  * @brief  Bind particle data that will be used in the simulation
  * @param  particleData*
  * @retval None
  */
void Simulation::SetParticleData(ParticleData* particleData)
{
    this->particleData = particleData;
}


/**
  * @brief  Set particle brush size used to add/remove particles
  * @param  size
  * @retval None
  */
void Simulation::SetParticleBrushSize(int size)
{
    this->particleBrushSize = glm::clamp(size, 1, 150);
}


/**
  * @brief  Set template that is used when the simulation is initiated or restarted
  * @param  simulationTemplate
  * @retval None
  */
void Simulation::SetSimulationTemplate(SimulationTemplate simulationTemplate)
{
    this->simulationTemplate = simulationTemplate;
}


/**
  * @brief  Set simulation time step to increase/decrease simulation speed
  * @param  timeStep
  * @retval None
  */
void Simulation::SetTimeStep(double timeStep)
{
    this->timeStep = timeStep;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
