/**
  ******************************************************************************
  * @file    VectorMath.hpp
  * @author  Josh Haden
  * @version V0.2.0
  * @date    09 FEB 2026
  * @brief   SIMD vectorized math operations for particle physics
  ******************************************************************************
  * @attention
  *
  * This file provides AVX2-optimized vector operations for processing
  * multiple particles simultaneously. Requires CPU with AVX2 support.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __VECTORMATH_HPP
#define __VECTORMATH_HPP

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"
#include <immintrin.h>  // AVX2 intrinsics

#include "ParticleData.hpp"
#include "Simulation.hpp"

/* Exported types ----------------------------------------------------------- */
/* Exported constants ------------------------------------------------------- */

// SIMD lane width for AVX2 (processes 4 doubles at once)
constexpr size_t SIMD_WIDTH = 4;

/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Update particle velocities and positions using AVX2 SIMD
 * @param particleData Reference to particle data (SoA)
 * @param startIdx Starting particle index
 * @param count Number of particles to process (should be multiple of 4)
 * @param timeStep Simulation time step
 * @retval None
 *
 * This function processes 4 particles at once using AVX2 instructions.
 * It performs: velocity += acceleration * dt, position += velocity * dt
 */
inline void UpdateParticlesSimd(ParticleData& particleData, size_t startIdx, size_t count, double timeStep)
{
    // Load constants into SIMD registers
    __m256d dt = _mm256_set1_pd(timeStep);
    __m256d dampingFactor = _mm256_set1_pd(DAMPING_FACTOR);

    // Process 4 particles at a time (AVX2 = 256 bits / 64 bits per double = 4 doubles)
    for (size_t i = startIdx; i < startIdx + count; i += SIMD_WIDTH)
    {
        // Load velocities (x components for 4 particles)
        __m256d velX = _mm256_loadu_pd(&particleData.velocities[i].x);
        // Load accelerations (x components for 4 particles)
        // Note: We need to gather x components which are strided by sizeof(glm::dvec2)

        // For simplicity with glm::dvec2 arrays, we'll process x and y separately
        // Load 4 velocity.x values
        double velX_array[4] = {
            particleData.velocities[i + 0].x,
            particleData.velocities[i + 1].x,
            particleData.velocities[i + 2].x,
            particleData.velocities[i + 3].x
        };
        velX = _mm256_loadu_pd(velX_array);

        // Load 4 acceleration.x values
        double accX_array[4] = {
            particleData.accelerations[i + 0].x,
            particleData.accelerations[i + 1].x,
            particleData.accelerations[i + 2].x,
            particleData.accelerations[i + 3].x
        };
        __m256d accX = _mm256_loadu_pd(accX_array);

        // velocity.x += acceleration.x * dt
        velX = _mm256_fmadd_pd(accX, dt, velX);

        // velocity.x *= damping
        velX = _mm256_mul_pd(velX, dampingFactor);

        // Store updated velocities.x
        double newVelX[4];
        _mm256_storeu_pd(newVelX, velX);
        for (int j = 0; j < 4; ++j)
        {
            particleData.velocities[i + j].x = newVelX[j];
        }

        // Repeat for Y component
        double velY_array[4] = {
            particleData.velocities[i + 0].y,
            particleData.velocities[i + 1].y,
            particleData.velocities[i + 2].y,
            particleData.velocities[i + 3].y
        };
        __m256d velY = _mm256_loadu_pd(velY_array);

        double accY_array[4] = {
            particleData.accelerations[i + 0].y,
            particleData.accelerations[i + 1].y,
            particleData.accelerations[i + 2].y,
            particleData.accelerations[i + 3].y
        };
        __m256d accY = _mm256_loadu_pd(accY_array);

        velY = _mm256_fmadd_pd(accY, dt, velY);
        velY = _mm256_mul_pd(velY, dampingFactor);

        double newVelY[4];
        _mm256_storeu_pd(newVelY, velY);
        for (int j = 0; j < 4; ++j)
        {
            particleData.velocities[i + j].y = newVelY[j];
        }

        // Update positions: position += velocity * dt
        double posX_array[4] = {
            particleData.positions[i + 0].x,
            particleData.positions[i + 1].x,
            particleData.positions[i + 2].x,
            particleData.positions[i + 3].x
        };
        __m256d posX = _mm256_loadu_pd(posX_array);

        velX = _mm256_loadu_pd(newVelX);  // Use updated velocity
        posX = _mm256_fmadd_pd(velX, dt, posX);

        double newPosX[4];
        _mm256_storeu_pd(newPosX, posX);
        for (int j = 0; j < 4; ++j)
        {
            particleData.positions[i + j].x = newPosX[j];
        }

        // Y positions
        double posY_array[4] = {
            particleData.positions[i + 0].y,
            particleData.positions[i + 1].y,
            particleData.positions[i + 2].y,
            particleData.positions[i + 3].y
        };
        __m256d posY = _mm256_loadu_pd(posY_array);

        velY = _mm256_loadu_pd(newVelY);  // Use updated velocity
        posY = _mm256_fmadd_pd(velY, dt, posY);

        double newPosY[4];
        _mm256_storeu_pd(newPosY, posY);
        for (int j = 0; j < 4; ++j)
        {
            particleData.positions[i + j].y = newPosY[j];
        }

        // Reset accelerations to zero
        for (int j = 0; j < 4; ++j)
        {
            particleData.accelerations[i + j] = glm::dvec2(0.0);
        }
    }
}

/**
 * @brief Check if CPU supports AVX2
 * @param None
 * @retval bool True if AVX2 is supported
 */
inline bool HasAvx2Support()
{
#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuid(cpuInfo, 0);
    int nIds = cpuInfo[0];

    if (nIds >= 7)
    {
        __cpuidex(cpuInfo, 7, 0);
        return (cpuInfo[1] & (1 << 5)) != 0;  // Check AVX2 bit
    }
#endif
    return false;
}

/* Forward declarations ----------------------------------------------------- */
/* Class definition --------------------------------------------------------- */



#endif /* __VECTORMATH_HPP */

/******************************** END OF FILE *********************************/
