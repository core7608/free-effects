#pragma once

#include "types.h"
#include <algorithm>
#include <cmath>

namespace FreeEffect {

struct MaterialOptions {
    bool castsShadows = true;
    double shadowDiffusion = 0.0;

    bool acceptsShadows = true;
    bool acceptsLights = true;

    double ambient = 10.0;
    double diffuse = 50.0;
    double specularIntensity = 50.0;
    double specularShininess = 50.0;
    double metal = 0.0;
    double reflectionIntensity = 0.0;

    Color shaderSurfaceColor{1, 1, 1, 1};

    Color evaluate(const Vec3& normal, const Vec3& lightDir, const Vec3& viewDir,
                   const Color& lightColor, double lightIntensity) const {
        Vec3 N = normal.normalized();
        Vec3 L = lightDir.normalized();
        Vec3 V = viewDir.normalized();

        double NdotL = std::max(0.0, N.dot(L));

        Vec3 R = N * (2.0 * NdotL) - L;
        double RdotV = std::max(0.0, R.dot(V));

        double shininess = specularShininess * 0.4 + 1.0;
        double spec = std::pow(RdotV, shininess);

        double ambFactor = ambient / 100.0;
        double diffFactor = diffuse / 100.0;
        double specFactor = (specularIntensity / 100.0) * spec;
        double metalFactor = metal / 100.0;
        double reflFactor = reflectionIntensity / 100.0;

        double intens = lightIntensity / 100.0;

        double diffuseContrib = diffFactor * NdotL * intens;

        double specR = specFactor * (lightColor.r * (1.0 - metalFactor) + shaderSurfaceColor.r * metalFactor);
        double specG = specFactor * (lightColor.g * (1.0 - metalFactor) + shaderSurfaceColor.g * metalFactor);
        double specB = specFactor * (lightColor.b * (1.0 - metalFactor) + shaderSurfaceColor.b * metalFactor);

        Color result;
        result.r = std::clamp(
            shaderSurfaceColor.r * ambFactor * lightColor.r +
            shaderSurfaceColor.r * diffuseContrib * lightColor.r * (1.0 - metalFactor) +
            shaderSurfaceColor.r * metalFactor * diffuseContrib * lightColor.r +
            specR * reflFactor,
            0.0, 1.0);
        result.g = std::clamp(
            shaderSurfaceColor.g * ambFactor * lightColor.g +
            shaderSurfaceColor.g * diffuseContrib * lightColor.g * (1.0 - metalFactor) +
            shaderSurfaceColor.g * metalFactor * diffuseContrib * lightColor.g +
            specG * reflFactor,
            0.0, 1.0);
        result.b = std::clamp(
            shaderSurfaceColor.b * ambFactor * lightColor.b +
            shaderSurfaceColor.b * diffuseContrib * lightColor.b * (1.0 - metalFactor) +
            shaderSurfaceColor.b * metalFactor * diffuseContrib * lightColor.b +
            specB * reflFactor,
            0.0, 1.0);
        result.a = shaderSurfaceColor.a;

        return result;
    }
};

struct CameraOptions {
    double zoom = 100.0;
    bool depthOfField = false;
    double focusDistance = 500.0;
    double aperture = 5.0;
    double blurLevel = 100.0;
    int blurType = 0;
};

} // namespace FreeEffect
