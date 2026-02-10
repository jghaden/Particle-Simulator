/**
  ******************************************************************************
  * @file    ParticleData.hpp
  * @author  Josh Haden
  * @version V0.2.0
  * @date    05 FEB 2026
  * @brief   Structure of Arrays (SoA) particle data layout for cache optimization
  ******************************************************************************
  * @attention
  *
  * This replaces the traditional Array of Structures (AoS) Particle class
  * with a Structure of Arrays layout for better cache utilization and
  * SIMD vectorization potential.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __PARTICLEDATA_HPP
#define __PARTICLEDATA_HPP

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

/**
 * @brief Structure of Arrays (SoA) for particle data
 *
 * Instead of storing particles as individual objects with all properties,
 * we store each property in separate contiguous arrays. This improves:
 * - Cache utilization (only load data needed for each operation)
 * - Memory access patterns (sequential reads/writes)
 * - SIMD vectorization potential (operate on multiple particles at once)
 */
class ParticleData
{
public:
    /* Public member variables -------------------------------------------------- */

    // Separate arrays for each particle property
    std::vector<double>     ages;
    std::vector<double>     masses;
    std::vector<glm::dvec2> accelerations;
    std::vector<glm::dvec2> positions;
    std::vector<glm::dvec2> velocities;
    std::vector<glm::vec3>  colors;

    // Color update caching (optimization)
    std::vector<int> framesSinceColorUpdate;
    static constexpr int COLOR_UPDATE_INTERVAL = 5;

    /* Public member functions -------------------------------------------------- */

    ParticleData();

    /**
     * @brief Add a new particle with the given properties
     * @param mass Particle mass
     * @param position Initial position
     * @param velocity Initial velocity
     * @retval Index of the newly added particle
     */
    size_t AddParticle(double mass, glm::dvec2 position, glm::dvec2 velocity);

    /**
     * @brief Remove particle at given index using swap-and-pop
     * @param index Index of particle to remove
     * @retval None
     * @note This changes the index of the last particle to the removed index
     */
    void RemoveParticle(size_t index);

    /**
     * @brief Clear all particles
     * @param None
     * @retval None
     */
    void Clear();

    /**
     * @brief Reserve capacity for a given number of particles
     * @param capacity Number of particles to reserve space for
     * @retval None
     */
    void Reserve(size_t capacity);

    /**
     * @brief Get the number of particles
     * @param None
     * @retval size_t Number of particles
     */
    size_t Size() const;

    /**
     * @brief Update a single particle's physics
     * @param index Particle index
     * @param timeStep Simulation time step
     * @retval None
     */
    void UpdateParticle(size_t index, double timeStep);

    /**
     * @brief Update particle color based on velocity
     * @param index Particle index
     * @retval None
     */
    void UpdateColor(size_t index);

    /**
     * @brief Calculate color from gradient based on speed
     * @param index Particle index
     * @retval glm::vec3 Color
     */
    glm::vec3 CalculateColor(size_t index) const;

    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */

private:
    /* Private member variables ------------------------------------------------- */
    /* Private member functions ------------------------------------------------- */
    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */
};



#endif /* __PARTICLEDATA_HPP */

/******************************** END OF FILE *********************************/
