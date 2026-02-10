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
#include "Simulation.hpp"

/* Global variables ----------------------------------------------------------*/

// Color gradient for particle visualization (defined in Particle.cpp originally)
extern COLOR_GRADIENT_T colorGradient;

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
  * @brief  Calculate particle color from gradient based on speed
  * @param  index
  * @retval glm::vec3
  */
glm::vec3 ParticleData::CalculateColor(size_t index) const
{
    double speed = glm::length(velocities[index]);
    float t = static_cast<float>(glm::clamp(speed / MAX_PARTICLE_COLOR_SPEED, 0.0, 1.0));

    if (t <= colorGradient.front().first) return colorGradient.front().second;
    if (t >= colorGradient.back().first) return colorGradient.back().second;

    for (size_t i = 1; i < colorGradient.size(); ++i)
    {
        if (t < colorGradient[i].first)
        {
            float localT = (t - colorGradient[i - 1].first) / (colorGradient[i].first - colorGradient[i - 1].first);
            return glm::mix(colorGradient[i - 1].second, colorGradient[i].second, localT);
        }
    }

    return colorGradient.back().second;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
