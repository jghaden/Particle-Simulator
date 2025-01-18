#include "PCH.hpp"

#include "Simulation.hpp"
#include "Particle.hpp"

Simulation::Simulation(Engine* engine, SimulationTemplate simulationTemplate)
{
    this->engine = engine;
    this->engine->SetSimulation(this);

    this->simulationTemplate = simulationTemplate;

    this->particleBrushSize = 1;
    this->newParticleVelocity = 0.0;
    this->newParticleMass = 10e8;
    this->simulationTime = 0.0;
}

void Simulation::SetParticles(Particles* particles)
{
    this->particles = particles;
}

void Simulation::initializeParticles()
{
    if (this->simulationTemplate != SimulationTemplate::Empty && simulationTemplate < SimulationTemplate::CircularOrbit)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        std::uniform_real_distribution<> dis_angle(0.0, 2.0 * M_PI);
        std::uniform_real_distribution<> dis_radius(0.0, 1.0);

        float radius = 0.5;

        for (int i = 0; i < NUM_PARTICLES; ++i)
        {
            Particle p;
            double angle = dis_angle(gen); // Random angle between 0 and 2*pi
            double r = radius * sqrt(dis_radius(gen)); // Random radius adjusted for area

            switch (this->simulationTemplate)
            {
                case SimulationTemplate::SquareFill: p.position = glm::vec2(dis(gen) / 1.05f, dis(gen) / 1.05f); break;
                case SimulationTemplate::CircleFill: p.position = glm::vec2(r * cos(angle), r * sin(angle)); break;
                case SimulationTemplate::CircleOutline: p.position = glm::vec2(cos(angle) / 1.1f, sin(angle) / 1.1f); break;
                case SimulationTemplate::EllipseOutline: glm::vec2(cos(angle) / 6.0f, sin(angle) / 1.1f); break;
                case SimulationTemplate::RightTriangle: p.position = glm::vec2(r * cos(angle) * cos(angle) - 0.25f, r * sin(angle) * sin(angle) - 0.25f); break;
                case SimulationTemplate::Wave: p.position = glm::vec2(cos(angle / 4 - 2.25) / 1.1f, sin(angle * 4) / 1.1f); break;                
            }

            p.velocity = glm::dvec2(0.0); // Set initial particle velocities to zero
            p.color = calculateColor(p.velocity);  // Set initial color based on velocity
            //p.color = glm::vec3(static_cast<float>(rand()) / RAND_MAX * 2.0f - 0.5f);
            p.mass = 1e8;
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

void Simulation::update()
{
    this->simulationTime += TIME_STEP;
}

void Simulation::updateParticles()
{
    for (size_t i = 0; i < this->particles->size(); ++i)
    {
        Particle& pI = (*particles)[i];

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
            double gravityDistance = std::max(distance, MIN_DISTANCE);
            //double forceMagnitude = (GRAVITATIONAL_CONSTANT * pI.mass * pJ.mass) / (distance * distance);
            double forceMagnitude = GRAVITATIONAL_CONSTANT * pI.mass * pJ.mass / (gravityDistance * gravityDistance + SOFTENING * SOFTENING);

            glm::dvec2 force = forceMagnitude * glm::normalize(direction);

            // Apply gravitational force
            pI.acceleration = force / pI.mass;
            pJ.acceleration = force / pJ.mass;

            pI.velocity += pI.acceleration * TIME_STEP;
            pJ.velocity -= pJ.acceleration * TIME_STEP;

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

void Simulation::addParticle(glm::dvec2 pos)
{
    Particle p;

    float x = 2.0f * (float)pos.x / (float)WINDOW_WIDTH - 1.0f;
    float y = 1.0f - 2.0f * (float)pos.y / (float)WINDOW_HEIGHT;

    p.position = glm::vec2(x, y);
    p.velocity = glm::dvec2(this->newParticleVelocity, 0.0);
    p.color = calculateColor(p.velocity);
    p.mass = this->newParticleMass + 1.0;

    this->particles->push_back(p);
}

void Simulation::addParticles(glm::dvec2 pos)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    std::uniform_real_distribution<> dis_angle(0.0, 2.0 * M_PI);
    std::uniform_real_distribution<> dis_radius(0.0, 1.0);

    double radius = 0.01 * particleBrushSize / 2.0;

    Particle p;

    double x = 2.0 * pos.x / (double)WINDOW_WIDTH - 1.0;
    double y = 1.0 - 2.0 * pos.y / (double)WINDOW_HEIGHT;

    for (int i = 0; i < this->particleBrushSize; ++i)
    {
        Particle p;
        double angle = dis_angle(gen); // Random angle between 0 and 2*pi
        double r = radius * sqrt(dis_radius(gen)); // Random radius adjusted for area

        p.position = glm::dvec2(r * cos(angle) + x, r * sin(angle) + y); // circle fill

        p.velocity = glm::dvec2(this->newParticleVelocity, 0.0);
        p.color = calculateColor(p.velocity);  // Set initial color based on velocity
        p.mass = this->newParticleMass + 1.0;
        this->particles->push_back(p);
    }
}

void Simulation::removeParticle(glm::dvec2 pos)
{
    double x = 2.0 * pos.x / (double)WINDOW_WIDTH - 1.0;
    double y = 1.0 - 2.0 * pos.y / (double)WINDOW_HEIGHT;

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

void Simulation::removeAllParticles()
{
    this->particles->clear();
}