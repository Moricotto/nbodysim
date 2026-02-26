#ifndef SIM_HPP
#define SIM_HPP

#include <vector>
#include <glm/vec3.hpp>

typedef double Num;
typedef glm::dvec3 Vec;

struct Body {
    double mass;
    Vec position;
    Vec velocity;
    Vec acceleration; //used to store acceleration between time steps; maybe better way?
    Body(double mass, Vec position, Vec velocity) : mass(mass), position(position), velocity(velocity), acceleration() {}
    Body() : mass(0), position(), velocity(), acceleration() {}    
};

void update(std::vector<Body>& bodies, double dt);

#endif
