// main.cpp
#include <cmath>
#include <iostream>

struct Vec2 {
    float x, y;
};

static float length(Vec2 v) {
    return std::sqrt(v.x*v.x + v.y*v.y);
}

static Vec2 normalize(Vec2 v) {
    float len = length(v);

    if (len == 0.0f) return {  0.0f, 0.0f  };
    else             return {  v.x / len, v.y / len  };
}

int main() {
    Vec2 v {  3.0f, 4.0f  };
    float len = length(v);
    Vec2 u = normalize(v);

    std::cout << "v=(" << v.x << ", " << v.y << ")" << std::endl;
    std::cout << "|v| = " << len << std::endl;
    std::cout << "u = normalize(v) = (" << u.x << ", " << u.y << ")" << std::endl;
    std::cout << "|u| = " << length(u) << std::endl;

    return 0;
}