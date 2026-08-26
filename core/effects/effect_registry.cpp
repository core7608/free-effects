#include "effect_registry.h"

// Expression Controls
#include "expression_controls/slider_control_effect.h"
#include "expression_controls/angle_control_effect.h"
#include "expression_controls/color_control_effect.h"
#include "expression_controls/point_control_effect.h"
#include "expression_controls/checkbox_control_effect.h"
#include "expression_controls/layer_control_effect.h"
#include "expression_controls/point_3d_control_effect.h"

// Channel Effects
#include "channel/shift_channels_effect.h"
#include "channel/set_channels_effect.h"
#include "channel/minimax_effect.h"
#include "channel/cc_composite_effect.h"
#include "channel/invert_effect.h"
#include "channel/arithmetic_effect.h"
#include "channel/blend_effect.h"
#include "channel/channel_combiner_effect.h"
#include "channel/maximum_effect.h"
#include "channel/minimum_effect.h"
#include "channel/remove_color_matting_effect.h"
#include "channel/set_matte_effect.h"
#include "channel/solid_composite_effect.h"

// Noise & Grain Effects
#include "noise/add_grain_effect.h"
#include "noise/remove_grain_effect.h"
#include "noise/dust_scratches_effect.h"
#include "noise/noise_hls_effect.h"
#include "noise/noise_hls_auto_effect.h"
#include "noise/turbulent_noise_effect.h"
#include "noise/match_grain_effect.h"
#include "noise/median_noise_effect.h"

// Distort Effects
#include "distort/turbulent_displace_effect.h"
#include "distort/mesh_warp_effect.h"
#include "distort/lens_distortion_effect.h"
#include "distort/cc_lens_effect.h"
#include "distort/displacement_map_effect.h"
#include "distort/warp_stabilizer_effect.h"
#include "distort/warp_effect.h"
#include "distort/offset_effect.h"
#include "distort/cc_bend_it_effect.h"
#include "distort/cc_bender_effect.h"
#include "distort/cc_power_pin_effect.h"
#include "distort/cc_slant_effect.h"
#include "distort/cc_tiler_effect.h"

// Stylize Effects
#include "stylize/roughen_edges_effect.h"
#include "stylize/scatter_effect.h"
#include "stylize/cc_kaleida_effect.h"
#include "stylize/cc_prism_effect.h"
#include "stylize/cc_block_load_effect.h"
#include "stylize/cc_burn_film_effect.h"
#include "stylize/cc_hex_tile_effect.h"
#include "stylize/cc_mr_smoothie_effect.h"
#include "stylize/cc_toner_effect.h"
#include "stylize/oil_paint_effect.h"
#include "stylize/color_halftone_effect.h"
#include "stylize/texturize_effect.h"
#include "stylize/wind_effect.h"
#include "stylize/cc_glass_stretch_effect.h"
#include "stylize/cc_sparkle_effect.h"
#include "stylize/cc_snowflake_effect.h"

// Transition Effects
#include "transition/card_wipe_effect.h"
#include "transition/linear_wipe_effect.h"
#include "transition/iris_wipe_effect.h"
#include "transition/cc_glass_wipe_effect.h"
#include "transition/cc_jaws_effect.h"
#include "transition/cc_light_wipe_effect.h"
#include "transition/cc_scale_wipe_effect.h"
#include "transition/cc_wedge_wipe_effect.h"

// Utility Effects
#include "utility/cineon_converter_effect.h"
#include "utility/color_profile_converter_effect.h"
#include "utility/cc_calculator_effect.h"
#include "utility/expand_effect.h"
#include "utility/pro_amp_effect.h"
#include "utility/reduce_interlace_flicker_effect.h"

// Color Correction Effects
#include "color/lumetri_color_effect.h"
#include "color/color_wheels_effect.h"
#include "color/color_balance_hls_effect.h"
#include "color/color_stabilizer_effect.h"
#include "color/change_color_effect.h"
#include "color/change_to_color_effect.h"
#include "color/color_balance_rgb_effect.h"
#include "color/equalize_effect.h"
#include "color/flicker_removal_effect.h"
#include "color/gamma_pedestal_gain_effect.h"
#include "color/leave_color_effect.h"
#include "color/lut_buddy_effect.h"
#include "color/reduce_noise_effect.h"
#include "color/white_balance_effect.h"
#include "color/saturation_effect.h"
#include "color/vignette_effect.h"
#include "color/gradient_map_effect.h"

// Blur & Sharpen Effects
#include "blur/camera_lens_blur_effect.h"
#include "blur/fast_box_blur_effect.h"
#include "blur/smart_blur_effect.h"
#include "blur/bilateral_blur_effect.h"
#include "blur/compound_blur_effect.h"
#include "blur/cc_cross_blur_effect.h"
#include "blur/cc_radial_blur_effect.h"
#include "blur/cc_vector_blur_effect.h"
#include "blur/channel_blur_effect.h"
#include "blur/depth_of_field_effect.h"

// Keying Effects
#include "keying/keylight_effect.h"
#include "keying/advanced_spill_suppressor_effect.h"
#include "keying/refine_hard_matte_effect.h"
#include "keying/refine_soft_matte_effect.h"
#include "keying/key_cleaner_effect.h"
#include "keying/simple_choker_effect.h"
#include "keying/inner_outer_key_effect.h"
#include "keying/spill_suppressor_effect.h"
#include "keying/extract_effect.h"

// Perspective Effects
#include "perspective/basic_3d_effect.h"
#include "perspective/cc_sphere_effect.h"
#include "perspective/depth_of_field_persp_effect.h"

// Simulation Effects
#include "simulation/cc_ball_action_effect.h"
#include "simulation/cc_pixel_polly_effect.h"
#include "simulation/card_dance_effect.h"
#include "simulation/caustics_effect.h"
#include "simulation/particle_playground_effect.h"

// 3D Channel Effects
#include "three_d_channel/channel_extract_effect.h"
#include "three_d_channel/depth_matte_effect.h"
#include "three_d_channel/fog_3d_effect.h"
#include "three_d_channel/id_matte_effect.h"
#include "three_d_channel/normality_effect.h"

// Paint Effects
#include "paint/paint_effect.h"
#include "paint/vector_paint_effect.h"

// Generate Effects
#include "generate/beam_effect.h"
#include "generate/cc_light_burst_effect.h"
#include "generate/cc_light_sweep_effect.h"
#include "generate/echo_space_effect.h"
#include "generate/vegas_effect.h"
#include "generate/circle_effect.h"
#include "generate/scribble_effect.h"
#include "generate/paint_bucket_effect.h"
#include "generate/fractal_effect.h"
#include "generate/cc_snow_effect.h"
#include "generate/cc_threads_effect.h"
#include "generate/hoop_effect.h"

namespace FreeEffect {

EffectRegistry& EffectRegistry::instance() {
    static EffectRegistry reg;
    return reg;
}

void EffectRegistry::registerEffect(const std::string& name, EffectFactory factory,
                                    const std::string& category, const std::string& subCategory) {
    auto it = m_nameIndex.find(name);
    if (it != m_nameIndex.end()) {
        m_effects[it->second].factory = std::move(factory);
        m_effects[it->second].category = category;
        m_effects[it->second].subCategory = subCategory;
        return;
    }
    m_nameIndex[name] = static_cast<int>(m_effects.size());
    m_effects.push_back({name, category, subCategory, std::move(factory)});
}

std::unique_ptr<Effect> EffectRegistry::create(const std::string& name) const {
    auto it = m_nameIndex.find(name);
    if (it != m_nameIndex.end()) {
        return m_effects[it->second].factory();
    }
    return nullptr;
}

bool EffectRegistry::hasEffect(const std::string& name) const {
    return m_nameIndex.find(name) != m_nameIndex.end();
}

std::vector<std::string> EffectRegistry::getEffectNames() const {
    std::vector<std::string> names;
    names.reserve(m_effects.size());
    for (const auto& e : m_effects) {
        names.push_back(e.name);
    }
    return names;
}

std::vector<std::string> EffectRegistry::getCategories() const {
    std::vector<std::string> cats;
    for (const auto& e : m_effects) {
        bool found = false;
        for (const auto& c : cats) {
            if (c == e.category) { found = true; break; }
        }
        if (!found) cats.push_back(e.category);
    }
    return cats;
}

std::vector<std::string> EffectRegistry::getEffectNamesInCategory(const std::string& category) const {
    std::vector<std::string> names;
    for (const auto& e : m_effects) {
        if (e.category == category) names.push_back(e.name);
    }
    return names;
}

std::vector<std::string> EffectRegistry::getEffectNamesInSubCategory(
    const std::string& category, const std::string& subCategory) const {
    std::vector<std::string> names;
    for (const auto& e : m_effects) {
        if (e.category == category && e.subCategory == subCategory) names.push_back(e.name);
    }
    return names;
}

} // namespace FreeEffect
