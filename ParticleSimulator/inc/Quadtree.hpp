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

constexpr double THETA = 2.0;   // Threshold distance to calculate long-range force (lower = more accurate, higher = faster)
constexpr size_t BUCKET_CAPACITY = 8;  // Maximum particles per leaf node before subdivision

/* Exported macro ----------------------------------------------------------- */
/* Exported variables ------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */
/* Forward declarations ----------------------------------------------------- */
/* Class definition --------------------------------------------------------- */

struct QuadtreeNode
{
    /* Public member variables -------------------------------------------------- */

    double     centerX;                 // Center x coordinate of the node
    double     centerY;                 // Center y coordinate of the node
    double     halfSize;                // Half the width/height of the node
    double     totalMass;               // Total mass of particles in this node
    glm::dvec2 centerOfMass;            // Center of mass for all particles in this node
    std::vector<size_t> particleIndices;   // If leaf node, store up to BUCKET_CAPACITY particle indices

    // Children nodes
    std::unique_ptr<QuadtreeNode> nw;   // north west
    std::unique_ptr<QuadtreeNode> ne;   // north east
    std::unique_ptr<QuadtreeNode> sw;   // south west
    std::unique_ptr<QuadtreeNode> se;   // south east


    /* Public member functions -------------------------------------------------- */

    QuadtreeNode(double centerX, double centerY, double halfSize);

    void ComputeMassDistribution(const ParticleData& particles);
    void Insert(size_t particleIndex, const ParticleData& particles);
    void InsertIntoChild(size_t particleIndex, const ParticleData& particles);
    void QueryRange(double xMin, double yMin, double xMax, double yMax, std::vector<size_t>& results);
    void Subdivide();
    bool Contains(double px, double py) const;

    /* Getters ------------------------------------------------------------------ */
    /* Setters ------------------------------------------------------------------ */
};


glm::dvec2 ComputeForceBarnesHut(size_t particleIndex, const ParticleData& particles, const QuadtreeNode* node, double theta);



#endif /* __QUADTREE_HPP */

/******************************** END OF FILE *********************************/
