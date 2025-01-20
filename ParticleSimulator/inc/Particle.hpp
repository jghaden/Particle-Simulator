/**
  ******************************************************************************
  * @file    Particle.hpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    20 JAN 2025
  * @brief   Header for Particle.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __PARTICLE_HPP
#define __PARTICLE_HPP

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

/* Exported types ----------------------------------------------------------- */

typedef std::vector<std::pair<float, glm::vec3>> COLOR_GRADIENT_T;

/* Exported constants ------------------------------------------------------- */
/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Forward declarations ----------------------------------------------------- */
/* Class definition --------------------------------------------------------- */

class Particle
{
public:
    /* Public member variables -------------------------------------------------- */
    /* Public member functions -------------------------------------------------- */

    Particle();
    Particle(double mass, glm::dvec2 position, glm::dvec2 velocity);

    void Init();
    void Update(double timeStep);
    void UpdateColor();
    glm::vec3 CalculateColor();

    /* Getters ------------------------------------------------------------------ */

    double GetAge() const;
    double GetMass() const;
    glm::dvec2 GetAcceleration() const;
    glm::dvec2 GetPosition() const;
    glm::dvec2 GetVelocity() const;
    glm::vec3 GetColor() const;

    /* Setters ------------------------------------------------------------------ */

    void SetAcceleration(glm::dvec2 acceleration);
    void SetAge(double age);
    void SetColor(glm::vec3 color);
    void SetMass(double mass);
    void SetPosition(glm::dvec2 position);
    void SetVelocity(glm::dvec2 velocity);
private:
    /* Private member variables ------------------------------------------------- */

    double     age;
    double     mass;
    glm::dvec2 acceleration;
    glm::dvec2 position;
    glm::dvec2 velocity;
    glm::vec3  color;

    /* Private member functions ------------------------------------------------- */
    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */
};



#endif /* __PARTICLE_HPP */

/******************************** END OF FILE *********************************/
