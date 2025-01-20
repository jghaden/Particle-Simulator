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

COLOR_GRADIENT_T colorGradient = {
    {0.0f,  glm::vec3(1.0f, 0.0f, 0.0f)},   // Red
    {0.25f, glm::vec3(1.0f, 1.0f, 0.0f)},   // Yellow
    {0.5f,  glm::vec3(0.0f, 1.0f, 0.0f)},   // Green
    {0.75f, glm::vec3(0.0f, 0.0f, 1.0f)},   // Blue
    {1.0f,  glm::vec3(1.0f, 0.0f, 0.8f)}    // Purple
};

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
void Particle::Update()
{
    position += velocity * TIME_STEP;
    velocity *= DAMPING_FACTOR;

    UpdateColor();
}


/**
  * @brief  Update particle color based on its speed
  * @param  None
  * @retval None
  */
void Particle::UpdateColor()
{
    color = CalculateColor();
}


/**
  * @brief  Calculate particle color from gradient based on its speed
  * @param  None
  * @retval None
  */
glm::vec3 Particle::CalculateColor()
{
    double speed = glm::length(this->velocity);
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

    return colorGradient.back().second; // Should never reach here
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



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
