#pragma once

#include "transform_3d.h"
#include "types.h"
#include <string>

namespace FreeEffect {

enum class LightType {
    Ambient,
    Directional,
    Point,
    Spot
};

class Light {
public:
    Light();
    Light(LightType type, const std::string& name = "Light");

    const UUID& getId() const { return m_id; }

    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    LightType getLightType() const { return m_type; }
    void setLightType(LightType type) { m_type = type; }

    Transform3D& getTransform() { return m_transform; }
    const Transform3D& getTransform() const { return m_transform; }

    void setColor(const Color& color) { m_color = color; }
    Color getColor() const { return m_color; }

    void setIntensity(double intensity) { m_intensity = intensity; }
    double getIntensity() const { return m_intensity; }

    void setCastShadows(bool cast) { m_castShadows = cast; }
    bool getCastShadows() const { return m_castShadows; }

    void setShadowDarkness(double darkness) { m_shadowDarkness = darkness; }
    double getShadowDarkness() const { return m_shadowDarkness; }

    void setShadowDiffusion(double pixels) { m_shadowDiffusion = pixels; }
    double getShadowDiffusion() const { return m_shadowDiffusion; }

    void setConeAngle(double degrees) { m_coneAngle = degrees; }
    double getConeAngle() const { return m_coneAngle; }

    void setConeFeather(double feather) { m_coneFeather = feather; }
    double getConeFeather() const { return m_coneFeather; }

    Color getLightContribution(const Vec3& surfacePoint, const Vec3& surfaceNormal) const;

private:
    UUID m_id;
    std::string m_name;
    LightType m_type = LightType::Ambient;
    Transform3D m_transform;
    Color m_color{1.0, 1.0, 1.0, 1.0};
    double m_intensity = 100.0;
    bool m_castShadows = false;
    double m_shadowDarkness = 75.0;
    double m_shadowDiffusion = 5.0;
    double m_coneAngle = 90.0;
    double m_coneFeather = 50.0;
};

} // namespace FreeEffect
