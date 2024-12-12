#pragma once

#include "PCH.hpp"

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

//class ParticleSystem
//{
//private:
//    std::vector<Particle> particles;
//    float cellSize;
//    std::unordered_map<int64_t, std::vector<size_t>> grid;
//
//    int64_t getGridKey(const glm::vec2& pos) const;
//
//    void updateGrid();
//
//public:
//    ParticleSystem(float _cellSize) : cellSize(_cellSize) {};
//
//    void addParticle(const Particle& particle);
//
//    void applyGravity(float deltaTime);
//
//    const std::vector<Particle>& getParticles() const;
//};

glm::vec3 interpolateColor(float t, const std::vector<std::pair<float, glm::vec3>>& colorStops);
glm::vec3 calculateColor(const glm::dvec2& velocity);