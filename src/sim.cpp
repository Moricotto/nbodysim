#include "sim.hpp"
#include <cstdio>
constexpr Num EPSILON = 0;
constexpr Num EPSILON_SQ = EPSILON * EPSILON;

void update(std::vector<Body>& bodies, double dt) {
    //kick-drift
    for (size_t i = 0; i < bodies.size(); i++) {
        bodies[i].velocity += bodies[i].acceleration * 0.5 * dt;
        bodies[i].position += bodies[i].velocity * dt;
    }
    //calculate acceleration
    for (size_t i = 0; i < bodies.size(); i++) {
        bodies[i].acceleration = Vec(0, 0, 0);
        for (size_t j = 0; j < bodies.size(); j++) {
            if (j == i) continue;
            Vec r = bodies[j].position - bodies[i].position; //vector from i to j
            Num distance_sq = r.magnitudeSquared();
            Num denom = EPSILON_SQ + distance_sq;
            bodies[i].acceleration += r * bodies[j].mu / std::sqrt(denom * denom * denom);
        }
        //kick
        bodies[i].velocity += bodies[i].acceleration * 0.5 * dt;
    }
}