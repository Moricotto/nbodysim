#include "sim.hpp"
#include <glm/glm.hpp>
#include <cstdio>
#include "container/octree.hpp"
constexpr Num EPSILON = 0.01;
constexpr Num EPSILON_SQ = EPSILON * EPSILON;

constexpr bool do_naive = false;
void barnes_hut(std::vector<Body>& bodies, double dt){
  Octree<Body> tree(bodies);
  tree.update(G, dt);
}

void naive_method(std::vector<Body>& bodies, double dt) {
    for (size_t i = 0; i < bodies.size(); i++) {
        bodies[i].acceleration = Vec(0, 0, 0);
        for (size_t j = 0; j < bodies.size(); j++) {
            if (j == i) continue;
            Vec r = bodies[j].position - bodies[i].position; //vector from i to j
						// this represent the squared distance
            Num distance_sq = glm::dot(r, r);
            Num denom = EPSILON_SQ + distance_sq;
            bodies[i].acceleration += r * bodies[j].mass * G / std::sqrt(denom * denom * denom);
        }
    }
}

void update(std::vector<Body>& bodies, double dt) {
    //kick-drift
    for (size_t i = 0; i < bodies.size(); i++) {
        bodies[i].velocity += bodies[i].acceleration * 0.5 * dt;
        bodies[i].position += bodies[i].velocity * dt;
    }
    if (do_naive)
      naive_method(bodies, dt);
    else
      barnes_hut(bodies, dt);

    // kick
    for (size_t i = 0; i < bodies.size(); i++){
          bodies[i].velocity += bodies[i].acceleration * 0.5 * dt;
    }

}
