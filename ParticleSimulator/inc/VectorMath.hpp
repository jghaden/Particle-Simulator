/**
  ******************************************************************************
  * @file    VectorMath.hpp
  * @author  Josh Haden
  * @version V0.3.0
  * @date    09 FEB 2026
  * @brief   SIMD vectorized math operations for particle physics
  ******************************************************************************
  * @attention
  *
  * glm::dvec2 stores x and y contiguously (16 bytes per element).
  * Two consecutive dvec2 values occupy exactly 32 bytes — one AVX2 register.
  * This allows direct _mm256_loadu_pd loads with no gather or temp arrays.
  *
  * Layout in memory:
  *   positions: [x0, y0, x1, y1, x2, y2, x3, y3, ...]
  *
  * One _mm256_loadu_pd at &positions[i].x loads [xi, yi, x(i+1), y(i+1)].
  * Two such loads per operation handle 4 particles per SIMD iteration.
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

// Processes 4 particles per iteration (2 particles per 256-bit AVX2 register)
constexpr size_t SIMD_WIDTH = 4;

/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * @brief Update particle velocities and positions using AVX2 SIMD
 * @param particleData Reference to particle data (SoA)
 * @param startIdx     Starting particle index (must leave room for SIMD_WIDTH particles)
 * @param count        Number of particles to process (must be a multiple of SIMD_WIDTH)
 * @param timeStep     Simulation time step
 * @retval None
 *
 * Processes 4 particles per loop iteration using two 256-bit AVX2 loads.
 * Each _mm256_loadu_pd reads two consecutive dvec2 values (x_i, y_i, x_{i+1}, y_{i+1})
 * directly from the vector — no temporary arrays or gather operations.
 *
 * Per iteration:
 *   vel += acc * dt    (fmadd x2)
 *   vel *= damping     (mul   x2)
 *   pos += vel * dt    (fmadd x2)
 *   acc  = 0           (store x2)
 */
inline void UpdateParticlesSimd(ParticleData& particleData, size_t startIdx, size_t count, double timeStep)
{
    const __m256d dt   = _mm256_set1_pd(timeStep);
    const __m256d damp = _mm256_set1_pd(DAMPING_FACTOR);
    const __m256d zero = _mm256_setzero_pd();

    for (size_t i = startIdx; i < startIdx + count; i += SIMD_WIDTH)
    {
        // Load [vx_i, vy_i, vx_{i+1}, vy_{i+1}] and [vx_{i+2}, vy_{i+2}, vx_{i+3}, vy_{i+3}]
        __m256d vel01 = _mm256_loadu_pd(&particleData.velocities[i + 0].x);
        __m256d vel23 = _mm256_loadu_pd(&particleData.velocities[i + 2].x);

        // Load accelerations for 4 particles
        __m256d acc01 = _mm256_loadu_pd(&particleData.accelerations[i + 0].x);
        __m256d acc23 = _mm256_loadu_pd(&particleData.accelerations[i + 2].x);

        // vel += acc * dt
        vel01 = _mm256_fmadd_pd(acc01, dt, vel01);
        vel23 = _mm256_fmadd_pd(acc23, dt, vel23);

        // vel *= damping
        vel01 = _mm256_mul_pd(vel01, damp);
        vel23 = _mm256_mul_pd(vel23, damp);

        // Store updated velocities
        _mm256_storeu_pd(&particleData.velocities[i + 0].x, vel01);
        _mm256_storeu_pd(&particleData.velocities[i + 2].x, vel23);

        // Load positions for 4 particles
        __m256d pos01 = _mm256_loadu_pd(&particleData.positions[i + 0].x);
        __m256d pos23 = _mm256_loadu_pd(&particleData.positions[i + 2].x);

        // pos += vel * dt
        pos01 = _mm256_fmadd_pd(vel01, dt, pos01);
        pos23 = _mm256_fmadd_pd(vel23, dt, pos23);

        // Store updated positions
        _mm256_storeu_pd(&particleData.positions[i + 0].x, pos01);
        _mm256_storeu_pd(&particleData.positions[i + 2].x, pos23);

        // Reset accelerations to zero
        _mm256_storeu_pd(&particleData.accelerations[i + 0].x, zero);
        _mm256_storeu_pd(&particleData.accelerations[i + 2].x, zero);
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
