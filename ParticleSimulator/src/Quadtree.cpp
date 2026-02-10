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
  * @brief  QuadtreeNode constructor
  * @param  centerX
  * @param  centerY
  * @param  halfSize
  * @retval None
  */
QuadtreeNode::QuadtreeNode(double centerX, double centerY, double halfSize)
{
    this->centerOfMass = glm::dvec2(0.0);
    this->centerX      = centerX;
    this->centerY      = centerY;
    this->halfSize     = halfSize;
    this->totalMass    = 0.0;
    this->particleIndices.reserve(BUCKET_CAPACITY);
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
        if (!particleIndices.empty())
        {
            double massSum = 0.0;
            glm::dvec2 weightedPosition(0.0);

            for (size_t idx : particleIndices)
            {
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


// Insert a particle into the quadtree
/**
  * @brief  Insert a particle into the quadtree
  * @param  particleIndex Index of particle in ParticleData
  * @param  particles Reference to particle data (SoA)
  * @retval None
  */
void QuadtreeNode::Insert(size_t particleIndex, const ParticleData& particles)
{
    double px = particles.positions[particleIndex].x;
    double py = particles.positions[particleIndex].y;

    // If this is a leaf node and bucket has capacity, add to bucket
    if (!nw && !ne && !sw && !se)
    {
        if (particleIndices.size() < BUCKET_CAPACITY)
        {
            particleIndices.push_back(particleIndex);
            return;
        }

        // Bucket is full, must subdivide
        Subdivide();

        // Re-insert all existing particles in the bucket
        for (size_t idx : particleIndices)
        {
            InsertIntoChild(idx, particles);
        }
        particleIndices.clear();
    }

    // Insert the new particle into appropriate child
    InsertIntoChild(particleIndex, particles);
}


// Insert a particle into a child node
/**
  * @brief  Insert a particle into a child node
  * @param  particleIndex Index of particle in ParticleData
  * @param  particles Reference to particle data (SoA)
  * @retval None
  */
void QuadtreeNode::InsertIntoChild(size_t particleIndex, const ParticleData& particles)
{
    double px = particles.positions[particleIndex].x;
    double py = particles.positions[particleIndex].y;

    if (nw->Contains(px, py)) { nw->Insert(particleIndex, particles); }
    else if (ne->Contains(px, py)) { ne->Insert(particleIndex, particles); }
    else if (sw->Contains(px, py)) { sw->Insert(particleIndex, particles); }
    else if (se->Contains(px, py)) { se->Insert(particleIndex, particles); }
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
    double nodeLeft = centerX - halfSize;
    double nodeRight = centerX + halfSize;
    double nodeBottom = centerY - halfSize;
    double nodeTop = centerY + halfSize;

    // No overlap if the boxes are strictly outside each other
    if (xMax < nodeLeft || xMin > nodeRight ||
        yMax < nodeBottom || yMin > nodeTop)
    {
        return;
    }

    // If leaf node, return all particle indices in bucket
    // Note: We don't filter by exact position here for performance,
    // the caller can do that if needed
    if (!nw && !ne && !sw && !se)
    {
        results.insert(results.end(), particleIndices.begin(), particleIndices.end());
        return;
    }

    // Otherwise, recursively search children
    if (nw) nw->QueryRange(xMin, yMin, xMax, yMax, results);
    if (ne) ne->QueryRange(xMin, yMin, xMax, yMax, results);
    if (sw) sw->QueryRange(xMin, yMin, xMax, yMax, results);
    if (se) se->QueryRange(xMin, yMin, xMax, yMax, results);
}

/**
  * @brief  Subdivide this node into four children
  * @param  None
  * @retval None
  */
void QuadtreeNode::Subdivide()
{
    double quarterSize = halfSize / 2.0;
    nw = std::make_unique<QuadtreeNode>(centerX - quarterSize, centerY + quarterSize, quarterSize);
    ne = std::make_unique<QuadtreeNode>(centerX + quarterSize, centerY + quarterSize, quarterSize);
    sw = std::make_unique<QuadtreeNode>(centerX - quarterSize, centerY - quarterSize, quarterSize);
    se = std::make_unique<QuadtreeNode>(centerX + quarterSize, centerY - quarterSize, quarterSize);
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
  * @param  particles Reference to particle data (SoA)
  * @param  node Quadtree node
  * @param  theta Barnes-Hut approximation threshold
  * @retval glm::dvec2 Force vector
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
        for (size_t idx : node->particleIndices)
        {
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
        glm::dvec2 dir(dx, dy);
        force += f * (dir / dist);
        return force;
    }
    else
    {
        // Otherwise, recurse into children
        force += ComputeForceBarnesHut(particleIndex, particles, node->nw.get(), theta);
        force += ComputeForceBarnesHut(particleIndex, particles, node->ne.get(), theta);
        force += ComputeForceBarnesHut(particleIndex, particles, node->sw.get(), theta);
        force += ComputeForceBarnesHut(particleIndex, particles, node->se.get(), theta);
    }
    return force;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
