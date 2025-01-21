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
    this->particle     = nullptr;
    this->totalMass    = 0.0;
}


/**
  * @brief  Compute the total mass and center of mass of each node after building the quadtree
  * @param  None
  * @retval None
  */
void QuadtreeNode::ComputeMassDistribution()
{
    // Leaf with a single particle
    if (!nw && !ne && !sw && !se)
    {
        if (particle)
        {
            totalMass = particle->GetMass();
            centerOfMass = particle->GetPosition();
        }
        return;
    }

    // Otherwise, get total mass from all children nodes
    double massSum = 0.0;
    glm::dvec2 weightedPosition(0.0);

    if (nw)
    {
        nw->ComputeMassDistribution();
        massSum += nw->totalMass;
        weightedPosition += nw->centerOfMass * nw->totalMass;
    }
    if (ne)
    {
        ne->ComputeMassDistribution();
        massSum += ne->totalMass;
        weightedPosition += ne->centerOfMass * ne->totalMass;
    }
    if (sw)
    {
        sw->ComputeMassDistribution();
        massSum += sw->totalMass;
        weightedPosition += sw->centerOfMass * sw->totalMass;
    }
    if (se)
    {
        se->ComputeMassDistribution();
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
  * @param  particle
  * @retval None
  */
void QuadtreeNode::Insert(Particle* particle)
{
    double px = particle->GetPosition().x;
    double py = particle->GetPosition().y;

    // If this node has no child and no particle stored, store it here
    if (!this->particle && !nw && !ne && !sw && !se)
    {
        this->particle = particle;
        return;
    }

    // If this node is a leaf but already has a particle,
    // we must subdivide and re-insert that existing particle + the new one
    if (!nw && !ne && !sw && !se)
    {
        Subdivide();

        // Re-insert the existing particle
        if (this->particle)
        {
            InsertIntoChild(this->particle);
            this->particle = nullptr;
        }
    }

    // Insert the new particle
    InsertIntoChild(particle);
}


// Insert a particle into a child node
/**
  * @brief  Insert a particle into a child node
  * @param  particle
  * @retval None
  */
void QuadtreeNode::InsertIntoChild(Particle* particle)
{
    double px = particle->GetPosition().x;
    double py = particle->GetPosition().y;

    if (nw->Contains(px, py)) { nw->Insert(particle); }
    else if (ne->Contains(px, py)) { ne->Insert(particle); }
    else if (sw->Contains(px, py)) { sw->Insert(particle); }
    else if (se->Contains(px, py)) { se->Insert(particle); }
}


/**
  * @brief  Get neighboring particles at specific location and radius
  * @param  xMin
  * @param  yMin
  * @param  xMax
  * @param  yMax
  * @param  results
  * @retval None
  */
void QuadtreeNode::QueryRange(double xMin, double yMin, double xMax, double yMax, std::vector<Particle*>& results)
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

    // If leaf node and has a particle
    if (!nw && !ne && !sw && !se)
    {
        if (particle)
        {
            const glm::dvec2& pos = particle->GetPosition();
            if (pos.x >= xMin && pos.x <= xMax &&
                pos.y >= yMin && pos.y <= yMax)
            {
                results.push_back(particle);
            }
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
  * @param  particle
  * @param  node
  * @param  theta
  * @retval bool
  */
glm::dvec2 ComputeForceBarnesHut(const Particle& particle, const QuadtreeNode* node, double theta)
{
    glm::dvec2 force(0.0);

    // If this node is empty, no force
    if (!node || node->totalMass <= 0.0)
        return force;

    // If leaf node with one particle
    if (!node->nw && !node->ne && !node->sw && !node->se)
    {
        // If it's the same particle, skip
        if (node->particle == &particle)
            return force;

        // Otherwise, compute direct force with that single particle
        glm::dvec2 dir = node->particle->GetPosition() - particle.GetPosition();
        double dist2 = glm::dot(dir, dir);
        if (dist2 < MIN_INTERACTION_DISTANCE * MIN_INTERACTION_DISTANCE)
            dist2 = MIN_INTERACTION_DISTANCE * MIN_INTERACTION_DISTANCE;

        double softDist2 = dist2 + SOFTENING * SOFTENING;
        double invDist = 1.0 / std::sqrt(softDist2);

        double f = GRAVITATIONAL_CONSTANT * particle.GetMass() * node->particle->GetMass() * invDist * invDist;
        force += f * glm::normalize(dir);
        return force;
    }

    // Size of this region
    double dx = node->centerOfMass.x - particle.GetPosition().x;
    double dy = node->centerOfMass.y - particle.GetPosition().y;
    double dist = std::sqrt(dx * dx + dy * dy);

    // If region is far away, treat as single mass
    if ((node->halfSize * 2.0) / dist < theta)
    {
        // Use node's total mass
        if (dist < MIN_INTERACTION_DISTANCE)
            dist = MIN_INTERACTION_DISTANCE;

        double dist2 = dist * dist + SOFTENING * SOFTENING;
        double invDist = 1.0 / std::sqrt(dist2);

        double f = GRAVITATIONAL_CONSTANT * particle.GetMass() * node->totalMass * invDist * invDist;
        glm::dvec2 dir(dx, dy);
        force += f * (dir / dist);
        return force;
    }
    else
    {
        // Otherwise, recurse into children
        force += ComputeForceBarnesHut(particle, node->nw.get(), theta);
        force += ComputeForceBarnesHut(particle, node->ne.get(), theta);
        force += ComputeForceBarnesHut(particle, node->sw.get(), theta);
        force += ComputeForceBarnesHut(particle, node->se.get(), theta);
    }
    return force;
}



/******************************************************************************/
/******************************************************************************/
/* Private Functions                                                          */
/******************************************************************************/
/******************************************************************************/



/******************************** END OF FILE *********************************/
