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
    this->newParticleVelocity = 0.0;
    this->newParticleMass     = 1e8;
    this->maxParticleCount    = MAX_NUM_PARTICLES;
    this->particles           = nullptr;
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
            Particle p;
            double angle = dis_angle(gen); // Random angle between 0 and 2*pi
            double r = radius * sqrt(dis_radius(gen)); // Random radius adjusted for area

            switch (this->GetSimulationTemplate())
            {
                case SimulationTemplate::SquareFill: p.position = glm::vec2(dis(gen) / 1.05f, dis(gen) / 1.05f); break;
                case SimulationTemplate::CircleFill: p.position = glm::vec2(r * cos(angle), r * sin(angle)); break;
                case SimulationTemplate::CircleOutline: p.position = glm::vec2(cos(angle) / 1.1f, sin(angle) / 1.1f); break;
                case SimulationTemplate::EllipseOutline: glm::vec2(cos(angle) / 6.0f, sin(angle) / 1.1f); break;
                case SimulationTemplate::RightTriangle: p.position = glm::vec2(r * cos(angle) * cos(angle) - 0.25f, r * sin(angle) * sin(angle) - 0.25f); break;
                case SimulationTemplate::Wave: p.position = glm::vec2(cos(angle / 4 - 2.25) / 1.1f, sin(angle * 4) / 1.1f); break;
            }

            p.velocity = glm::dvec2(0.0);          // Set initial particle velocities to zero
            p.color = calculateColor(p.velocity);  // Set initial color based on velocity
            p.mass = 1e8;                          // Set initial particles mass

            this->particles->push_back(p);
        }
    }
    else
    {
        switch (this->simulationTemplate)
        {
            case SimulationTemplate::CircularOrbit:
            {
                Particle star(glm::dvec2(0.0, 0.0), glm::dvec2(0.0), 1e6);
                Particle planet(glm::dvec2(0, 0.25), glm::dvec2(5, 0.0), 1);

                this->particles->push_back(star);
                this->particles->push_back(planet);
                break;
            }
            case SimulationTemplate::EllipticalOrbit:
            {
                Particle star(glm::dvec2(0.0, 0.0), glm::dvec2(0.0), 1e10);
                Particle planet(glm::dvec2(0, 0.1), glm::dvec2(20, 0.0), 1);

                this->particles->push_back(star);
                this->particles->push_back(planet);
                break;
            }
            case SimulationTemplate::PlanetaryOrbit:
            {
                Particle star(glm::dvec2(0.0, 0.0), glm::dvec2(0.0), 1e6);
                Particle planet1(glm::dvec2(0, 0.1), glm::dvec2(3.2, 0.0), 1.0);
                Particle planet2(glm::dvec2(0, 0.3), glm::dvec2(5.5, 0.0), 1.0);
                Particle planet3(glm::dvec2(0, 0.5), glm::dvec2(7.0, 0.0), 1.0);
                Particle planet4(glm::dvec2(0, 0.75), glm::dvec2(8.5, 0.0), 1.0);

                this->particles->push_back(star);
                this->particles->push_back(planet1);
                this->particles->push_back(planet2);
                this->particles->push_back(planet3);
                this->particles->push_back(planet4);
                break;
            }
            case SimulationTemplate::BinaryStar:
            {
                Particle star1(glm::dvec2(-0.25, 0.0), glm::dvec2(0.0, 5.0), 1e6);
                Particle star2(glm::dvec2(0.25, 0.0), glm::dvec2(0.f, -5.0), 1e6);

                this->particles->push_back(star1);
                this->particles->push_back(star2);
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
    Particle p;

    float x = 2.0f * (float)position.x / (float)WINDOW_WIDTH - 1.0f;
    float y = 1.0f - 2.0f * (float)position.y / (float)WINDOW_HEIGHT;

    p.position = glm::vec2(x, y);
    p.velocity = glm::dvec2(this->newParticleVelocity, 0.0);
    p.color = calculateColor(p.velocity);
    p.mass = this->newParticleMass + 1.0;

    if (this->GetParticleCount() < this->GetMaxParticleCount())
    {
        this->particles->push_back(p);
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
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    std::uniform_real_distribution<> dis_angle(0.0, 2.0 * MATH_PI_CONSTANT);
    std::uniform_real_distribution<> dis_radius(0.0, 1.0);

    double radius = 0.01 * particleBrushSize / 2.0;

    Particle p;

    double x = 2.0 * position.x / (double)WINDOW_WIDTH - 1.0;
    double y = 1.0 - 2.0 * position.y / (double)WINDOW_HEIGHT;

    for (int i = 0; i < this->particleBrushSize; ++i)
    {
        Particle p;
        double angle = dis_angle(gen); // Random angle between 0 and 2*pi
        double r = radius * sqrt(dis_radius(gen)); // Random radius adjusted for area

        p.position = glm::dvec2(r * cos(angle) + x, r * sin(angle) + y); // circle fill

        p.velocity = glm::dvec2(this->newParticleVelocity, 0.0);
        p.color = calculateColor(p.velocity);  // Set initial color based on velocity
        p.mass = this->newParticleMass + 1.0;

        if (this->GetParticleCount() < this->GetMaxParticleCount())
        {
            this->particles->push_back(p);
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
    this->particles->clear();
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

    for (size_t i = 0; i < this->particles->size(); ++i)
    {
        Particle& p = (*particles)[i];

        glm::dvec2 direction = p.position - glm::dvec2(x, y);
        double distance = glm::length(direction);

        if (distance < (this->particleBrushSize * PARTICLE_RADIUS / 2))
        {
            std::vector<Particle>::iterator it = this->particles->begin();
            std::advance(it, i);
            this->particles->erase(it);
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
  * @brief  Calculate forces applied to particles using n-body physics
  * @param  None
  * @retval None
  */
void Simulation::UpdateParticles()
{
    this->totalMass = 0;

    for (size_t i = 0; i < this->particles->size(); ++i)
    {
        Particle& pI = (*particles)[i];

        this->totalMass += pI.mass;

        /*
        Bounding box to prevent particles escaping from view
        */
        if (BOUNDING_BOX)
        {
            for (int j = 0; j < 2; ++j)
            {

                Particle& pJ = (*particles)[j];

                if (std::abs(pI.position[j]) > 1.0)
                {
                    pI.position[j] = glm::sign(pI.position[j]) * 1.0;
                    pI.velocity[j] *= -0.9;
                }
            }
        }

        /*
        Calculate and apply gravitational forces and handle collision of particles
        */
        for (size_t j = i + 1; j < this->particles->size(); ++j)
        {
            Particle& pJ = (*particles)[j];

            glm::dvec2 direction = pJ.position - pI.position;
            double distance = glm::length(direction);

            // Gravity calculation
            double gravityDistance = std::max(distance, MIN_INTERACTION_DISTANCE);
            //double forceMagnitude = (GRAVITATIONAL_CONSTANT * pI.mass * pJ.mass) / (distance * distance);
            double forceMagnitude = GRAVITATIONAL_CONSTANT * pI.mass * pJ.mass / (gravityDistance * gravityDistance + SOFTENING * SOFTENING);

            glm::dvec2 force = forceMagnitude * glm::normalize(direction);

            // Apply gravitational force
            pI.acceleration = force / pI.mass;
            pJ.acceleration = force / pJ.mass;

            pI.velocity += pI.acceleration * this->GetTimeStep();
            pJ.velocity -= pJ.acceleration * this->GetTimeStep();

            /*
            Collision detection and response
            */
            if (distance < 2 * PARTICLE_RADIUS)
            {
                glm::dvec2 collisionNormal = glm::normalize(direction);
                glm::dvec2 relativeVelocity = pJ.velocity - pI.velocity;

                double separatingVelocity = glm::dot(relativeVelocity, collisionNormal);

                if (separatingVelocity < 0)
                {
                    double impulse = -(1 + COLLISION_DAMPING) * separatingVelocity / ((1 / pI.mass) + (1 / pJ.mass));

                    pI.velocity -= impulse / pI.mass * collisionNormal * REPULSION_FACTOR;
                    pJ.velocity += impulse / pJ.mass * collisionNormal * REPULSION_FACTOR;

                    // Separate overlapping particles
                    double overlap = 2 * PARTICLE_RADIUS - distance;
                    glm::dvec2 separationVector = overlap * 0.5 * collisionNormal;
                    pI.position -= separationVector;
                    pJ.position += separationVector;
                }
            }
        }

        /*
        Update the particle properties
        */
        pI.update();
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
    return this->particles->size();
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
  * @retval double
  */
double Simulation::GetNewParticleVelocity() const
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
  * @brief  Get pointer to particles
  * @param  None
  * @retval Particles*
  */
Particles* Simulation::GetParticles() const
{
    return this->particles;
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
void Simulation::SetNewParticleVelocity(double velocity)
{
    this->newParticleVelocity = velocity;
}


/**
  * @brief  Bind particles that will be used in the simulation
  * @param  particles*
  * @retval None
  */
void Simulation::SetParticles(Particles* particles)
{
    this->particles = particles;
}


/**
  * @brief  Set particle brush size used to add/remove particles
  * @param  size
  * @retval None
  */
void Simulation::SetParticleBrushSize(int size)
{
    this->particleBrushSize = size;
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
