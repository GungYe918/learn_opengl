// main.cpp
#include <cmath>
#include <cstdio>

struct Vec3 {
    float x, y, z;
};

static Vec3 sub(Vec3 a, Vec3 b) {
    return {a.x-b.x, a.y-b.y, a.z-b.z}; 
}

static float dot(Vec3 a, Vec3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

static Vec3 cross(Vec3 a, Vec3 b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

static float length(Vec3 v) {
    return std::sqrt(dot(v, v));
}

static Vec3 normalize(Vec3 v) {
    float len = length(v);
    if (len == 0) return { 0.0f, 0.0f, 0.0f };
    else          return { v.x/len, v.y/len, v.z/len }; 
}

int main() {
    Vec3 P0 {0.0f, 0.0f, 0.0f };
    Vec3 P1 {1.0f, 0.0f, 0.0f };
    Vec3 P2 {0.0f, 1.0f, 0.0f };

    Vec3 e1 = sub(P1, P0);
    Vec3 e2 = sub(P2, P0);

    Vec3 n  = cross(e1, e2);
    Vec3 nh = normalize(n);

    std::printf("n  = (%.3f, %.3f, %.3f), |n|  = %.3f\n", n.x, n.y, n.z,    length(n));
    std::printf("nh = (%.3f, %.3f, %.3f), |nh| = %.3f\n", nh.x, nh.y, nh.z, length(nh));

    std::printf("dot(n, e1) = %.3f\n", dot(n, e1));
    std::printf("dot(n, e2) = %.3f\n", dot(n, e2));

    return 0;
}