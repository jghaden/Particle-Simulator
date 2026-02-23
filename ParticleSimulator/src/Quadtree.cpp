/**
  ******************************************************************************
  * @file    Quadtree.cpp
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

#include "Quadtree.hpp"

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
  * @brief  Initialize a QuadtreeNode (called by pool allocator each frame)
  * @param  cx    Center x coordinate
  * @param  cy    Center y coordinate
  * @param  hs    Half-size of the node
  * @retval None
  */
void QuadtreeNode::Init(double cx, double cy, double hs)
{
    this->centerOfMass  = glm::dvec2(0.0);
    this->centerX       = cx;
    this->centerY       = cy;
    this->halfSize      = hs;
    this->totalMass     = 0.0;
    this->particleCount = 0;
    this->nw = nullptr;
    this->ne = nullptr;
    this->sw = nullptr;
    this->se = nullptr;
}


/**
  * @brief  QuadtreeNodePool constructor — pre-allocates all nodes once
  * @retval None
  */
QuadtreeNodePool::QuadtreeNodePool()
{
    nodes.resize(POOL_MAX_NODES);
    nextIndex = 0;
}


/**
  * @brief  Allocate and initialize one node from the pool
  * @param  cx    Center x
  * @param  cy    Center y
  * @param  hs    Half-size
  * @retval Pointer to initialized node
  */
QuadtreeNode* QuadtreeNodePool::Allocate(double cx, double cy, double hs)
{
    assert(nextIndex < POOL_MAX_NODES && "QuadtreeNodePool exhausted — increase POOL_MAX_NODES");
    QuadtreeNode* node = &nodes[nextIndex++];
    node->Init(cx, cy, hs);
    return node;
}


/**
  * @brief  Reset the pool for reuse next frame (no destructors, O(1))
  * @retval None
  */
void QuadtreeNodePool::Reset()
{
    nextIndex = 0;
}


/**
  * @brief  Compute the total mass and center of mass of each node after building the quadtree
  * @param  particles Reference to particle data (SoA)
  * @retval None
  */
void QuadtreeNode::ComputeMassDistribution(const ParticleData& particles)
{
    // Leaf node with particles in bucket
    if (!nw && !ne && !sw && !se)
    {
        if (particleCount > 0)
        {
            double massSum = 0.0;
            glm::dvec2 weightedPosition(0.0);

            for (size_t k = 0; k < particleCount; ++k)
            {
                size_t idx = particleIndices[k];
                double mass = particles.masses[idx];
                massSum += mass;
                weightedPosition += particles.positions[idx] * mass;
            }

            totalMass = massSum;
            if (massSum > 0.0)
            {
                centerOfMass = weightedPosition / massSum;
            }
        }
        return;
    }

    // Otherwise, get total mass from all children nodes
    double massSum = 0.0;
    glm::dvec2 weightedPosition(0.0);

    if (nw)
    {
        nw->ComputeMassDistribution(particles);
        massSum += nw->totalMass;
        weightedPosition += nw->centerOfMass * nw->totalMass;
    }
    if (ne)
    {
        ne->ComputeMassDistribution(particles);
        massSum += ne->totalMass;
        weightedPosition += ne->centerOfMass * ne->totalMass;
    }
    if (sw)
    {
        sw->ComputeMassDistribution(particles);
        massSum += sw->totalMass;
        weightedPosition += sw->centerOfMass * sw->totalMass;
    }
    if (se)
    {
        se->ComputeMassDistribution(particles);
        massSum += se->totalMass;
        weightedPosition += se->centerOfMass * se->totalMass;
    }

    totalMass = massSum;
    if (massSum > 0.0)
    {
        centerOfMass = weightedPosition / massSum;
    }
}


/**
  * @brief  Insert a particle into the quadtree
  * @param  particleIndex Index of particle in ParticleData
  * @param  particles     Reference to particle data (SoA)
  * @param  pool          Node pool for allocating child nodes
  * @retval None
  */
void QuadtreeNode::Insert(size_t particleIndex, const ParticleData& particles, QuadtreeNodePool& pool)
{
    // If this is a leaf node and bucket has capacity, add to bucket
    if (!nw && !ne && !sw && !se)
    {
        if (particleCount < BUCKET_CAPACITY)
        {
            particleIndices[particleCount++] = particleIndex;
            return;
        }

        // Bucket is full — subdivide and redistribute existing particles
        Subdivide(pool);

        for (size_t k = 0; k < particleCount; ++k)
        {
            InsertIntoChild(particleIndices[k], particles, pool);
        }
        particleCount = 0;
    }

    // Insert the new particle into appropriate child
    InsertIntoChild(particleIndex, particles, pool);
}


/**
  * @brief  Insert a particle into a child node
  * @param  particleIndex Index of particle in ParticleData
  * @param  particles     Reference to particle data (SoA)
  * @param  pool          Node pool for allocating child nodes
  * @retval None
  */
void QuadtreeNode::InsertIntoChild(size_t particleIndex, const ParticleData& particles, QuadtreeNodePool& pool)
{
    double px = particles.positions[particleIndex].x;
    double py = particles.positions[particleIndex].y;

    if      (nw->Contains(px, py)) { nw->Insert(particleIndex, particles, pool); }
    else if (ne->Contains(px, py)) { ne->Insert(particleIndex, particles, pool); }
    else if (sw->Contains(px, py)) { sw->Insert(particleIndex, particles, pool); }
    else if (se->Contains(px, py)) { se->Insert(particleIndex, particles, pool); }
}


/**
  * @brief  Get neighboring particle indices at specific location and radius
  * @param  xMin
  * @param  yMin
  * @param  xMax
  * @param  yMax
  * @param  results Vector of particle indices
  * @retval None
  */
void QuadtreeNode::QueryRange(double xMin, double yMin, double xMax, double yMax, std::vector<size_t>& results)
{
    // If this node's region does not intersect the query box, skip entirely
    double nodeLeft   = centerX - halfSize;
    double nodeRight  = centerX + halfSize;
    double nodeBottom = centerY - halfSize;
    double nodeTop    = centerY + halfSize;

    // No overlap if the boxes are strictly outside each other
    if (xMax < nodeLeft || xMin > nodeRight ||
        yMax < nodeBottom || yMin > nodeTop)
    {
        return;
    }

    // If leaf node, return all particle indices in bucket
    if (!nw && !ne && !sw && !se)
    {
        for (size_t k = 0; k < particleCount; ++k)
        {
            results.push_back(particleIndices[k]);
        }
        return;
    }

    // Otherwise, recursively search children
    if (nw) nw->QueryRange(xMin, yMin, xMax, yMax, results);
    if (ne) ne->QueryRange(xMin, yMin, xMax, yMax, results);
    if (sw) sw->QueryRange(xMin, yMin, xMax, yMax, results);
    if (se) se->QueryRange(xMin, yMin, xMax, yMax, results);
}


/**
  * @brief  Subdivide this node into four children (allocated from pool)
  * @param  pool  Node pool
  * @retval None
  */
void QuadtreeNode::Subdivide(QuadtreeNodePool& pool)
{
    double quarterSize = halfSize / 2.0;
    nw = pool.Allocate(centerX - quarterSize, centerY + quarterSize, quarterSize);
    ne = pool.Allocate(centerX + quarterSize, centerY + quarterSize, quarterSize);
    sw = pool.Allocate(centerX - quarterSize, centerY - quarterSize, quarterSize);
    se = pool.Allocate(centerX + quarterSize, centerY - quarterSize, quarterSize);
}


/**
  * @brief  Check if a (x, y) point is inside this node's bounding box
  * @param  px
  * @param  py
  * @retval bool
  */
bool QuadtreeNode::Contains(double px, double py) const
{
    return (px >= (centerX - halfSize) && px < (centerX + halfSize) &&
            py >= (centerY - halfSize) && py < (centerY + halfSize));
}


/**
  * @brief  Compute forces between particles that share a node and use approximations for further away nodes
  * @param  particleIndex Index of particle to compute force for
  * @param  particles     Reference to particle data (SoA)
  * @param  node          Quadtree node
  * @param  theta         Barnes-Hut approximation threshold
  * @retval glm::dvec2    Force vector
  */
glm::dvec2 ComputeForceBarnesHut(size_t particleIndex, const ParticleData& particles, const QuadtreeNode* node, double theta)
{
    glm::dvec2 force(0.0);

    // If this node is empty, no force
    if (!node || node->totalMass <= 0.0)
        return force;

    // If leaf node with particles in bucket
    if (!node->nw && !node->ne && !node->sw && !node->se)
    {
        // Compute direct force with each particle in the bucket
        for (size_t k = 0; k < node->particleCount; ++k)
        {
            size_t idx = node->particleIndices[k];

            // Skip if it's the same particle
            if (idx == particleIndex)
                continue;

            glm::dvec2 dir = particles.positions[idx] - particles.positions[particleIndex];
            double dist2 = glm::dot(dir, dir);
            if (dist2 < MIN_INTERACTION_DISTANCE * MIN_INTERACTION_DISTANCE)
                dist2 = MIN_INTERACTION_DISTANCE * MIN_INTERACTION_DISTANCE;

            double softDist2 = dist2 + SOFTENING * SOFTENING;
            double invDist = 1.0 / std::sqrt(softDist2);

            double f = GRAVITATIONAL_CONSTANT * particles.masses[particleIndex] * particles.masses[idx] * invDist * invDist;
            force += f * glm::normalize(dir);
        }
        return force;
    }

    // Size of this region
    double dx = node->centerOfMass.x - particles.positions[particleIndex].x;
    double dy = node->centerOfMass.y - particles.positions[particleIndex].y;
    double dist = std::sqrt(dx * dx + dy * dy);

    // If region is far away, treat as single mass
    if ((node->halfSize * 2.0) / dist < theta)
    {
        // Use node's total mass
        if (dist < MIN_INTERACTION_DISTANCE)
            dist = MIN_INTERACTION_DISTANCE;

        double dist2 = dist * dist + SOFTENING * SOFTENING;
        double invDist = 1.0 / std::sqrt(dist2);

        double f = GRAVITATIONAL_CONSTANT * particles.masses[particleIndex] * node->totalMass * invDist * invDist;
        glm::dvec2 dir = glm::normalize(glm::dvec2(dx, dy));
        force += f * dir;
        return force;
    }
    else
    {
        // Otherwise, recurse into children
        force += ComputeForceBarnesHut(particleIndex, particles, node->nw, theta);
        force += ComputeForceBarnesHut(particleIndex, particles, node->ne, theta);
        force += ComputeForceBarnesHut(particleIndex, particles, node->sw, theta);
        force += ComputeForceBarnesHut(particleIndex, particles, node->se, theta);
    }
    return force;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
