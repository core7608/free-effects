#include "test_framework.h"
#include "../core/timeline/types.h"
#include "../core/timeline/keyframe.h"
#include "../core/timeline/property_track.h"
#include "../core/timeline/layer.h"
#include "../core/timeline/composition.h"
#include <cmath>

using namespace FreeEffect;

TEST(uuid_generation) {
    UUID id1 = generateUUID();
    UUID id2 = generateUUID();
    ASSERT_TRUE(!id1.empty());
    ASSERT_TRUE(!id2.empty());
    ASSERT_NE(id1, id2);
}

TEST(uuid_format) {
    UUID id = generateUUID();
    // UUID v4 format: 8-4-4-4-12
    ASSERT_EQ(id.size(), 36u);
    ASSERT_EQ(id[8], '-');
    ASSERT_EQ(id[13], '-');
    ASSERT_EQ(id[18], '-');
    ASSERT_EQ(id[23], '-');
}

TEST(keyframe_creation) {
    Keyframe kf(0.0, 100.0, InterpolationType::Linear);
    ASSERT_EQ(kf.getTime(), 0.0);
    ASSERT_EQ(kf.getValue(), 100.0);
    ASSERT_EQ(kf.getInterpolation(), InterpolationType::Linear);
}

TEST(keyframe_linear_interpolation) {
    Keyframe kf1(0.0, 0.0, InterpolationType::Linear);
    Keyframe kf2(1.0, 100.0, InterpolationType::Linear);
    
    double mid = kf1.interpolate(kf2, 0.5);
    ASSERT_TRUE(std::abs(mid - 50.0) < 1e-6);
}

TEST(keyframe_ease_in_interpolation) {
    Keyframe kf1(0.0, 0.0, InterpolationType::EaseIn);
    Keyframe kf2(1.0, 100.0, InterpolationType::EaseIn);
    
    double mid = kf1.interpolate(kf2, 0.5);
    // EaseIn: t*t = 0.25, so value = 25
    ASSERT_TRUE(std::abs(mid - 25.0) < 1e-6);
}

TEST(keyframe_hold_interpolation) {
    Keyframe kf1(0.0, 0.0, InterpolationType::Hold);
    Keyframe kf2(1.0, 100.0, InterpolationType::Hold);
    
    double mid = kf1.interpolate(kf2, 0.5);
    ASSERT_EQ(mid, 0.0);
}

TEST(keyframe_edge_cases) {
    Keyframe kf1(0.0, 0.0, InterpolationType::Linear);
    Keyframe kf2(1.0, 100.0, InterpolationType::Linear);
    
    ASSERT_EQ(kf1.interpolate(kf2, -1.0), 0.0);
    ASSERT_EQ(kf1.interpolate(kf2, 2.0), 100.0);
    ASSERT_EQ(kf1.interpolate(kf2, 0.0), 0.0);
    ASSERT_EQ(kf1.interpolate(kf2, 1.0), 100.0);
}

TEST(property_track_add_keyframe) {
    PropertyTrack track("Position");
    track.addKeyframe(Keyframe(0.0, 100.0));
    track.addKeyframe(Keyframe(1.0, 200.0));
    
    ASSERT_EQ(track.getKeyframes().size(), 2u);
    ASSERT_TRUE(track.hasKeyframes());
}

TEST(property_track_sorted_insert) {
    PropertyTrack track("Position");
    track.addKeyframe(Keyframe(1.0, 200.0));
    track.addKeyframe(Keyframe(0.0, 100.0));
    
    ASSERT_TRUE(std::abs(track.getKeyframes()[0].getTime() - 0.0) < 1e-6);
    ASSERT_TRUE(std::abs(track.getKeyframes()[1].getTime() - 1.0) < 1e-6);
}

TEST(property_track_remove_keyframe) {
    PropertyTrack track("Position");
    track.addKeyframe(Keyframe(0.0, 100.0));
    track.addKeyframe(Keyframe(1.0, 200.0));
    track.removeKeyframe(0.0);
    
    ASSERT_EQ(track.getKeyframes().size(), 1u);
}

TEST(property_track_get_value) {
    PropertyTrack track("Position");
    track.addKeyframe(Keyframe(0.0, 0.0));
    track.addKeyframe(Keyframe(1.0, 100.0));
    
    ASSERT_TRUE(std::abs(track.getValueAtTime(0.5) - 50.0) < 1e-6);
}

TEST(property_track_default_value) {
    PropertyTrack track("Position");
    ASSERT_EQ(track.getDefaultValue(), 0.0);
    track.setDefaultValue(50.0);
    ASSERT_EQ(track.getDefaultValue(), 50.0);
}

TEST(property_track_no_keyframes) {
    PropertyTrack track("Position");
    ASSERT_EQ(track.getValueAtTime(0.5), 0.0);
    track.setDefaultValue(42.0);
    ASSERT_EQ(track.getValueAtTime(0.5), 42.0);
}

TEST(layer_creation) {
    Layer layer("Test Layer", LayerType::Video);
    ASSERT_TRUE(!layer.getId().empty());
    ASSERT_EQ(layer.getName(), "Test Layer");
    ASSERT_EQ(layer.getType(), LayerType::Video);
    ASSERT_TRUE(layer.isVisible());
    ASSERT_TRUE(layer.isAudioEnabled());
    ASSERT_FALSE(layer.isLocked());
    ASSERT_FALSE(layer.isSolo());
}

TEST(layer_transform_defaults) {
    Layer layer("Test", LayerType::Solid);
    ASSERT_EQ(layer.getPosition().getDefaultValue(), 0.0);
    ASSERT_EQ(layer.getScale().getDefaultValue(), 100.0);
    ASSERT_EQ(layer.getRotation().getDefaultValue(), 0.0);
    ASSERT_EQ(layer.getOpacity().getDefaultValue(), 100.0);
}

TEST(layer_active_at_time) {
    Layer layer("Test", LayerType::Video);
    layer.setStartTime(1.0);
    layer.setDuration(5.0);
    
    ASSERT_FALSE(layer.isActiveAtTime(0.5));
    ASSERT_TRUE(layer.isActiveAtTime(1.0));
    ASSERT_TRUE(layer.isActiveAtTime(3.0));
    ASSERT_TRUE(layer.isActiveAtTime(6.0));
    ASSERT_FALSE(layer.isActiveAtTime(7.0));
}

TEST(composition_creation) {
    Resolution res = {1920, 1080};
    FrameRate fps = {30.0};
    Composition comp("Test Comp", res, fps, 10.0);
    
    ASSERT_TRUE(!comp.getId().empty());
    ASSERT_EQ(comp.getName(), "Test Comp");
    ASSERT_EQ(comp.getResolution().width, 1920);
    ASSERT_EQ(comp.getResolution().height, 1080);
    ASSERT_TRUE(std::abs(comp.getFrameRate().fps - 30.0) < 1e-6);
    ASSERT_TRUE(std::abs(comp.getDuration() - 10.0) < 1e-6);
}

TEST(composition_add_remove_layer) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    
    auto layer = comp.addLayer("Layer 1", LayerType::Video);
    ASSERT_EQ(comp.getLayerCount(), 1);
    
    UUID layerId = layer->getId();
    comp.removeLayer(layerId);
    ASSERT_EQ(comp.getLayerCount(), 0);
}

TEST(composition_get_layer) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    comp.addLayer("Layer 1", LayerType::Video);
    comp.addLayer("Layer 2", LayerType::Audio);
    
    ASSERT_TRUE(comp.getLayer(0) != nullptr);
    ASSERT_TRUE(comp.getLayer(1) != nullptr);
    ASSERT_TRUE(comp.getLayer(2) == nullptr);
    ASSERT_TRUE(comp.getLayerByName("Layer 1") != nullptr);
    ASSERT_TRUE(comp.getLayerByName("Nonexistent") == nullptr);
}

TEST(composition_move_layer) {
    Composition comp("Test", {1920, 1080}, {30.0}, 10.0);
    comp.addLayer("Layer 1", LayerType::Video);
    comp.addLayer("Layer 2", LayerType::Audio);
    comp.addLayer("Layer 3", LayerType::Solid);
    
    comp.moveLayer(2, 0);
    ASSERT_EQ(comp.getLayer(0)->getName(), "Layer 3");
    ASSERT_EQ(comp.getLayer(1)->getName(), "Layer 1");
    ASSERT_EQ(comp.getLayer(2)->getName(), "Layer 2");
}
