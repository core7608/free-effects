#pragma once

#include "../timeline/types.h"
#include <cmath>
#include <cstring>

namespace FreeEffect {

struct Vec4 {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double w = 0.0;

    Vec4() = default;
    Vec4(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, double w) : x(v.x), y(v.y), z(v.z), w(w) {}
};

struct Mat4 {
    float m[16];

    Mat4() { std::memset(m, 0, sizeof(m)); }

    float& operator[](int idx) { return m[idx]; }
    float operator[](int idx) const { return m[idx]; }

    float& at(int row, int col) { return m[col * 4 + row]; }
    float at(int row, int col) const { return m[col * 4 + row]; }

    static Mat4 identity() {
        Mat4 r;
        r.m[0] = 1.0f; r.m[5] = 1.0f; r.m[10] = 1.0f; r.m[15] = 1.0f;
        return r;
    }

    static Mat4 translation(double x, double y, double z) {
        Mat4 r = identity();
        r.m[12] = static_cast<float>(x);
        r.m[13] = static_cast<float>(y);
        r.m[14] = static_cast<float>(z);
        return r;
    }

    static Mat4 rotationX(double degrees) {
        double rad = degrees * 3.14159265358979323846 / 180.0;
        float c = static_cast<float>(std::cos(rad));
        float s = static_cast<float>(std::sin(rad));
        Mat4 r = identity();
        r.m[5] = c;   r.m[6] = s;
        r.m[9] = -s;  r.m[10] = c;
        return r;
    }

    static Mat4 rotationY(double degrees) {
        double rad = degrees * 3.14159265358979323846 / 180.0;
        float c = static_cast<float>(std::cos(rad));
        float s = static_cast<float>(std::sin(rad));
        Mat4 r = identity();
        r.m[0] = c;   r.m[2] = -s;
        r.m[8] = s;   r.m[10] = c;
        return r;
    }

    static Mat4 rotationZ(double degrees) {
        double rad = degrees * 3.14159265358979323846 / 180.0;
        float c = static_cast<float>(std::cos(rad));
        float s = static_cast<float>(std::sin(rad));
        Mat4 r = identity();
        r.m[0] = c;   r.m[1] = s;
        r.m[4] = -s;  r.m[5] = c;
        return r;
    }

    static Mat4 scale(double x, double y, double z) {
        Mat4 r;
        r.m[0] = static_cast<float>(x);
        r.m[5] = static_cast<float>(y);
        r.m[10] = static_cast<float>(z);
        r.m[15] = 1.0f;
        return r;
    }

    static Mat4 perspective(double fovYDegrees, double aspect, double nearVal, double farVal) {
        double rad = fovYDegrees * 3.14159265358979323846 / 180.0;
        double tanHalfFov = std::tan(rad * 0.5);

        Mat4 r;
        r.m[0] = static_cast<float>(1.0 / (aspect * tanHalfFov));
        r.m[5] = static_cast<float>(1.0 / tanHalfFov);
        r.m[10] = static_cast<float>(-(farVal + nearVal) / (farVal - nearVal));
        r.m[11] = -1.0f;
        r.m[12] = static_cast<float>(-(2.0 * farVal * nearVal) / (farVal - nearVal));
        return r;
    }

    static Mat4 lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
        Vec3 f = (target - eye).normalized();
        Vec3 s = f.cross(up).normalized();
        Vec3 u = s.cross(f);

        Mat4 r = identity();
        r.m[0] = static_cast<float>(s.x);
        r.m[4] = static_cast<float>(s.y);
        r.m[8] = static_cast<float>(s.z);

        r.m[1] = static_cast<float>(u.x);
        r.m[5] = static_cast<float>(u.y);
        r.m[9] = static_cast<float>(u.z);

        r.m[2] = static_cast<float>(-f.x);
        r.m[6] = static_cast<float>(-f.y);
        r.m[10] = static_cast<float>(-f.z);

        r.m[12] = static_cast<float>(-s.dot(eye));
        r.m[13] = static_cast<float>(-u.dot(eye));
        r.m[14] = static_cast<float>(f.dot(eye));

        return r;
    }

    static Mat4 multiply(const Mat4& a, const Mat4& b) {
        Mat4 r;
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += a.m[k * 4 + row] * b.m[col * 4 + k];
                }
                r.m[col * 4 + row] = sum;
            }
        }
        return r;
    }

    static Vec4 multiplyPoint(const Mat4& m, const Vec4& p) {
        Vec4 r;
        r.x = m.m[0] * p.x + m.m[4] * p.y + m.m[8] * p.z + m.m[12] * p.w;
        r.y = m.m[1] * p.x + m.m[5] * p.y + m.m[9] * p.z + m.m[13] * p.w;
        r.z = m.m[2] * p.x + m.m[6] * p.y + m.m[10] * p.z + m.m[14] * p.w;
        r.w = m.m[3] * p.x + m.m[7] * p.y + m.m[11] * p.z + m.m[15] * p.w;
        return r;
    }

    static Mat4 inverse(Mat4 mat) {
        float inv[16], det;
        float* o = mat.m;

        inv[0] = o[5]  * o[10] * o[15] -
                 o[5]  * o[11] * o[14] -
                 o[9]  * o[6]  * o[15] +
                 o[9]  * o[7]  * o[14] +
                 o[13] * o[6]  * o[11] -
                 o[13] * o[7]  * o[10];

        inv[4] = -o[4]  * o[10] * o[15] +
                  o[4]  * o[11] * o[14] +
                  o[8]  * o[6]  * o[15] -
                  o[8]  * o[7]  * o[14] -
                  o[12] * o[6]  * o[11] +
                  o[12] * o[7]  * o[10];

        inv[8] = o[4]  * o[9] * o[15] -
                 o[4]  * o[11] * o[13] -
                 o[8]  * o[5] * o[15] +
                 o[8]  * o[7] * o[13] +
                 o[12] * o[5] * o[11] -
                 o[12] * o[7] * o[9];

        inv[12] = -o[4]  * o[9] * o[14] +
                   o[4]  * o[10] * o[13] +
                   o[8]  * o[5] * o[14] -
                   o[8]  * o[6] * o[13] -
                   o[12] * o[5] * o[10] +
                   o[12] * o[6] * o[9];

        inv[1] = -o[1]  * o[10] * o[15] +
                  o[1]  * o[11] * o[14] +
                  o[9]  * o[2] * o[15] -
                  o[9]  * o[3] * o[14] -
                  o[13] * o[2] * o[11] +
                  o[13] * o[3] * o[10];

        inv[5] = o[0]  * o[10] * o[15] -
                 o[0]  * o[11] * o[14] -
                 o[8]  * o[2] * o[15] +
                 o[8]  * o[3] * o[14] +
                 o[12] * o[2] * o[11] -
                 o[12] * o[3] * o[10];

        inv[9] = -o[0]  * o[9] * o[15] +
                  o[0]  * o[11] * o[13] +
                  o[8]  * o[1] * o[15] -
                  o[8]  * o[3] * o[13] -
                  o[12] * o[1] * o[11] +
                  o[12] * o[3] * o[9];

        inv[13] = o[0]  * o[9] * o[14] -
                  o[0]  * o[10] * o[13] -
                  o[8]  * o[1] * o[14] +
                  o[8]  * o[2] * o[13] +
                  o[12] * o[1] * o[10] -
                  o[12] * o[2] * o[9];

        inv[2] = o[1]  * o[6] * o[15] -
                 o[1]  * o[7] * o[14] -
                 o[5]  * o[2] * o[15] +
                 o[5]  * o[3] * o[14] +
                 o[13] * o[2] * o[7] -
                 o[13] * o[3] * o[6];

        inv[6] = -o[0]  * o[6] * o[15] +
                  o[0]  * o[7] * o[14] +
                  o[4]  * o[2] * o[15] -
                  o[4]  * o[3] * o[14] -
                  o[12] * o[2] * o[7] +
                  o[12] * o[3] * o[6];

        inv[10] = o[0]  * o[5] * o[15] -
                  o[0]  * o[7] * o[13] -
                  o[4]  * o[1] * o[15] +
                  o[4]  * o[3] * o[13] +
                  o[12] * o[1] * o[7] -
                  o[12] * o[3] * o[5];

        inv[14] = -o[0]  * o[5] * o[14] +
                   o[0]  * o[6] * o[13] +
                   o[4]  * o[1] * o[14] -
                   o[4]  * o[2] * o[13] -
                   o[12] * o[1] * o[6] +
                   o[12] * o[2] * o[5];

        inv[3] = -o[1] * o[6] * o[11] +
                  o[1] * o[7] * o[10] +
                  o[5] * o[2] * o[11] -
                  o[5] * o[3] * o[10] -
                  o[9] * o[2] * o[7] +
                  o[9] * o[3] * o[6];

        inv[7] = o[0] * o[6] * o[11] -
                 o[0] * o[7] * o[10] -
                 o[4] * o[2] * o[11] +
                 o[4] * o[3] * o[10] +
                 o[8] * o[2] * o[7] -
                 o[8] * o[3] * o[6];

        inv[11] = -o[0] * o[5] * o[11] +
                   o[0] * o[7] * o[9] +
                   o[4] * o[1] * o[11] -
                   o[4] * o[3] * o[9] -
                   o[8] * o[1] * o[7] +
                   o[8] * o[3] * o[5];

        inv[15] = o[0] * o[5] * o[10] -
                  o[0] * o[6] * o[9] -
                  o[4] * o[1] * o[10] +
                  o[4] * o[2] * o[9] +
                  o[8] * o[1] * o[6] -
                  o[8] * o[2] * o[5];

        det = o[0] * inv[0] + o[1] * inv[4] + o[2] * inv[8] + o[3] * inv[12];

        Mat4 result;
        if (std::abs(det) < 1e-10f) {
            return identity();
        }

        det = 1.0f / det;
        for (int i = 0; i < 16; ++i) {
            result.m[i] = inv[i] * det;
        }
        return result;
    }

    static Mat4 transpose(Mat4 mat) {
        Mat4 r;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                r.m[i * 4 + j] = mat.m[j * 4 + i];
            }
        }
        return r;
    }
};

inline Mat4 operator*(const Mat4& a, const Mat4& b) {
    return Mat4::multiply(a, b);
}

} // namespace FreeEffect
