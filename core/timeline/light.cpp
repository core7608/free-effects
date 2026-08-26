#include "light.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

Light::Light()
    : m_id(generateUUID()) {
    m_transform.is3D = true;
}

Light::Light(LightType type, const std::string& name)
    : m_id(generateUUID())
    , m_name(name)
    , m_type(type) {
    m_transform.is3D = true;
}

Color Light::getLightContribution(const Vec3& surfacePoint, const Vec3& surfaceNormal) const {
    double factor = 0.0;
    Vec3 toLight;

    switch (m_type) {
        case LightType::Ambient: {
            factor = 1.0;
            break;
        }
        case LightType::Directional: {
            Vec3 lightDir = Vec3(
                -std::sin(m_transform.rotation.y * 3.14159265358979323846 / 180.0) *
                std::cos(m_transform.rotation.x * 3.14159265358979323846 / 180.0),
                std::sin(m_transform.rotation.x * 3.14159265358979323846 / 180.0),
                -std::cos(m_transform.rotation.y * 3.14159265358979323846 / 180.0) *
                std::cos(m_transform.rotation.x * 3.14159265358979323846 / 180.0)
            ).normalized();
            factor = std::max(0.0, surfaceNormal.normalized().dot(lightDir));
            break;
        }
        case LightType::Point: {
            toLight = m_transform.position - surfacePoint;
            double dist = toLight.length();
            if (dist < 1e-6) {
                factor = 1.0;
            } else {
                Vec3 lightDir = toLight.normalized();
                factor = std::max(0.0, surfaceNormal.normalized().dot(lightDir));
                factor *= 1.0 / (1.0 + 0.001 * dist + 0.0001 * dist * dist);
            }
            break;
        }
        case LightType::Spot: {
            toLight = m_transform.position - surfacePoint;
            double dist = toLight.length();
            if (dist < 1e-6) {
                factor = 1.0;
            } else {
                Vec3 lightDir = toLight.normalized();
                double diffuse = std::max(0.0, surfaceNormal.normalized().dot(lightDir));

                Vec3 spotDir = Vec3(
                    -std::sin(m_transform.rotation.y * 3.14159265358979323846 / 180.0) *
                    std::cos(m_transform.rotation.x * 3.14159265358979323846 / 180.0),
                    std::sin(m_transform.rotation.x * 3.14159265358979323846 / 180.0),
                    -std::cos(m_transform.rotation.y * 3.14159265358979323846 / 180.0) *
                    std::cos(m_transform.rotation.x * 3.14159265358979323846 / 180.0)
                ).normalized();

                double cosAngle = (-lightDir).dot(spotDir);
                double halfCone = m_coneAngle * 0.5;
                double cosHalfCone = std::cos(halfCone * 3.14159265358979323846 / 180.0);
                double featherOuter = m_coneFeather * 0.5;
                double cosFeather = std::cos((halfCone - featherOuter) * 3.14159265358979323846 / 180.0);

                double spotFactor = 0.0;
                if (cosAngle >= cosHalfCone) {
                    spotFactor = 1.0;
                } else if (cosAngle >= cosFeather && cosHalfCone != cosFeather) {
                    spotFactor = (cosAngle - cosHalfCone) / (cosFeather - cosHalfCone);
                    spotFactor = std::clamp(spotFactor, 0.0, 1.0);
                }

                factor = diffuse * spotFactor;
                factor *= 1.0 / (1.0 + 0.001 * dist + 0.0001 * dist * dist);
            }
            break;
        }
    }

    double intensityScale = m_intensity / 100.0;
    factor *= intensityScale;

    Color result;
    result.r = m_color.r * factor;
    result.g = m_color.g * factor;
    result.b = m_color.b * factor;
    result.a = m_color.a * std::clamp(factor, 0.0, 1.0);

    return result;
}

} // namespace FreeEffect
