#pragma once

#include "../timeline/types.h"
#include <algorithm>
#include <cmath>
#include <string>

namespace FreeEffect {

enum class BlendMode { Opaque, AlphaBlend, Additive, Multiply };

struct Material {
    std::string name = "Default";
    Color diffuseColor{1, 1, 1, 1};
    Color specularColor{1, 1, 1, 1};
    Color ambientColor{0.1, 0.1, 0.1, 1.0};
    Color emissionColor{0, 0, 0, 0};

    double specularIntensity = 0.5;
    double specularExponent = 32.0;
    double roughness = 0.5;
    double metallic = 0.0;
    double opacity = 1.0;

    bool doubleSided = false;
    BlendMode blendMode = BlendMode::AlphaBlend;

    Color evaluate(const Vec3& normal, const Vec3& lightDir, const Vec3& viewDir) const {
        Vec3 N = normal.normalized();
        Vec3 L = lightDir.normalized();
        Vec3 V = viewDir.normalized();
        Vec3 H = (L + V).normalized();

        double NdotL = N.dot(L);
        if (!doubleSided) {
            NdotL = std::max(NdotL, 0.0);
        } else {
            NdotL = std::abs(NdotL);
        }

        double NdotH = std::max(N.dot(H), 0.0);

        double specularFactor = std::pow(NdotH, specularExponent) * specularIntensity;

        double roughFactor = 1.0 - roughness;
        double metalFactor = metallic;

        Color ambient;
        ambient.r = ambientColor.r * diffuseColor.r;
        ambient.g = ambientColor.g * diffuseColor.g;
        ambient.b = ambientColor.b * diffuseColor.b;
        ambient.a = opacity;

        Color diffuse;
        diffuse.r = diffuseColor.r * NdotL * roughFactor;
        diffuse.g = diffuseColor.g * NdotL * roughFactor;
        diffuse.b = diffuseColor.b * NdotL * roughFactor;
        diffuse.a = 0;

        Color spec;
        spec.r = specularColor.r * specularFactor * (1.0 - metalFactor);
        spec.g = specularColor.g * specularFactor * (1.0 - metalFactor);
        spec.b = specularColor.b * specularFactor * (1.0 - metalFactor);
        spec.a = 0;

        Color metalSpec;
        metalSpec.r = diffuseColor.r * specularFactor * metalFactor;
        metalSpec.g = diffuseColor.g * specularFactor * metalFactor;
        metalSpec.b = diffuseColor.b * specularFactor * metalFactor;
        metalSpec.a = 0;

        Color result;
        result.r = std::clamp(ambient.r + diffuse.r + spec.r + metalSpec.r + emissionColor.r, 0.0, 1.0);
        result.g = std::clamp(ambient.g + diffuse.g + spec.g + metalSpec.g + emissionColor.g, 0.0, 1.0);
        result.b = std::clamp(ambient.b + diffuse.b + spec.b + metalSpec.b + emissionColor.b, 0.0, 1.0);
        result.a = opacity;
        return result;
    }
};

} // namespace FreeEffect
