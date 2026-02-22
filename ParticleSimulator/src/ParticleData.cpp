/**
  ******************************************************************************
  * @file    ParticleData.cpp
  * @author  Josh Haden
  * @version V0.2.0
  * @date    05 FEB 2026
  * @brief   Implementation of Structure of Arrays particle data
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

#include "ParticleData.hpp"
#include "Particle.hpp"
#include "Simulation.hpp"

/* Global variables ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/



/******************************************************************************/
/******************************************************************************/
/* Public Functions                                                           */
/******************************************************************************/
/******************************************************************************/


/**
  * @brief  ParticleData constructor
  * @param  None
  * @retval None
  */
ParticleData::ParticleData()
{
    // Constructor - arrays are empty by default
}


/**
  * @brief  Add a new particle to the data structure
  * @param  mass
  * @param  position
  * @param  velocity
  * @retval size_t - Index of the newly added particle
  */
size_t ParticleData::AddParticle(double mass, glm::dvec2 position, glm::dvec2 velocity)
{
    ages.push_back(0.0);
    masses.push_back(mass);
    accelerations.push_back(glm::dvec2(0.0));
    positions.push_back(position);
    velocities.push_back(velocity);
    colors.push_back(glm::vec3(1.0f));
    framesSinceColorUpdate.push_back(0);

    size_t index = positions.size() - 1;
    UpdateColor(index);  // Calculate initial color
    return index;
}


/**
  * @brief  Remove particle at given index using swap-and-pop
  * @param  index
  * @retval None
  */
void ParticleData::RemoveParticle(size_t index)
{
    if (index >= positions.size()) return;

    size_t lastIndex = positions.size() - 1;

    if (index != lastIndex)
    {
        // Swap with last element
        ages[index] = ages[lastIndex];
        masses[index] = masses[lastIndex];
        accelerations[index] = accelerations[lastIndex];
        positions[index] = positions[lastIndex];
        velocities[index] = velocities[lastIndex];
        colors[index] = colors[lastIndex];
        framesSinceColorUpdate[index] = framesSinceColorUpdate[lastIndex];
    }

    // Remove last element
    ages.pop_back();
    masses.pop_back();
    accelerations.pop_back();
    positions.pop_back();
    velocities.pop_back();
    colors.pop_back();
    framesSinceColorUpdate.pop_back();
}


/**
  * @brief  Clear all particles
  * @param  None
  * @retval None
  */
void ParticleData::Clear()
{
    ages.clear();
    masses.clear();
    accelerations.clear();
    positions.clear();
    velocities.clear();
    colors.clear();
    framesSinceColorUpdate.clear();
}


/**
  * @brief  Reserve capacity for particles
  * @param  capacity
  * @retval None
  */
void ParticleData::Reserve(size_t capacity)
{
    ages.reserve(capacity);
    masses.reserve(capacity);
    accelerations.reserve(capacity);
    positions.reserve(capacity);
    velocities.reserve(capacity);
    colors.reserve(capacity);
    framesSinceColorUpdate.reserve(capacity);
}


/**
  * @brief  Get number of particles
  * @param  None
  * @retval size_t
  */
size_t ParticleData::Size() const
{
    return positions.size();
}


/**
  * @brief  Update a single particle's physics
  * @param  index
  * @param  timeStep
  * @retval None
  */
void ParticleData::UpdateParticle(size_t index, double timeStep)
{
    ages[index] += timeStep;
    velocities[index] += accelerations[index] * timeStep;
    velocities[index] *= DAMPING_FACTOR;
    positions[index] += velocities[index] * timeStep;
    accelerations[index] = glm::dvec2(0.0);

    UpdateColor(index);
}


/**
  * @brief  Update particle color based on velocity
  * @param  index
  * @retval None
  */
void ParticleData::UpdateColor(size_t index)
{
    // Only update color every N frames for performance (optimization)
    if (framesSinceColorUpdate[index] >= COLOR_UPDATE_INTERVAL)
    {
        colors[index] = CalculateColor(index);
        framesSinceColorUpdate[index] = 0;
    }
    else
    {
        framesSinceColorUpdate[index]++;
    }
}


/**
  * @brief  Calculate color from gradient based on current color mode
  * @param  index
  * @retval glm::vec3
  */
glm::vec3 ParticleData::CalculateColor(size_t index) const
{
    // Get the color value based on current mode
    double value = 0.0;
    ParticleColorMode mode = Particle::GetColorMode();

    switch (mode)
    {
        case ParticleColorMode::Velocity:
        {
            double speed = glm::length(velocities[index]);
            value = speed / MAX_PARTICLE_COLOR_SPEED;
            break;
        }

        case ParticleColorMode::Acceleration:
        {
            double accelMagnitude = glm::length(accelerations[index]);
            constexpr double MAX_ACCEL = 100.0;
            value = accelMagnitude / MAX_ACCEL;
            break;
        }

        case ParticleColorMode::Mass:
        {
            constexpr double MIN_MASS = 1e7;
            constexpr double MAX_MASS = 1e9;
            value = glm::clamp((masses[index] - MIN_MASS) / (MAX_MASS - MIN_MASS), 0.0, 1.0);
            break;
        }

        case ParticleColorMode::KineticEnergy:
        {
            double speed = glm::length(velocities[index]);
            double kineticEnergy = 0.5 * masses[index] * speed * speed;
            constexpr double MAX_KINETIC_ENERGY = 1e15;
            value = kineticEnergy / MAX_KINETIC_ENERGY;
            break;
        }

        case ParticleColorMode::CoMDistance:
        {
            double distance = glm::length(positions[index] - Particle::GetCenterOfMass());
            constexpr double MAX_DISTANCE = 1.0; // Normalized to viewport size
            value = distance / MAX_DISTANCE;
            break;
        }

        case ParticleColorMode::Age:
        {
            constexpr double MAX_AGE = 1000.0;
            value = ages[index] / MAX_AGE;
            break;
        }
    }

    // Use the static gradient from Particle class
    float t = static_cast<float>(glm::clamp(value, 0.0, 1.0));
    const COLOR_GRADIENT_T& currentGradient = Particle::GetCurrentGradient();

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

    return currentGradient.back().second;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
