// main.cpp
#include <cmath>
#include <cstdio>

struct Vec2 {
    double x, y;
};

static double dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

static double length(Vec2 v) {
    return std::sqrt(dot(v, v));
}

static Vec2 rotate(Vec2 v, double theta_rad) {
    // [x'] = [ cos -sin ][x]
    // [y']   [ sin  cos ][y]
    const double c = std::cos(theta_rad);
    const double s = std::sin(theta_rad);

    return {
        v.x * c - v.y * s,
        v.x * s + v.y * c
    };
}

static Vec2 dir_from_angle(double theta_rad) {
    // unit direction for given angle
    return {
        std::cos(theta_rad),
        std::sin(theta_rad)
    };
}

static double angle_from_dir(Vec2 v) {
    if (v.x == 0.0 && v.y == 0) return 0.0;
    else return std::atan2(v.y, v.x);
}

static double deg_to_rad(double deg) {
    return deg * (3.14159265358979323846 / 180.0);
}

static double rad_to_deg(double rad) {
    return rad * (180.0 / 3.14159265358979323846);
}

static double normalize_angle_pi(double rad) {
    constexpr double pi     = 3.14159265358979323846;
    constexpr double two_pi = 2.0 * pi;

    rad = std::fmod(rad, two_pi);

    if (rad <= -pi) rad += two_pi;
    if (rad >  pi)  rad -= two_pi;
    return rad;
}


int main() {
    std::printf("=== CH2-5: 2D rotation + atan2 (radians) ===\n\n");

    Vec2 v {3.0f, 4.0f };               // length = 5
    double theta = deg_to_rad(30.0);    // rotate by 30

    Vec2 vr = rotate(v, theta);

    std::printf("Rotate v    = (%.6f, %.6f) by 30 deg\n", v.x, v.y);
    std::printf("      |v|   = %.6f\n", length(v));
    std::printf("       vr   = (%.6f, %.6f)\n", vr.x, vr.y);
    std::printf("      |vr|  = %.6f  (should be close to |v|)\n\n", length(vr));

    double a0 = deg_to_rad(135.0);
    Vec2 d0 = dir_from_angle(a0);
    double a1 = angle_from_dir(d0);

    std::printf("Angle -> dir -> angle (round-trip)\n");
    std::printf("a0(deg) = %.3f\n", rad_to_deg(a0));
    std::printf("d0      = (%.6f, %.6f)  (unit direction)\n", d0.x, d0.y);
    std::printf("a1(deg) = %.3f  (atan2 result)\n\n", rad_to_deg(a1));

    double big = deg_to_rad(135.0 + 720.0 + 30.0); // large angle
    double big_norm = normalize_angle_pi(big);

    std::printf("Angle normalization\n");
    std::printf("big(deg)  = %.3f\n", rad_to_deg(big));
    std::printf("norm(deg) = %.3f  (in (-180, 180])\n\n", rad_to_deg(big_norm));

    Vec2 d = dir_from_angle(deg_to_rad(10.0));
    Vec2 dr = rotate(d, deg_to_rad(80.0)); // expect ~90 deg direction
    double ar = angle_from_dir(dr);

    std::printf("Rotate direction and read angle\n");
    std::printf("start dir angle(deg)=10.000\n");
    std::printf("rotated dir      = (%.6f, %.6f)\n", dr.x, dr.y);
    std::printf("atan2 angle(deg) = %.3f (expected around 90 deg)\n\n", rad_to_deg(ar));

    return 0;
}