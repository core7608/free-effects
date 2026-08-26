#include "test_framework.h"
#include "../core/timeline/layer.h"
#include "../core/timeline/composition.h"
#include "../core/rendering/renderer.h"
#include "../core/effects/effect.h"
#include "../core/effects/effect_registry.h"
#include "../core/effects/blur/gaussian_blur_effect.h"
#include <cmath>

using namespace FreeEffect;

TEST(layer_add_effect) {
    Layer layer("Test Layer", LayerType::Solid);
    ASSERT_FALSE(layer.hasEffects());
    ASSERT_EQ(layer.getEffectCount(), 0);

    auto effect = std::make_shared<GaussianBlurEffect>();
    UUID effectId = effect->getId();
    layer.addEffect(effect);

    ASSERT_TRUE(layer.hasEffects());
    ASSERT_EQ(layer.getEffectCount(), 1);
    ASSERT_TRUE(layer.getEffectById(effectId) != nullptr);
}

TEST(layer_remove_effect) {
    Layer layer("Test Layer", LayerType::Solid);
    auto effect = std::make_shared<GaussianBlurEffect>();
    UUID effectId = effect->getId();
    layer.addEffect(effect);
    ASSERT_EQ(layer.getEffectCount(), 1);

    layer.removeEffect(effectId);
    ASSERT_EQ(layer.getEffectCount(), 0);
    ASSERT_FALSE(layer.hasEffects());
}

TEST(layer_remove_effect_by_index) {
    Layer layer("Test Layer", LayerType::Solid);
    layer.addEffect(std::make_shared<GaussianBlurEffect>());
    layer.addEffect(std::make_shared<GaussianBlurEffect>());
    ASSERT_EQ(layer.getEffectCount(), 2);

    layer.removeEffect(0);
    ASSERT_EQ(layer.getEffectCount(), 1);
}

TEST(layer_move_effect) {
    Layer layer("Test Layer", LayerType::Solid);
    auto e1 = std::make_shared<GaussianBlurEffect>();
    auto e2 = std::make_shared<GaussianBlurEffect>();
    layer.addEffect(e1);
    layer.addEffect(e2);

    layer.moveEffect(0, 1);
    ASSERT_EQ(layer.getEffect(0)->getId(), e2->getId());
    ASSERT_EQ(layer.getEffect(1)->getId(), e1->getId());
    ASSERT_EQ(layer.getEffect(0)->getOrder(), 0);
    ASSERT_EQ(layer.getEffect(1)->getOrder(), 1);
}

TEST(layer_clear_effects) {
    Layer layer("Test Layer", LayerType::Solid);
    layer.addEffect(std::make_shared<GaussianBlurEffect>());
    layer.addEffect(std::make_shared<GaussianBlurEffect>());
    ASSERT_EQ(layer.getEffectCount(), 2);

    layer.clearEffects();
    ASSERT_EQ(layer.getEffectCount(), 0);
}

TEST(effect_registry_has_effects) {
    auto& registry = EffectRegistry::instance();
    ASSERT_TRUE(registry.hasEffect("Gaussian Blur"));
    ASSERT_TRUE(registry.hasEffect("Box Blur"));
    ASSERT_TRUE(registry.hasEffect("Brightness & Contrast"));
    ASSERT_TRUE(registry.hasEffect("Hue/Saturation"));
    ASSERT_FALSE(registry.hasEffect("Nonexistent Effect"));
}

TEST(effect_registry_create) {
    auto& registry = EffectRegistry::instance();
    auto effect = registry.create("Gaussian Blur");
    ASSERT_TRUE(effect != nullptr);
    ASSERT_EQ(effect->getName(), "Gaussian Blur");
    ASSERT_EQ(effect->getCategory(), "Blur & Sharpen");
}

TEST(effect_registry_categories) {
    auto& registry = EffectRegistry::instance();
    auto categories = registry.getCategories();
    ASSERT_TRUE(categories.size() > 0);
    bool foundBlur = false;
    for (const auto& cat : categories) {
        if (cat == "Blur & Sharpen") foundBlur = true;
    }
    ASSERT_TRUE(foundBlur);
}

TEST(effect_registry_effects_in_category) {
    auto& registry = EffectRegistry::instance();
    auto effects = registry.getEffectNamesInCategory("Blur & Sharpen");
    ASSERT_TRUE(effects.size() > 0);
    bool foundGaussian = false;
    for (const auto& name : effects) {
        if (name == "Gaussian Blur") foundGaussian = true;
    }
    ASSERT_TRUE(foundGaussian);
}

TEST(end_to_end_renderer_with_effect) {
    Composition comp("Test Comp", {100, 100}, {30.0}, 5.0);

    auto layer = comp.addLayer("Solid Layer", LayerType::Solid);
    layer->setStartTime(0.0);
    layer->setDuration(5.0);
    layer->getPosition().setDefaultValue(50.0);
    layer->getScale().setDefaultValue(15.0);
    layer->getOpacity().setDefaultValue(100.0);

    Renderer renderer;
    renderer.setResolution(100, 100);

    PixelBuffer frameWithoutEffect = renderer.renderFrame(comp, 0.5);

    auto effect = std::make_shared<GaussianBlurEffect>();
    effect->setParameterValue("radius", 10.0);
    layer->addEffect(effect);

    PixelBuffer frameWithEffect = renderer.renderFrame(comp, 0.5);

    bool pixelsChanged = false;
    for (int i = 0; i < 100 * 100 * 4 && !pixelsChanged; ++i) {
        if (frameWithoutEffect.data[i] != frameWithEffect.data[i]) {
            pixelsChanged = true;
        }
    }
    ASSERT_TRUE(pixelsChanged);
}

TEST(end_to_end_effect_disabled_no_change) {
    Composition comp("Test Comp", {100, 100}, {30.0}, 5.0);

    auto layer = comp.addLayer("Solid Layer", LayerType::Solid);
    layer->setStartTime(0.0);
    layer->setDuration(5.0);
    layer->getPosition().setDefaultValue(50.0);
    layer->getScale().setDefaultValue(15.0);
    layer->getOpacity().setDefaultValue(100.0);

    Renderer renderer;
    renderer.setResolution(100, 100);

    PixelBuffer frameWithoutEffect = renderer.renderFrame(comp, 0.5);

    auto effect = std::make_shared<GaussianBlurEffect>();
    effect->setParameterValue("radius", 10.0);
    effect->setEnabled(false);
    layer->addEffect(effect);

    PixelBuffer frameWithEffect = renderer.renderFrame(comp, 0.5);

    bool pixelsIdentical = true;
    for (int i = 0; i < 100 * 100 * 4 && pixelsIdentical; ++i) {
        if (frameWithoutEffect.data[i] != frameWithEffect.data[i]) {
            pixelsIdentical = false;
        }
    }
    ASSERT_TRUE(pixelsIdentical);
}

TEST(end_to_end_multiple_effects_applied) {
    Composition comp("Test Comp", {100, 100}, {30.0}, 5.0);

    auto layer = comp.addLayer("Solid Layer", LayerType::Solid);
    layer->setStartTime(0.0);
    layer->setDuration(5.0);
    layer->getPosition().setDefaultValue(50.0);
    layer->getScale().setDefaultValue(15.0);
    layer->getOpacity().setDefaultValue(100.0);

    auto blur = std::make_shared<GaussianBlurEffect>();
    blur->setParameterValue("radius", 5.0);
    layer->addEffect(blur);

    Renderer renderer;
    renderer.setResolution(100, 100);
    PixelBuffer frameOneEffect = renderer.renderFrame(comp, 0.5);

    auto blur2 = std::make_shared<GaussianBlurEffect>();
    blur2->setParameterValue("radius", 20.0);
    layer->addEffect(blur2);
    PixelBuffer frameTwoEffects = renderer.renderFrame(comp, 0.5);

    bool pixelsChanged = false;
    for (int i = 0; i < 100 * 100 * 4 && !pixelsChanged; ++i) {
        if (frameOneEffect.data[i] != frameTwoEffects.data[i]) {
            pixelsChanged = true;
        }
    }
    ASSERT_TRUE(pixelsChanged);
}
