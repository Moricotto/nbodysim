#ifndef VEC_HPP
#define VEC_HPP

#include <math.h>

template <typename T>
struct Vec3d {
    T x, y, z;
    Vec3d(T x, T y, T z) : x(x), y(y), z(z) {}
    Vec3d() : x(), y(), z() {}
    template <typename U>
    operator Vec3d<U>() {
        return {static_cast<U>(x), static_cast<U>(y), static_cast<U>(z)};
    }
    T operator[](size_t i) const {
        //i must not be out of range
        return (i == 0) ? x : (i == 1) ? y : z;
    }
    Vec3d operator+(const Vec3d& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }
    Vec3d operator+=(const Vec3d& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    Vec3d operator-(const Vec3d& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }
    Vec3d operator*(T scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }
    Vec3d operator/(T scalar) const {
        return {x / scalar, y / scalar, z / scalar};
    }
    T dot(const Vec3d& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    Vec3d cross(const Vec3d& other) const {
        return {y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x};
    }
    T magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }
    T magnitudeSquared() const {
        return x * x + y * y + z * z;
    }
    Vec3d normalized() const {
        return *this / magnitude();
    }
};

#endif