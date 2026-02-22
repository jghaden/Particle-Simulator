/**
  ******************************************************************************
  * @file    Particle.cpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    20 JAN 2025
  * @brief
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// Static member initialization
ParticleColorMode Particle::currentColorMode = ParticleColorMode::Velocity;
COLOR_GRADIENT_T Particle::currentGradient = Particle::GetIRtoUVGradient();
glm::dvec2 Particle::centerOfMass = glm::dvec2(0.0);

/* Private function prototypes -----------------------------------------------*/



/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/******************************************************************************/


/**
  * @brief  Default Particle constructor
  * @param  None
  * @retval None
  */
Particle::Particle()
{
    this->Init();
}


/**
  * @brief  Particle constructor
  * @param  mass
  * @param  position
  * @param  velocity
  * @retval None
  */
Particle::Particle(double mass, glm::dvec2 position, glm::dvec2 velocity)
{
    this->age = 0.0;
    this->mass = mass;
    this->acceleration = glm::dvec2(0.0);
    this->position = position;
    this->velocity = velocity;
    this->color = this->CalculateColor();
}


/**
  * @brief  Initialize particle variables if not set with constructor
  * @param  None
  * @retval None
  */
void Particle::Init()
{
    this->age = 0.0;
    this->mass = 1e8;
    this->acceleration = glm::dvec2(0.0);
    this->position = glm::dvec2(0.0);
    this->velocity = glm::dvec2(0.0);
    this->color = this->CalculateColor();
}


/**
  * @brief  Update particle properties
  * @param  None
  * @retval None
  */
void Particle::Update(double timeStep)
{
    velocity += acceleration * timeStep;
    velocity *= DAMPING_FACTOR;
    position += velocity * timeStep;
    acceleration = glm::dvec2(0.0);

    UpdateColor();
}


/**
  * @brief  Update particle color based on its speed
  * @param  None
  * @retval None
  */
void Particle::UpdateColor()
{
    // Only update color every N frames for performance (optimization)
    if (framesSinceColorUpdate >= COLOR_UPDATE_INTERVAL)
    {
        color = CalculateColor();
        framesSinceColorUpdate = 0;
    }
    else
    {
        framesSinceColorUpdate++;
    }
}


/**
  * @brief  Calculate particle color from gradient based on current color mode
  * @param  None
  * @retval glm::vec3
  */
glm::vec3 Particle::CalculateColor()
{
    double value = GetColorValue();
    float t = static_cast<float>(glm::clamp(value, 0.0, 1.0));

    if (t <= currentGradient.front().first) return currentGradient.front().second;
    if (t >= currentGradient.back().first) return currentGradient.back().second;

    for (size_t i = 1; i < currentGradient.size(); ++i)
    {
        if (t < currentGradient[i].first)
        {
            float localT = (t - currentGradient[i - 1].first) / (currentGradient[i].first - currentGradient[i - 1].first);
            return glm::mix(currentGradient[i - 1].second, currentGradient[i].second, localT);
        }
    }

    return currentGradient.back().second; // Should never reach here
}


/**
  * @brief  Get the value to use for coloring based on current color mode
  * @param  None
  * @retval double - normalized value between 0.0 and 1.0
  */
double Particle::GetColorValue() const
{
    switch (currentColorMode)
    {
        case ParticleColorMode::Velocity:
        {
            double speed = glm::length(this->velocity);
            return speed / MAX_PARTICLE_COLOR_SPEED;
        }

        case ParticleColorMode::Acceleration:
        {
            double accelMagnitude = glm::length(this->acceleration);
            constexpr double MAX_ACCEL = 100.0; // Adjust based on typical acceleration values
            return accelMagnitude / MAX_ACCEL;
        }

        case ParticleColorMode::Mass:
        {
            constexpr double MIN_MASS = 1e7;
            constexpr double MAX_MASS = 1e9;
            return glm::clamp((this->mass - MIN_MASS) / (MAX_MASS - MIN_MASS), 0.0, 1.0);
        }

        case ParticleColorMode::KineticEnergy:
        {
            double speed = glm::length(this->velocity);
            double kineticEnergy = 0.5 * this->mass * speed * speed;
            constexpr double MAX_KINETIC_ENERGY = 1e15; // Adjust based on typical values
            return kineticEnergy / MAX_KINETIC_ENERGY;
        }

        case ParticleColorMode::CoMDistance:
        {
            double distance = glm::length(this->position - centerOfMass);
            constexpr double MAX_DISTANCE = 1.0; // Normalized to viewport size
            return distance / MAX_DISTANCE;
        }

        case ParticleColorMode::Age:
        {
            constexpr double MAX_AGE = 1000.0; // Maximum age for color scaling (seconds)
            return this->age / MAX_AGE;
        }

        default:
            return 0.0;
    }
}


/**
  * @brief  Get the age of particle
  * @param  None
  * @retval double
  */
double Particle::GetAge() const
{
    return this->age;
}


/**
  * @brief  Get the mass of the particle
  * @param  None
  * @retval double
  */
double Particle::GetMass() const
{
    return this->mass;
}


/**
  * @brief  Get the acceleration of the particle
  * @param  None
  * @retval glm::dvec2
  */
glm::dvec2 Particle::GetAcceleration() const
{
    return this->acceleration;
}


/**
  * @brief  Get the position of the particle
  * @param  None
  * @retval glm::dvec2
  */
glm::dvec2 Particle::GetPosition() const
{
    return this->position;
}


/**
  * @brief  Get the velocity of the particle
  * @param  None
  * @retval glm::dvec2
  */
glm::dvec2 Particle::GetVelocity() const
{
    return this->velocity;
}


/**
  * @brief  Get the color of the particle
  * @param  None
  * @retval glm::vec3
  */
glm::vec3 Particle::GetColor() const
{
    return this->color;
}


/**
  * @brief  Set the acceleration of the particle
  * @param  acceleration
  * @retval None
  */
void Particle::SetAcceleration(glm::dvec2 acceleration)
{
    this->acceleration = acceleration;
}


/**
  * @brief  Set the age of the particle
  * @param  age
  * @retval None
  */
void Particle::SetAge(double age)
{
    this->age = age;
}


/**
  * @brief  Set the color of the particle
  * @param  color
  * @retval None
  */
void Particle::SetColor(glm::vec3 color)
{
    this->color = color;
}


/**
  * @brief  Set the mass of the particle
  * @param  mass
  * @retval None
  */
void Particle::SetMass(double mass)
{
    this->mass = mass;
}


/**
  * @brief  Set the position of the particle
  * @param  position
  * @retval None
  */
void Particle::SetPosition(glm::dvec2 position)
{
    this->position = position;
}


/**
  * @brief  Set the velocity of the particle
  * @param  velocity
  * @retval None
  */
void Particle::SetVelocity(glm::dvec2 velocity)
{
    this->velocity = velocity;
}


/**
  * @brief  Set the color mode for all particles
  * @param  mode - The color mode to use
  * @retval None
  */
void Particle::SetColorMode(ParticleColorMode mode)
{
    currentColorMode = mode;
}


/**
  * @brief  Get the current color mode
  * @param  None
  * @retval ParticleColorMode
  */
ParticleColorMode Particle::GetColorMode()
{
    return currentColorMode;
}


/**
  * @brief  Set the color gradient for all particles
  * @param  gradient - The color gradient to use
  * @retval None
  */
void Particle::SetColorGradient(const COLOR_GRADIENT_T& gradient)
{
    currentGradient = gradient;
}


/**
  * @brief  Get the current color gradient
  * @param  None
  * @retval const COLOR_GRADIENT_T&
  */
const COLOR_GRADIENT_T& Particle::GetCurrentGradient()
{
    return currentGradient;
}


/**
  * @brief  Get an IR-to-UV inspired gradient (deep red to bright violet)
  * @param  None
  * @retval COLOR_GRADIENT_T
  */
COLOR_GRADIENT_T Particle::GetIRtoUVGradient()
{
    return {
        {0.00f, glm::vec3(0.20f, 0.00f, 0.00f)},    // Deep dark red (near-IR)
        {0.15f, glm::vec3(0.80f, 0.10f, 0.00f)},    // Dark orange-red
        {0.30f, glm::vec3(1.00f, 0.40f, 0.00f)},    // Orange
        {0.45f, glm::vec3(1.00f, 0.90f, 0.00f)},    // Yellow-orange
        {0.60f, glm::vec3(0.00f, 0.80f, 1.00f)},    // Cyan
        {0.75f, glm::vec3(0.20f, 0.40f, 1.00f)},    // Blue
        {0.90f, glm::vec3(0.50f, 0.20f, 1.00f)},    // Violet
        {1.00f, glm::vec3(0.80f, 0.60f, 1.00f)}     // Bright blue-violet (near-UV)
    };
}


/**
  * @brief  Get the classic gradient (original color scheme)
  * @param  None
  * @retval COLOR_GRADIENT_T
  */
COLOR_GRADIENT_T Particle::GetClassicGradient()
{
    return {
        {0.0f,  glm::vec3(1.0f, 0.0f, 0.0f)},   // Red
        {0.25f, glm::vec3(1.0f, 1.0f, 0.0f)},   // Yellow
        {0.5f,  glm::vec3(0.0f, 1.0f, 0.0f)},   // Green
        {0.75f, glm::vec3(0.0f, 0.0f, 1.0f)},   // Blue
        {1.0f,  glm::vec3(1.0f, 0.0f, 0.8f)}    // Purple
    };
}


/**
  * @brief  Set the center of mass for all particles
  * @param  com - Center of mass position
  * @retval None
  */
void Particle::SetCenterOfMass(glm::dvec2 com)
{
    centerOfMass = com;
}


/**
  * @brief  Get the center of mass
  * @param  None
  * @retval glm::dvec2
  */
glm::dvec2 Particle::GetCenterOfMass()
{
    return centerOfMass;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
