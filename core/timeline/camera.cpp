#include "camera.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

Camera::Camera()
    : m_id(generateUUID()) {
    m_transform.is3D = true;
}

Camera::Camera(CameraType type)
    : m_id(generateUUID())
    , m_type(type) {
    m_transform.is3D = true;
}

Mat4 Camera::getViewMatrix() const {
    Vec3 eye = m_transform.position;
    Vec3 target = m_pointOfInterest;
    Vec3 up = Vec3(0.0, 1.0, 0.0);

    Vec3 fwd = (target - eye).normalized();
    if (std::abs(fwd.dot(up)) > 0.999) {
        up = Vec3(0.0, 0.0, 1.0);
    }

    Vec3 right = fwd.cross(up).normalized();
    up = right.cross(fwd);

    Mat4 view = Mat4::identity();
    view.m[0] = static_cast<float>(right.x);
    view.m[4] = static_cast<float>(right.y);
    view.m[8] = static_cast<float>(right.z);

    view.m[1] = static_cast<float>(up.x);
    view.m[5] = static_cast<float>(up.y);
    view.m[9] = static_cast<float>(up.z);

    view.m[2] = static_cast<float>(-fwd.x);
    view.m[6] = static_cast<float>(-fwd.y);
    view.m[10] = static_cast<float>(-fwd.z);

    view.m[12] = static_cast<float>(-right.dot(eye));
    view.m[13] = static_cast<float>(-up.dot(eye));
    view.m[14] = static_cast<float>(fwd.dot(eye));

    return view;
}

Mat4 Camera::getProjectionMatrix() const {
    double aspect = m_apertureWidth / m_apertureHeight;
    double fovY = 2.0 * std::atan(m_apertureHeight / (2.0 * m_focalLength)) * (180.0 / 3.14159265358979323846);
    fovY *= m_zoom;
    return Mat4::perspective(fovY, aspect, m_nearClip, m_farClip);
}

Mat4 Camera::getViewProjectionMatrix() const {
    return Mat4::multiply(getProjectionMatrix(), getViewMatrix());
}

Vec3 Camera::worldToScreen(const Vec3& world, int width, int height) const {
    Mat4 vp = getViewProjectionMatrix();
    Vec4 clip = Mat4::multiplyPoint(vp, Vec4(world.x, world.y, world.z, 1.0));

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

void Camera::lookAt(const Vec3& target, const Vec3& up) {
    Vec3 eye = m_transform.position;
    Vec3 fwd = (target - eye).normalized();

    double rotY = std::atan2(fwd.x, -fwd.z) * (180.0 / 3.14159265358979323846);
    double rotX = std::asin(-fwd.y) * (180.0 / 3.14159265358979323846);

    m_transform.rotation.x = rotX;
    m_transform.rotation.y = rotY;
    m_transform.rotation.z = 0.0;
    m_pointOfInterest = target;
}

} // namespace FreeEffect
