#pragma once

#include "../math/matrix4.h"
#include "types.h"

namespace FreeEffect {

struct Transform3D {
    Vec3 position{0.0, 0.0, 0.0};
    Vec3 rotation{0.0, 0.0, 0.0};
    Vec3 scale{1.0, 1.0, 1.0};
    Vec3 anchorPoint{0.0, 0.0, 0.0};
    bool is3D = false;

    Mat4 computeModelMatrix() const {
        Mat4 t = Mat4::translation(
            position.x - anchorPoint.x,
            position.y - anchorPoint.y,
            position.z - anchorPoint.z);

        Mat4 rx = Mat4::rotationX(rotation.x);
        Mat4 ry = Mat4::rotationY(rotation.y);
        Mat4 rz = Mat4::rotationZ(rotation.z);

        Mat4 r = Mat4::multiply(rz, Mat4::multiply(ry, rx));

        Mat4 s = Mat4::scale(scale.x, scale.y, scale.z);

        return Mat4::multiply(Mat4::multiply(t, r), s);
    }

    Vec3 projectTo2D(const Vec3& worldPoint, const Mat4& viewProjection,
                     int width, int height) const {
        Vec4 clip = Mat4::multiplyPoint(viewProjection,
            Vec4(worldPoint.x, worldPoint.y, worldPoint.z, 1.0));

        if (std::abs(clip.w) < 1e-10) {
            return {-1.0, -1.0, 0.0};
        }

        double ndcX = clip.x / clip.w;
        double ndcY = clip.y / clip.w;
        double ndcZ = clip.z / clip.w;

        double screenX = (ndcX + 1.0) * 0.5 * width;
        double screenY = (1.0 - ndcY) * 0.5 * height;

        return {screenX, screenY, ndcZ};
    }
};

} // namespace FreeEffect
