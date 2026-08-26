#include "camera_tools.h"
#include "../timeline/camera.h"
#include "../timeline/composition.h"
#include <cmath>

namespace FreeEffect {

void CameraToolBase::onMouseDown(double x, double y, int modifiers) {
    m_startPos = {x, y};
    m_dragging = true;
    m_accumDx = 0;
    m_accumDy = 0;
}

void CameraToolBase::onMouseMove(double x, double y, int modifiers) {
    if (!m_dragging) return;

    double dx = x - m_startPos.x;
    double dy = y - m_startPos.y;
    m_accumDx = dx;
    m_accumDy = dy;

    applyDelta(dx, dy);
    m_startPos = {x, y};
}

void CameraToolBase::onMouseUp(double x, double y, int modifiers) {
    m_dragging = false;
}

ToolResult CameraToolBase::getResult() const {
    ToolResult result;
    result.consumed = m_dragging;
    return result;
}

void CameraToolBase::reset() {
    m_dragging = false;
    m_accumDx = 0;
    m_accumDy = 0;
}

void OrbitCameraTool::applyDelta(double dx, double dy) {
    if (!m_camera) return;

    auto& transform = m_camera->getTransform();
    Vec3 poi = m_camera->getPointOfInterest();
    Vec3 pos = transform.position;

    double azimuth = dx * 0.005;
    double elevation = dy * 0.005;

    double relX = pos.x - poi.x;
    double relY = pos.y - poi.y;
    double relZ = pos.z - poi.z;

    double cosAz = std::cos(azimuth);
    double sinAz = std::sin(azimuth);
    double newX = relX * cosAz - relZ * sinAz;
    double newZ = relX * sinAz + relZ * cosAz;

    double cosEl = std::cos(elevation);
    double sinEl = std::sin(elevation);
    double newY = relY * cosEl - newZ * sinEl;
    double finalZ = relY * sinEl + newZ * cosEl;

    transform.position = {poi.x + newX, poi.y + newY, poi.z + finalZ};
    m_camera->lookAt(poi);
}

void TrackXYCameraTool::applyDelta(double dx, double dy) {
    if (!m_camera) return;

    auto& transform = m_camera->getTransform();
    Vec3 poi = m_camera->getPointOfInterest();

    double panSpeed = 0.005;
    double panX = dx * panSpeed * m_camera->getZoom();
    double panY = dy * panSpeed * m_camera->getZoom();

    transform.position.x -= panX;
    transform.position.y += panY;
    poi.x -= panX;
    poi.y += panY;
    m_camera->setPointOfInterest(poi);
}

void TrackZCameraTool::applyDelta(double dx, double dy) {
    if (!m_camera) return;

    auto& transform = m_camera->getTransform();
    Vec3 poi = m_camera->getPointOfInterest();

    double dollySpeed = 0.01;
    double dolly = -(dx + dy) * dollySpeed;

    Vec3 forward = poi - transform.position;
    double len = forward.length();
    if (len < 1e-6) return;

    forward = forward.normalized();
    transform.position.x += forward.x * dolly;
    transform.position.y += forward.y * dolly;
    transform.position.z += forward.z * dolly;
}

} // namespace FreeEffect
