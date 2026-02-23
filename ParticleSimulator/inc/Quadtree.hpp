/**
  ******************************************************************************
  * @file    Quadtree.hpp
  * @author  Josh Haden
  * @version V0.1.0
  * @date    20 JAN 2025
  * @brief   Header for Quadtree.cpp
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __QUADTREE_HPP
#define __QUADTREE_HPP

/* Includes ----------------------------------------------------------------- */

#include "PCH.hpp"

#include "Simulation.hpp"
#include "Particle.hpp"
#include "ParticleData.hpp"

/* Exported types ----------------------------------------------------------- */
/* Exported constants ------------------------------------------------------- */

constexpr double THETA = 2.0;                               // Threshold distance to calculate long-range force (lower = more accurate, higher = faster)
constexpr size_t BUCKET_CAPACITY = 8;                       // Maximum particles per leaf node before subdivision
constexpr size_t POOL_MAX_NODES = MAX_NUM_PARTICLES * 4;    // Pre-allocated node pool size (generous upper bound)

/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Forward declarations ----------------------------------------------------- */

struct QuadtreeNodePool;

/* Class definition --------------------------------------------------------- */

struct QuadtreeNode
{
    /* Public member variables -------------------------------------------------- */

    double     centerX;                         // Center x coordinate of the node
    double     centerY;                         // Center y coordinate of the node
    double     halfSize;                        // Half the width/height of the node
    double     totalMass;                       // Total mass of particles in this node
    glm::dvec2 centerOfMass;                    // Center of mass for all particles in this node
    size_t     particleIndices[BUCKET_CAPACITY];// Fixed-size bucket (no heap allocation per node)
    size_t     particleCount;                   // Number of particles currently in bucket

    // Children nodes (raw pointers into pool — no ownership, no heap alloc)
    QuadtreeNode* nw;   // north west
    QuadtreeNode* ne;   // north east
    QuadtreeNode* sw;   // south west
    QuadtreeNode* se;   // south east


    /* Public member functions -------------------------------------------------- */

    void Init(double centerX, double centerY, double halfSize);

    void ComputeMassDistribution(const ParticleData& particles);
    void Insert(size_t particleIndex, const ParticleData& particles, QuadtreeNodePool& pool);
    void InsertIntoChild(size_t particleIndex, const ParticleData& particles, QuadtreeNodePool& pool);
    void QueryRange(double xMin, double yMin, double xMax, double yMax, std::vector<size_t>& results);
    void Subdivide(QuadtreeNodePool& pool);
    bool Contains(double px, double py) const;

    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */
};


/* Pool allocator for QuadtreeNodes — pre-allocated, reset each frame       */
struct QuadtreeNodePool
{
    std::vector<QuadtreeNode> nodes;    // Contiguous block, never resized after init
    size_t                    nextIndex; // Index of next free node

    QuadtreeNodePool();

    QuadtreeNode* Allocate(double cx, double cy, double hs);
    void          Reset();
};


glm::dvec2 ComputeForceBarnesHut(size_t particleIndex, const ParticleData& particles, const QuadtreeNode* node, double theta);



#endif /* __QUADTREE_HPP */

/******************************** END OF FILE *********************************/
