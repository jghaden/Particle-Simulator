#include "PCH.hpp"

#include "Engine.hpp"
#include "Simulation.hpp"
#include "Particle.hpp"

Particle::Particle() {}

Particle::Particle(glm::dvec2 position, glm::dvec2 velocity, double mass) : position(position), velocity(velocity), acceleration(glm::dvec2(0.0)), mass(mass) {}

// Function to interpolate between colors based on a normalized value
glm::vec3 Particle::interpolateColor(float t, const std::vector<std::pair<float, glm::vec3>>& colorStops)
{
    if (t <= colorStops.front().first) return colorStops.front().second;
    if (t >= colorStops.back().first) return colorStops.back().second;

    for (size_t i = 1; i < colorStops.size(); ++i)
    {
        if (t < colorStops[i].first)
        {
            float localT = (t - colorStops[i - 1].first) / (colorStops[i].first - colorStops[i - 1].first);
            return glm::mix(colorStops[i - 1].second, colorStops[i].second, localT);
        }
    }

    return colorStops.back().second; // Should never reach here
}

void Particle::updateColor()
{
    double speed = glm::length(this->velocity);
    float t = static_cast<float>(glm::clamp(speed / MAX_PARTICLE_COLOR_SPEED, 0.0, 1.0));

    // Define the color stops
    std::vector<std::pair<float, glm::vec3>> colorStops = {
        {0.0f, glm::vec3(1.0f, 0.0f, 0.0f)},  // Red
        {0.25f, glm::vec3(1.0f, 1.0f, 0.0f)}, // Yellow
        {0.5f, glm::vec3(0.0f, 1.0f, 0.0f)},  // Green
        {0.75f, glm::vec3(0.0f, 0.0f, 1.0f)}, // Blue
        {1.0f, glm::vec3(1.0f, 0.0f, 0.8f)}   // Purple
    };

    color = interpolateColor(t, colorStops);
}

void Particle::update()
{
    position += velocity * TIME_STEP;
    velocity *= DAMPING;

    updateColor();
}

// Function to interpolate between colors based on a normalized value
glm::vec3 interpolateColor(float t, const std::vector<std::pair<float, glm::vec3>>& colorStops)
{
    if (t <= colorStops.front().first) return colorStops.front().second;
    if (t >= colorStops.back().first) return colorStops.back().second;

    for (size_t i = 1; i < colorStops.size(); ++i)
    {
        if (t < colorStops[i].first)
        {
            float localT = (t - colorStops[i - 1].first) / (colorStops[i].first - colorStops[i - 1].first);
            return glm::mix(colorStops[i - 1].second, colorStops[i].second, localT);
        }
    }

    return colorStops.back().second; // Should never reach here
}

glm::vec3 calculateColor(const glm::dvec2& velocity)
{
    double speed = glm::length(velocity);
    float t = static_cast<float>(glm::clamp(speed / MAX_PARTICLE_COLOR_SPEED, 0.0, 1.0));

    // Define the color stops
    std::vector<std::pair<float, glm::vec3>> colorStops = {
        {0.0f, glm::vec3(1.0f, 0.0f, 0.0f)},  // Red
        {0.25f, glm::vec3(1.0f, 1.0f, 0.0f)}, // Yellow
        {0.5f, glm::vec3(0.0f, 1.0f, 0.0f)},  // Green
        {0.75f, glm::vec3(0.0f, 0.0f, 1.0f)}, // Blue
        {1.0f, glm::vec3(1.0f, 0.0f, 0.8f)}   // Purple
    };

    return interpolateColor(t, colorStops);
}
