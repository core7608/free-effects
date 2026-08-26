#pragma once

#include "../math/matrix4.h"
#include "transform_3d.h"
#include "types.h"

namespace FreeEffect {

enum class CameraType {
    Active,
    Free
};

class Camera {
public:
    Camera();
    Camera(CameraType type);

    const UUID& getId() const { return m_id; }

    CameraType getCameraType() const { return m_type; }
    void setCameraType(CameraType type) { m_type = type; }

    Transform3D& getTransform() { return m_transform; }
    const Transform3D& getTransform() const { return m_transform; }

    void setFocalLength(double mm) { m_focalLength = mm; }
    double getFocalLength() const { return m_focalLength; }

    void setApertureWidth(double mm) { m_apertureWidth = mm; }
    double getApertureWidth() const { return m_apertureWidth; }

    void setApertureHeight(double mm) { m_apertureHeight = mm; }
    double getApertureHeight() const { return m_apertureHeight; }

    void setNearClip(double near) { m_nearClip = near; }
    double getNearClip() const { return m_nearClip; }

    void setFarClip(double far) { m_farClip = far; }
    double getFarClip() const { return m_farClip; }

    void setDepthOfField(bool enabled) { m_depthOfField = enabled; }
    bool isDepthOfField() const { return m_depthOfField; }

    void setFocusDistance(double dist) { m_focusDistance = dist; }
    double getFocusDistance() const { return m_focusDistance; }

    void setAperture(double fstop) { m_aperture = fstop; }
    double getAperture() const { return m_aperture; }

    void setBlurLevel(double level) { m_blurLevel = level; }
    double getBlurLevel() const { return m_blurLevel; }

    void setZoom(double zoom) { m_zoom = zoom; }
    double getZoom() const { return m_zoom; }

    void setPointOfInterest(const Vec3& poi) { m_pointOfInterest = poi; }
    Vec3 getPointOfInterest() const { return m_pointOfInterest; }

    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix() const;
    Mat4 getViewProjectionMatrix() const;
    Vec3 worldToScreen(const Vec3& world, int width, int height) const;

    void lookAt(const Vec3& target, const Vec3& up = {0.0, 1.0, 0.0});

private:
    UUID m_id;
    CameraType m_type = CameraType::Active;
    Transform3D m_transform;
    double m_focalLength = 50.0;
    double m_apertureWidth = 36.0;
    double m_apertureHeight = 24.0;
    double m_nearClip = 0.1;
    double m_farClip = 10000.0;
    bool m_depthOfField = false;
    double m_focusDistance = 100.0;
    double m_aperture = 2.8;
    double m_blurLevel = 100.0;
    double m_zoom = 1.0;
    Vec3 m_pointOfInterest{0.0, 0.0, 0.0};
};

} // namespace FreeEffect
