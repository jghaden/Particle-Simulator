/**
  ******************************************************************************
  * @file    Particle.hpp
  * @author  Josh Haden
  * @version V0.0.1
  * @date    18 JAN 2025
  * @brief   Header for Particle.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PARTICLE_HPP
#define __PARTICLE_HPP

/* Includes ------------------------------------------------------------------*/

#include "PCH.hpp"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

glm::vec3 interpolateColor(float t, const std::vector<std::pair<float, glm::vec3>>& colorStops);
glm::vec3 calculateColor(const glm::dvec2& velocity);

/* Class definition --------------------------------------------------------- */

class Particle
{
public:
    glm::dvec2 position;
    glm::dvec2 velocity;
    glm::dvec2 acceleration;
    glm::vec3 color;
    double mass;
    float age;

    Particle();
    Particle(glm::dvec2 position, glm::dvec2 velocity, double mass);

    glm::vec3 interpolateColor(float t, const std::vector<std::pair<float, glm::vec3>>& colorStops);

    void updateColor();

    void update();
};



#endif /* __PARTICLE_HPP */

/************************END OF FILE************************/
