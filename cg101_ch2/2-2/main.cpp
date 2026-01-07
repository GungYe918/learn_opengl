#include <cmath>
#include <iostream>

struct Vec2 {
    float x, y;
};

static float dot(Vec2 a, Vec2 b) {
    return a.x*b.x + a.y*b.y;
}

static float length(Vec2 v) {
    return std::sqrt(dot(v, v));
}

static Vec2 normalize(Vec2 v) {
    float len = length(v);
    if (len == 0.0f) return { 0.0f, 0.0f };
    return { v.x/len, v.y/len };
}

static float clamp(float x, float lo, float hi) {
    return (x < lo) ? lo : (x > hi) ? hi : x;
}


int main() {
    Vec2 a { 1.0f, 0.0f };
    Vec2 b { 1.0f, 1.0f };

    Vec2 ah = normalize(a);
    Vec2 bh = normalize(b);

    float c = dot(ah, bh);      // = cos(theata)
    c = clamp(c, -1.0f, 1.0f);
    float theta = std::acos(c); // radians

    std::printf("cos(theta)=%.6f\n", c);
    std::printf("theta(rad)=%.6f, theta(deg)=%.3f\n", theta, theta * 180 / 3.141592653589793);

    return 0;
}