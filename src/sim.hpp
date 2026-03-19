#ifndef SIM_HPP
#define SIM_HPP

#include "vec.hpp"
#include <vector>

typedef double Num;
typedef Vec3d<Num> Vec;

constexpr Num TAU = 2 * 3.14159265358979323846;
constexpr Num G = TAU * TAU; //time in years, distance in AU, mass in solar mass

struct Body {
    Num mu; //G * mass
    Vec position;
    Vec velocity;
    Vec acceleration; //used to store acceleration between time steps; maybe better way?
    Body(Num mass, Vec position, Vec velocity) : mu(G * mass), position(position), velocity(velocity), acceleration() {}
    Body() : mu(), position(), velocity(), acceleration() {}    
};

void update(std::vector<Body>& bodies, double dt);

#endif