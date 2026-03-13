#include "sim.hpp"
#include <glm/glm.hpp>
#include <cstdio>
constexpr Num EPSILON = 0;
constexpr Num EPSILON_SQ = EPSILON * EPSILON;
constexpr Num G = 1; //time in years, distance in AU, giving mass in solar mass / (4*pi*pi)

void update(std::vector<Body>& bodies, double dt) {
    for (size_t i = 0; i < bodies.size(); i++) {
        Vec acceleration(0, 0, 0);
        for (size_t j = 0; j < bodies.size(); j++) {
            if (j == i) continue;
            Vec r = bodies[j].position - bodies[i].position; //vector from i to j
						// this represent the squared distance
            Num distance_sq = glm::dot(r, r);
            Num denom = EPSILON_SQ + distance_sq;
            acceleration += r * bodies[j].mass / std::sqrt(denom * denom * denom);
        }
        bodies[i].acceleration = acceleration;
        bodies[i].velocity += acceleration * dt;
        bodies[i].position += bodies[i].velocity * dt;
    }
}
