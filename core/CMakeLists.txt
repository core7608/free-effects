option(FREEEFFECT_ENABLE_SIMULATION "Enable simulation effects" ON)
option(FREEEFFECT_ENABLE_PLUGIN_EFFECTS "Enable plugin effect equivalents" ON)
option(FREEEFFECT_ENABLE_TRANSITION_EFFECTS "Enable transition effects" ON)
option(FREEEFFECT_ENABLE_EXPRESSION_ENGINE "Enable expression engine" ON)
option(FREEEFFECT_ENABLE_TRACKING "Enable motion tracking" ON)
option(FREEEFFECT_ENABLE_PUPPET "Enable puppet tool" ON)
option(FREEEFFECT_ENABLE_GPU "Enable GPU-accelerated rendering" OFF)

# Core library - no Qt dependency
add_library(freeeffect_core STATIC
    # Timeline data model
    timeline/types.h
    timeline/keyframe.h
    timeline/keyframe.cpp
    timeline/keyframe_interpolation.h
    timeline/property_track.h
    timeline/property_track.cpp
    timeline/layer.h
    timeline/layer.cpp
    timeline/layer_style.h
    timeline/layer_style.cpp
    timeline/material_options.h
    timeline/composition.h
    timeline/composition.cpp
    
    # 3D Transform System
    timeline/transform_3d.h
    
    # Camera System
    timeline/camera.h
    timeline/camera.cpp
    
    # Light System
    timeline/light.h
    timeline/light.cpp
    
    # Mask System
    timeline/mask.h
    timeline/mask.cpp
    timeline/track_matte.h
    
    # Shape Layer
    timeline/shape_layer.h
    timeline/shape_layer.cpp
    timeline/shape_operations.h
    timeline/shape_operations.cpp
    
    # Text Animation
    timeline/text_animator.h
    timeline/text_features.h
    timeline/text_features.cpp
    
    # Keyframe Operations
    timeline/keyframe_operations.h
    timeline/keyframe_operations.cpp
    
    # Responsive Design
    timeline/responsive_design.h
    
    # Matrix Math
    math/matrix4.h
    
    # Command pattern for undo/redo
    commands/command.h
    commands/command.cpp
    commands/command_stack.h
    commands/command_stack.cpp
    commands/add_layer_command.h
    commands/add_layer_command.cpp
    commands/remove_layer_command.h
    commands/remove_layer_command.cpp
    commands/move_layer_command.h
    commands/move_layer_command.cpp
    commands/set_keyframe_command.h
    commands/set_keyframe_command.cpp
    commands/remove_keyframe_command.h
    commands/remove_keyframe_command.cpp
    commands/set_property_command.h
    commands/set_property_command.cpp
    commands/layer_operations.h
    commands/layer_operations.cpp
    commands/precompose_command.h
    commands/precompose_command.cpp
    commands/split_layer_command.h
    commands/split_layer_command.cpp
    commands/freeze_frame_command.h
    commands/freeze_frame_command.cpp
    commands/time_reverse_command.h
    commands/time_reverse_command.cpp
    commands/auto_trace_command.h
    commands/auto_trace_command.cpp
    commands/create_shapes_from_text.h
    commands/create_shapes_from_text.cpp
    commands/create_masks_from_text.h
    commands/create_masks_from_text.cpp
    
    # Tools
    tools/tool_interface.h
    tools/pen_tool.h
    tools/pen_tool.cpp
    tools/shape_tools.h
    tools/shape_tools.cpp
    tools/freehand_tool.h
    tools/freehand_tool.cpp
    tools/selection_tool.h
    tools/selection_tool.cpp
    tools/brush_tools.h
    tools/brush_tools.cpp
    tools/camera_tools.h
    tools/camera_tools.cpp
    tools/puppet_tools.h
    tools/puppet_tools.cpp
    tools/transform_tools.h
    tools/transform_tools.cpp
    
    # Asset management
    project/asset_reference.h
    project/asset_reference.cpp
    project/project_state.h
    project/project_state.cpp
    project/missing_footage_handler.h
    project/missing_footage_handler.cpp
    project/auto_save.h
    project/auto_save.cpp
    project/project_manager.h
    project/project_manager.cpp
    project/footage_interpreter.h
    project/footage_interpreter.cpp
    project/solids_library.h
    project/solids_library.cpp
    project/project_manager_ext.h
    project/project_manager_ext.cpp
    
    # Rendering
    rendering/renderer.h
    rendering/renderer.cpp
    rendering/blend_modes.h
    rendering/blend_modes.cpp
    rendering/motion_blur.h
    rendering/motion_blur.cpp
    rendering/render_queue.h
    rendering/render_queue.cpp
    rendering/opengl_renderer.h
    rendering/opengl_renderer.cpp
    rendering/scene_renderer.h
    rendering/scene_renderer.cpp
    rendering/material.h
    rendering/c4d_renderer.h
    rendering/c4d_renderer.cpp
    
    # Audio
    audio/audio_decoder.h
    audio/audio_decoder.cpp
    audio/audio_effects.h
    audio/audio_effects.cpp
    audio/waveform_generator.h
    audio/waveform_generator.cpp
    
    # I/O
    io/importer.h
    io/importer.cpp
    io/exporter.h
    io/exporter.cpp
    io/project_file.h
    io/project_file.cpp
    io/psd_importer.h
    io/psd_importer.cpp
    io/ai_importer.h
    io/ai_importer.cpp
    io/camera_data_importer.h
    io/camera_data_importer.cpp
    io/obj_importer.h
    io/obj_importer.cpp
    io/fbx_importer.h
    io/fbx_importer.cpp
    io/c4d_importer.h
    io/c4d_importer.cpp
    io/exr_importer.h
    io/exr_importer.cpp
    io/hdr_importer.h
    io/hdr_importer.cpp
    io/dpx_importer.h
    io/dpx_importer.cpp
    io/tga_importer.h
    io/tga_importer.cpp
    io/pdf_importer.h
    io/pdf_importer.cpp
    io/camera_raw_importer.h
    io/camera_raw_importer.cpp
    io/prores_exporter.h
    io/prores_exporter.cpp
    io/dnxhd_exporter.h
    io/dnxhd_exporter.cpp
    io/mxf_exporter.h
    io/mxf_exporter.cpp
    io/image_sequence_exporter.h
    io/image_sequence_exporter.cpp
    
    # Effects System
    effects/effect.h
    effects/effect.cpp
    effects/effect_registry.h
    effects/effect_registry.cpp
    effects/effect_serializer.h
    effects/effect_serializer.cpp
    
    # Blur & Sharpen Effects
    effects/blur/gaussian_blur_effect.h
    effects/blur/gaussian_blur_effect.cpp
    effects/blur/box_blur_effect.h
    effects/blur/box_blur_effect.cpp
    effects/blur/radial_blur_effect.h
    effects/blur/radial_blur_effect.cpp
    effects/blur/motion_blur_effect.h
    effects/blur/motion_blur_effect.cpp
    effects/blur/sharpen_effect.h
    effects/blur/sharpen_effect.cpp
    effects/blur/unsharp_mask_effect.h
    effects/blur/unsharp_mask_effect.cpp
    effects/blur/camera_lens_blur_effect.h
    effects/blur/camera_lens_blur_effect.cpp
    effects/blur/fast_box_blur_effect.h
    effects/blur/fast_box_blur_effect.cpp
    effects/blur/smart_blur_effect.h
    effects/blur/smart_blur_effect.cpp
    effects/blur/bilateral_blur_effect.h
    effects/blur/bilateral_blur_effect.cpp
    effects/blur/compound_blur_effect.h
    effects/blur/compound_blur_effect.cpp
    effects/blur/cc_cross_blur_effect.h
    effects/blur/cc_cross_blur_effect.cpp
    effects/blur/cc_radial_blur_effect.h
    effects/blur/cc_radial_blur_effect.cpp
    effects/blur/cc_vector_blur_effect.h
    effects/blur/cc_vector_blur_effect.cpp
    effects/blur/channel_blur_effect.h
    effects/blur/channel_blur_effect.cpp
    effects/blur/depth_of_field_effect.h
    effects/blur/depth_of_field_effect.cpp
    
    # Color Correction Effects
    effects/color/levels_effect.h
    effects/color/levels_effect.cpp
    effects/color/curves_effect.h
    effects/color/curves_effect.cpp
    effects/color/hue_saturation_effect.h
    effects/color/hue_saturation_effect.cpp
    effects/color/brightness_contrast_effect.h
    effects/color/brightness_contrast_effect.cpp
    effects/color/color_balance_effect.h
    effects/color/color_balance_effect.cpp
    effects/color/exposure_effect.h
    effects/color/exposure_effect.cpp
    effects/color/photo_filter_effect.h
    effects/color/photo_filter_effect.cpp
    effects/color/tint_effect.h
    effects/color/tint_effect.cpp
    effects/color/tritone_effect.h
    effects/color/tritone_effect.cpp
    effects/color/vibrance_effect.h
    effects/color/vibrance_effect.cpp
    
    # Distort Effects
    effects/distort/wave_warp_effect.h
    effects/distort/wave_warp_effect.cpp
    effects/distort/bulge_effect.h
    effects/distort/bulge_effect.cpp
    effects/distort/spherize_effect.h
    effects/distort/spherize_effect.cpp
    effects/distort/pinch_effect.h
    effects/distort/pinch_effect.cpp
    effects/distort/twirl_effect.h
    effects/distort/twirl_effect.cpp
    effects/distort/ripple_effect.h
    effects/distort/ripple_effect.cpp
    effects/distort/magnify_effect.h
    effects/distort/magnify_effect.cpp
    effects/distort/displacement_map_effect.h
    effects/distort/displacement_map_effect.cpp
    effects/distort/warp_stabilizer_effect.h
    effects/distort/warp_stabilizer_effect.cpp
    effects/distort/warp_effect.h
    effects/distort/warp_effect.cpp
    effects/distort/offset_effect.h
    effects/distort/offset_effect.cpp
    effects/distort/cc_bend_it_effect.h
    effects/distort/cc_bend_it_effect.cpp
    effects/distort/cc_bender_effect.h
    effects/distort/cc_bender_effect.cpp
    effects/distort/cc_power_pin_effect.h
    effects/distort/cc_power_pin_effect.cpp
    effects/distort/cc_slant_effect.h
    effects/distort/cc_slant_effect.cpp
    effects/distort/cc_tiler_effect.h
    effects/distort/cc_tiler_effect.cpp
    
    # Generate Effects
    effects/generate/fill_effect.h
    effects/generate/fill_effect.cpp
    effects/generate/gradient_ramp_effect.h
    effects/generate/gradient_ramp_effect.cpp
    effects/generate/stroke_effect.h
    effects/generate/stroke_effect.cpp
    effects/generate/ellipse_effect.h
    effects/generate/ellipse_effect.cpp
    effects/generate/rectangle_effect.h
    effects/generate/rectangle_effect.cpp
    effects/generate/lens_flare_effect.h
    effects/generate/lens_flare_effect.cpp
    effects/generate/grid_effect.h
    effects/generate/grid_effect.cpp
    effects/generate/checkerboard_effect.h
    effects/generate/checkerboard_effect.cpp
    effects/generate/cell_pattern_effect.h
    effects/generate/cell_pattern_effect.cpp
    effects/generate/audio_spectrum_effect.h
    effects/generate/audio_spectrum_effect.cpp
    effects/generate/beam_effect.h
    effects/generate/beam_effect.cpp
    effects/generate/cc_light_burst_effect.h
    effects/generate/cc_light_burst_effect.cpp
    effects/generate/cc_light_sweep_effect.h
    effects/generate/cc_light_sweep_effect.cpp
    effects/generate/echo_space_effect.h
    effects/generate/echo_space_effect.cpp
    effects/generate/vegas_effect.h
    effects/generate/vegas_effect.cpp
    effects/generate/circle_effect.h
    effects/generate/circle_effect.cpp
    effects/generate/scribble_effect.h
    effects/generate/scribble_effect.cpp
    effects/generate/paint_bucket_effect.h
    effects/generate/paint_bucket_effect.cpp
    effects/generate/fractal_effect.h
    effects/generate/fractal_effect.cpp
    effects/generate/cc_snow_effect.h
    effects/generate/cc_snow_effect.cpp
    effects/generate/cc_threads_effect.h
    effects/generate/cc_threads_effect.cpp
    effects/generate/hoop_effect.h
    effects/generate/hoop_effect.cpp
    
    # Stylize Effects
    effects/stylize/glow_effect.h
    effects/stylize/glow_effect.cpp
    effects/stylize/directional_blur_effect.h
    effects/stylize/directional_blur_effect.cpp
    effects/stylize/drop_shadow_effect.h
    effects/stylize/drop_shadow_effect.cpp
    effects/stylize/inner_glow_effect.h
    effects/stylize/inner_glow_effect.cpp
    effects/stylize/bevel_alpha_effect.h
    effects/stylize/bevel_alpha_effect.cpp
    effects/stylize/brush_strokes_effect.h
    effects/stylize/brush_strokes_effect.cpp
    effects/stylize/find_edges_effect.h
    effects/stylize/find_edges_effect.cpp
    effects/stylize/posterize_effect.h
    effects/stylize/posterize_effect.cpp
    effects/stylize/threshold_effect.h
    effects/stylize/threshold_effect.cpp
    effects/stylize/noise_effect.h
    effects/stylize/noise_effect.cpp
    effects/stylize/noise_alpha_effect.h
    effects/stylize/noise_alpha_effect.cpp
    effects/stylize/fractal_noise_effect.h
    effects/stylize/fractal_noise_effect.cpp
    effects/stylize/cc_glass_effect.h
    effects/stylize/cc_glass_effect.cpp
    effects/stylize/mosaic_effect.h
    effects/stylize/mosaic_effect.cpp
    effects/stylize/motion_tile_effect.h
    effects/stylize/motion_tile_effect.cpp
    effects/stylize/cc_kaleida_effect.h
    effects/stylize/cc_kaleida_effect.cpp
    effects/stylize/cc_prism_effect.h
    effects/stylize/cc_prism_effect.cpp
    effects/stylize/cc_block_load_effect.h
    effects/stylize/cc_block_load_effect.cpp
    effects/stylize/cc_burn_film_effect.h
    effects/stylize/cc_burn_film_effect.cpp
    effects/stylize/cc_hex_tile_effect.h
    effects/stylize/cc_hex_tile_effect.cpp
    effects/stylize/cc_mr_smoothie_effect.h
    effects/stylize/cc_mr_smoothie_effect.cpp
    effects/stylize/cc_toner_effect.h
    effects/stylize/cc_toner_effect.cpp
    effects/stylize/oil_paint_effect.h
    effects/stylize/oil_paint_effect.cpp
    effects/stylize/color_halftone_effect.h
    effects/stylize/color_halftone_effect.cpp
    effects/stylize/texturize_effect.h
    effects/stylize/texturize_effect.cpp
    effects/stylize/wind_effect.h
    effects/stylize/wind_effect.cpp
    effects/stylize/cc_glass_stretch_effect.h
    effects/stylize/cc_glass_stretch_effect.cpp
    effects/stylize/cc_sparkle_effect.h
    effects/stylize/cc_sparkle_effect.cpp
    effects/stylize/cc_snowflake_effect.h
    effects/stylize/cc_snowflake_effect.cpp
    
    # Keying Effects
    effects/keying/color_key_effect.h
    effects/keying/color_key_effect.cpp
    effects/keying/linear_color_key_effect.h
    effects/keying/linear_color_key_effect.cpp
    effects/keying/color_range_effect.h
    effects/keying/color_range_effect.cpp
    effects/keying/difference_matte_effect.h
    effects/keying/difference_matte_effect.cpp
    effects/keying/keylight_effect.h
    effects/keying/keylight_effect.cpp
    effects/keying/advanced_spill_suppressor_effect.h
    effects/keying/advanced_spill_suppressor_effect.cpp
    effects/keying/refine_hard_matte_effect.h
    effects/keying/refine_hard_matte_effect.cpp
    effects/keying/refine_soft_matte_effect.h
    effects/keying/refine_soft_matte_effect.cpp
    effects/keying/key_cleaner_effect.h
    effects/keying/key_cleaner_effect.cpp
    effects/keying/simple_choker_effect.h
    effects/keying/simple_choker_effect.cpp
    effects/keying/inner_outer_key_effect.h
    effects/keying/inner_outer_key_effect.cpp
    effects/keying/spill_suppressor_effect.h
    effects/keying/spill_suppressor_effect.cpp
    effects/keying/extract_effect.h
    effects/keying/extract_effect.cpp
    
    # Perspective Effects
    effects/perspective/cc_cylinder_effect.h
    effects/perspective/cc_cylinder_effect.cpp
    effects/perspective/bevel_edges_effect.h
    effects/perspective/bevel_edges_effect.cpp
    effects/perspective/perspective_drop_shadow_effect.h
    effects/perspective/perspective_drop_shadow_effect.cpp
    effects/perspective/basic_3d_effect.h
    effects/perspective/basic_3d_effect.cpp
    effects/perspective/cc_sphere_effect.h
    effects/perspective/cc_sphere_effect.cpp
    effects/perspective/depth_of_field_persp_effect.h
    effects/perspective/depth_of_field_persp_effect.cpp
    
    # Utility Effects
    effects/utility/transform_effect.h
    effects/utility/transform_effect.cpp
    effects/utility/tile_effect.h
    effects/utility/tile_effect.cpp
    effects/utility/time_displacement_effect.h
    effects/utility/time_displacement_effect.cpp
    effects/utility/posterize_time_effect.h
    effects/utility/posterize_time_effect.cpp
    effects/utility/cineon_converter_effect.h
    effects/utility/cineon_converter_effect.cpp
    effects/utility/color_profile_converter_effect.h
    effects/utility/color_profile_converter_effect.cpp
    effects/utility/cc_calculator_effect.h
    effects/utility/cc_calculator_effect.cpp
    effects/utility/expand_effect.h
    effects/utility/expand_effect.cpp
    effects/utility/pro_amp_effect.h
    effects/utility/pro_amp_effect.cpp
    effects/utility/reduce_interlace_flicker_effect.h
    effects/utility/reduce_interlace_flicker_effect.cpp
    
    # Plugin System
    plugins/plugin_interface.h
    plugins/plugin_manager.h
    plugins/plugin_manager.cpp
    plugins/plugin_registry.h
    plugins/plugin_registry.cpp
    
    # Built-in Plugins
    plugins/builtin/color_grading_plugin.h
    plugins/builtin/color_grading_plugin.cpp
    plugins/builtin/lens_effects_plugin.h
    plugins/builtin/lens_effects_plugin.cpp
    plugins/builtin/particle_generator_plugin.h
    plugins/builtin/particle_generator_plugin.cpp
    plugins/builtin/text_animator_plugin.h
    plugins/builtin/text_animator_plugin.cpp
    plugins/builtin/transition_pack_plugin.h
    plugins/builtin/transition_pack_plugin.cpp
    
    # Templates
    templates/template_data.h
    templates/template_manager.h
    templates/template_manager.cpp
    templates/essential_graphics.h
    templates/essential_graphics.cpp
    
    # Effects Presets
    effects/presets/preset_data.h
    effects/presets/preset_manager.h
    effects/presets/preset_manager.cpp
    
    # Simulation Effects (conditional)
    # Transition Effects (conditional)
    
    # Additional Stylize Effects
    effects/stylize/cartoon_effect.h
    effects/stylize/cartoon_effect.cpp
    effects/stylize/color_emboss_effect.h
    effects/stylize/color_emboss_effect.cpp
    effects/stylize/emboss_effect.h
    effects/stylize/emboss_effect.cpp
    effects/stylize/strobe_effect.h
    effects/stylize/strobe_effect.cpp
    effects/stylize/median_effect.h
    effects/stylize/median_effect.cpp
    effects/stylize/roughen_edges_effect.h
    effects/stylize/roughen_edges_effect.cpp
    effects/stylize/scatter_effect.h
    effects/stylize/scatter_effect.cpp
    
    # Additional Distort Effects
    effects/distort/liquify_effect.h
    effects/distort/liquify_effect.cpp
    effects/distort/mirror_effect.h
    effects/distort/mirror_effect.cpp
    effects/distort/optical_compensation_effect.h
    effects/distort/optical_compensation_effect.cpp
    effects/distort/polar_coordinates_effect.h
    effects/distort/polar_coordinates_effect.cpp
    effects/distort/corner_pin_effect.h
    effects/distort/corner_pin_effect.cpp
    effects/distort/turbulent_displace_effect.h
    effects/distort/turbulent_displace_effect.cpp
    effects/distort/mesh_warp_effect.h
    effects/distort/mesh_warp_effect.cpp
    effects/distort/lens_distortion_effect.h
    effects/distort/lens_distortion_effect.cpp
    effects/distort/cc_lens_effect.h
    effects/distort/cc_lens_effect.cpp
    
    # Expression Controls Effects
    effects/expression_controls/slider_control_effect.h
    effects/expression_controls/slider_control_effect.cpp
    effects/expression_controls/angle_control_effect.h
    effects/expression_controls/angle_control_effect.cpp
    effects/expression_controls/color_control_effect.h
    effects/expression_controls/color_control_effect.cpp
    effects/expression_controls/point_control_effect.h
    effects/expression_controls/point_control_effect.cpp
    effects/expression_controls/checkbox_control_effect.h
    effects/expression_controls/checkbox_control_effect.cpp
    effects/expression_controls/layer_control_effect.h
    effects/expression_controls/layer_control_effect.cpp
    effects/expression_controls/point_3d_control_effect.h
    effects/expression_controls/point_3d_control_effect.cpp
    
    # Channel Effects
    effects/channel/shift_channels_effect.h
    effects/channel/shift_channels_effect.cpp
    effects/channel/set_channels_effect.h
    effects/channel/set_channels_effect.cpp
    effects/channel/minimax_effect.h
    effects/channel/minimax_effect.cpp
    effects/channel/cc_composite_effect.h
    effects/channel/cc_composite_effect.cpp
    effects/channel/invert_effect.h
    effects/channel/invert_effect.cpp
    effects/channel/arithmetic_effect.h
    effects/channel/arithmetic_effect.cpp
    effects/channel/blend_effect.h
    effects/channel/blend_effect.cpp
    effects/channel/channel_combiner_effect.h
    effects/channel/channel_combiner_effect.cpp
    effects/channel/maximum_effect.h
    effects/channel/maximum_effect.cpp
    effects/channel/minimum_effect.h
    effects/channel/minimum_effect.cpp
    effects/channel/remove_color_matting_effect.h
    effects/channel/remove_color_matting_effect.cpp
    effects/channel/set_matte_effect.h
    effects/channel/set_matte_effect.cpp
    effects/channel/solid_composite_effect.h
    effects/channel/solid_composite_effect.cpp
    
    # Noise & Grain Effects
    effects/noise/add_grain_effect.h
    effects/noise/add_grain_effect.cpp
    effects/noise/remove_grain_effect.h
    effects/noise/remove_grain_effect.cpp
    effects/noise/dust_scratches_effect.h
    effects/noise/dust_scratches_effect.cpp
    effects/noise/noise_hls_effect.h
    effects/noise/noise_hls_effect.cpp
    effects/noise/noise_hls_auto_effect.h
    effects/noise/noise_hls_auto_effect.cpp
    effects/noise/turbulent_noise_effect.h
    effects/noise/turbulent_noise_effect.cpp
    effects/noise/match_grain_effect.h
    effects/noise/match_grain_effect.cpp
    effects/noise/median_noise_effect.h
    effects/noise/median_noise_effect.cpp
    
    # Additional Color Effects
    effects/color/selective_color_effect.h
    effects/color/selective_color_effect.cpp
    effects/color/shadow_highlight_effect.h
    effects/color/shadow_highlight_effect.cpp
    effects/color/channel_mixer_effect.h
    effects/color/channel_mixer_effect.cpp
    effects/color/lumetri_color_effect.h
    effects/color/lumetri_color_effect.cpp
    effects/color/color_wheels_effect.h
    effects/color/color_wheels_effect.cpp
    effects/color/color_balance_hls_effect.h
    effects/color/color_balance_hls_effect.cpp
    effects/color/color_stabilizer_effect.h
    effects/color/color_stabilizer_effect.cpp
    effects/color/change_color_effect.h
    effects/color/change_color_effect.cpp
    effects/color/change_to_color_effect.h
    effects/color/change_to_color_effect.cpp
    effects/color/color_balance_rgb_effect.h
    effects/color/color_balance_rgb_effect.cpp
    effects/color/equalize_effect.h
    effects/color/equalize_effect.cpp
    effects/color/flicker_removal_effect.h
    effects/color/flicker_removal_effect.cpp
    effects/color/gamma_pedestal_gain_effect.h
    effects/color/gamma_pedestal_gain_effect.cpp
    effects/color/leave_color_effect.h
    effects/color/leave_color_effect.cpp
    effects/color/lut_buddy_effect.h
    effects/color/lut_buddy_effect.cpp
    effects/color/reduce_noise_effect.h
    effects/color/reduce_noise_effect.cpp
    effects/color/white_balance_effect.h
    effects/color/white_balance_effect.cpp
    effects/color/saturation_effect.h
    effects/color/saturation_effect.cpp
    effects/color/vignette_effect.h
    effects/color/vignette_effect.cpp
    effects/color/gradient_map_effect.h
    effects/color/gradient_map_effect.cpp
    
    # Additional Generate Effects
    effects/generate/advanced_lightning_effect.h
    effects/generate/advanced_lightning_effect.cpp
    effects/generate/write_on_effect.h
    effects/generate/write_on_effect.cpp
    effects/generate/audio_waveform_effect.h
    effects/generate/audio_waveform_effect.cpp
    effects/generate/radio_waves_effect.h
    effects/generate/radio_waves_effect.cpp
    
    # Additional Keying Effects
    effects/keying/luma_key_effect.h
    effects/keying/luma_key_effect.cpp
    
    # 3D Channel Effects
    effects/three_d_channel/channel_extract_effect.h
    effects/three_d_channel/channel_extract_effect.cpp
    effects/three_d_channel/depth_matte_effect.h
    effects/three_d_channel/depth_matte_effect.cpp
    effects/three_d_channel/fog_3d_effect.h
    effects/three_d_channel/fog_3d_effect.cpp
    effects/three_d_channel/id_matte_effect.h
    effects/three_d_channel/id_matte_effect.cpp
    effects/three_d_channel/normality_effect.h
    effects/three_d_channel/normality_effect.cpp
    
    # Paint Effects
    effects/paint/paint_effect.h
    effects/paint/paint_effect.cpp
    effects/paint/vector_paint_effect.h
    effects/paint/vector_paint_effect.cpp
    
    # Third-Party Plugin Effect Equivalents (conditional)
    
    # Expression Engine (conditional)
    
    # Expression Library (conditional)
    
    # Motion Tracking (conditional)
    
    # Puppet Tool & Data-Driven Animation (conditional)
    animation/data_driven.h
    animation/data_driven.cpp
    animation/graph_editor.h
    animation/graph_editor.cpp
    animation/keyframe_assistant.h
    animation/keyframe_assistant.cpp
    
    # Preferences System
    preferences/preferences.h
    preferences/preferences.cpp
    
    # Color Management
    color/color_management.h
    color/color_management.cpp
    
    # Performance
    performance/frame_cache.h
    performance/frame_cache.cpp
    performance/disk_cache.h
    performance/disk_cache.cpp
    performance/spatial_grid.h
    performance/spatial_grid.cpp
    performance/profiler.h
    performance/profiler.cpp
    performance/render_worker.h
    performance/render_worker.cpp

    # Collaboration & Team Features
    collaboration/team_project.h
    collaboration/team_project.cpp
    collaboration/asset_library.h
    collaboration/asset_library.cpp
)

# ── Conditional feature modules ──────────────────────────────────
option(FREEEFFECT_ENABLE_SCRIPTING "Enable ExtendScript engine" ON)
if(FREEEFFECT_ENABLE_SCRIPTING)
    target_sources(freeeffect_core PRIVATE
        scripting/extend_script_engine.h
        scripting/extend_script_engine.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_SCRIPTING)
endif()

if(FREEEFFECT_ENABLE_GPU)
    target_sources(freeeffect_core PRIVATE
        rendering/gpu_renderer.h
        rendering/gpu_renderer.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_GPU)
endif()

if(FREEEFFECT_ENABLE_SIMULATION)
    target_sources(freeeffect_core PRIVATE
        effects/simulation/particle_burst_effect.h
        effects/simulation/particle_burst_effect.cpp
        effects/simulation/rain_effect.h
        effects/simulation/rain_effect.cpp
        effects/simulation/snow_effect.h
        effects/simulation/snow_effect.cpp
        effects/simulation/fire_effect.h
        effects/simulation/fire_effect.cpp
        effects/simulation/smoke_effect.h
        effects/simulation/smoke_effect.cpp
        effects/simulation/sparkle_effect.h
        effects/simulation/sparkle_effect.cpp
        effects/simulation/bubbles_effect.h
        effects/simulation/bubbles_effect.cpp
        effects/simulation/energy_field_effect.h
        effects/simulation/energy_field_effect.cpp
        effects/simulation/shatter_effect.h
        effects/simulation/shatter_effect.cpp
        effects/simulation/wave_effect.h
        effects/simulation/wave_effect.cpp
        effects/simulation/lightning_effect.h
        effects/simulation/lightning_effect.cpp
        effects/simulation/pixel_dissolve_effect.h
        effects/simulation/pixel_dissolve_effect.cpp
        effects/simulation/disintegrate_effect.h
        effects/simulation/disintegrate_effect.cpp
        effects/simulation/galaxy_effect.h
        effects/simulation/galaxy_effect.cpp
        effects/simulation/matrix_rain_effect.h
        effects/simulation/matrix_rain_effect.cpp
        effects/simulation/cc_ball_action_effect.h
        effects/simulation/cc_ball_action_effect.cpp
        effects/simulation/cc_pixel_polly_effect.h
        effects/simulation/cc_pixel_polly_effect.cpp
        effects/simulation/card_dance_effect.h
        effects/simulation/card_dance_effect.cpp
        effects/simulation/caustics_effect.h
        effects/simulation/caustics_effect.cpp
        effects/simulation/particle_playground_effect.h
        effects/simulation/particle_playground_effect.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_SIMULATION)
endif()

if(FREEEFFECT_ENABLE_PLUGIN_EFFECTS)
    target_sources(freeeffect_core PRIVATE
        effects/plugin_effect/saber_effect.h
        effects/plugin_effect/saber_effect.cpp
        effects/plugin_effect/optical_flare_effect.h
        effects/plugin_effect/optical_flare_effect.cpp
        effects/plugin_effect/plexus_effect.h
        effects/plugin_effect/plexus_effect.cpp
        effects/plugin_effect/shine_effect.h
        effects/plugin_effect/shine_effect.cpp
        effects/plugin_effect/starglow_effect.h
        effects/plugin_effect/starglow_effect.cpp
        effects/plugin_effect/particular_effect.h
        effects/plugin_effect/particular_effect.cpp
        effects/plugin_effect/form_effect.h
        effects/plugin_effect/form_effect.cpp
        effects/plugin_effect/mir_effect.h
        effects/plugin_effect/mir_effect.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_PLUGIN_EFFECTS)
endif()

if(FREEEFFECT_ENABLE_TRANSITION_EFFECTS)
    target_sources(freeeffect_core PRIVATE
        effects/transition/cross_dissolve_effect.h
        effects/transition/cross_dissolve_effect.cpp
        effects/transition/wipe_left_effect.h
        effects/transition/wipe_left_effect.cpp
        effects/transition/zoom_blur_transition.h
        effects/transition/zoom_blur_transition.cpp
        effects/transition/iris_transition.h
        effects/transition/iris_transition.cpp
        effects/transition/block_dissolve_effect.h
        effects/transition/block_dissolve_effect.cpp
        effects/transition/directional_wipe_effect.h
        effects/transition/directional_wipe_effect.cpp
        effects/transition/radial_wipe_effect.h
        effects/transition/radial_wipe_effect.cpp
        effects/transition/gradient_wipe_effect.h
        effects/transition/gradient_wipe_effect.cpp
        effects/transition/venetian_blinds_effect.h
        effects/transition/venetian_blinds_effect.cpp
        effects/transition/pixelate_transition.h
        effects/transition/pixelate_transition.cpp
        effects/transition/morph_transition.h
        effects/transition/morph_transition.cpp
    effects/transition/glitch_transition.h
    effects/transition/glitch_transition.cpp
    effects/transition/card_wipe_effect.h
    effects/transition/card_wipe_effect.cpp
    effects/transition/linear_wipe_effect.h
    effects/transition/linear_wipe_effect.cpp
    effects/transition/iris_wipe_effect.h
    effects/transition/iris_wipe_effect.cpp
    effects/transition/cc_glass_wipe_effect.h
    effects/transition/cc_glass_wipe_effect.cpp
    effects/transition/cc_jaws_effect.h
    effects/transition/cc_jaws_effect.cpp
    effects/transition/cc_light_wipe_effect.h
    effects/transition/cc_light_wipe_effect.cpp
    effects/transition/cc_scale_wipe_effect.h
    effects/transition/cc_scale_wipe_effect.cpp
    effects/transition/cc_wedge_wipe_effect.h
    effects/transition/cc_wedge_wipe_effect.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_TRANSITION_EFFECTS)
endif()

if(FREEEFFECT_ENABLE_EXPRESSION_ENGINE)
    target_sources(freeeffect_core PRIVATE
        expressions/expression_engine.h
        expressions/expression_engine.cpp
        expressions/expression_library.h
        expressions/expression_library.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_EXPRESSION_ENGINE)
endif()

if(FREEEFFECT_ENABLE_TRACKING)
    target_sources(freeeffect_core PRIVATE
        tracking/motion_tracker.h
        tracking/motion_tracker.cpp
        tracking/rotoscoping.h
        tracking/rotoscoping.cpp
        tracking/camera_tracker.h
        tracking/camera_tracker.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_TRACKING)
endif()

if(FREEEFFECT_ENABLE_PUPPET)
    target_sources(freeeffect_core PRIVATE
        animation/puppet_tool.h
        animation/puppet_tool.cpp
    )
    target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_ENABLE_PUPPET)
endif()

target_include_directories(freeeffect_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(freeeffect_core PRIVATE 
    FREEEFFECT_BUILTIN_PLUGIN
    _USE_MATH_DEFINES
    NOMINMAX
)

# Plugin system: link dl for dlopen/dlclose on Linux, stdc++fs for filesystem
if(NOT WIN32)
    target_link_libraries(freeeffect_core PRIVATE ${CMAKE_DL_LIBS})
endif()

# Optional: Find OpenGL (for future GPU rendering)
find_package(OpenGL QUIET)
if(OpenGL_FOUND)
    target_link_libraries(freeeffect_core PUBLIC OpenGL::GL)
endif()

# Optional: Find nlohmann/json (for future JSON serialization)
find_package(nlohmann_json 3.11.0 QUIET)
if(nlohmann_json_FOUND)
    target_link_libraries(freeeffect_core PUBLIC nlohmann_json::nlohmann_json)
    target_compile_definitions(freeeffect_core PRIVATE HAS_NLOHMANN_JSON)
endif()

# Optional: Find FFmpeg
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(AVFORMAT QUIET libavformat)
    pkg_check_modules(AVCODEC QUIET libavcodec)
    pkg_check_modules(AVUTIL QUIET libavutil)
    pkg_check_modules(SWSCALE QUIET libswscale)
    pkg_check_modules(SWRESAMPLE QUIET libswresample)
    
    if(AVFORMAT_FOUND AND AVCODEC_FOUND AND AVUTIL_FOUND AND SWSCALE_FOUND)
        target_include_directories(freeeffect_core PUBLIC
            ${AVFORMAT_INCLUDE_DIRS}
            ${AVCODEC_INCLUDE_DIRS}
            ${AVUTIL_INCLUDE_DIRS}
            ${SWSCALE_INCLUDE_DIRS}
        )
        target_link_libraries(freeeffect_core PUBLIC
            ${AVFORMAT_LIBRARIES}
            ${AVCODEC_LIBRARIES}
            ${AVUTIL_LIBRARIES}
            ${SWSCALE_LIBRARIES}
        )
        target_compile_definitions(freeeffect_core PRIVATE HAS_FFMPEG)
        
        if(SWRESAMPLE_FOUND)
            target_include_directories(freeeffect_core PUBLIC ${SWRESAMPLE_INCLUDE_DIRS})
            target_link_libraries(freeeffect_core PUBLIC ${SWRESAMPLE_LIBRARIES})
        endif()
    endif()
endif()

# Required: Find zlib (for PSD import compression and PNG export)
find_package(ZLIB QUIET)
if(ZLIB_FOUND)
    target_link_libraries(freeeffect_core PRIVATE ZLIB::ZLIB)
    target_compile_definitions(freeeffect_core PRIVATE HAS_ZLIB)
else()
    # Fallback: try pkg-config
    if(PkgConfig_FOUND)
        pkg_check_modules(ZLIB QUIET zlib)
        if(ZLIB_FOUND)
            target_include_directories(freeeffect_core PRIVATE ${ZLIB_INCLUDE_DIRS})
            target_link_libraries(freeeffect_core PRIVATE ${ZLIB_LIBRARIES})
            target_compile_definitions(freeeffect_core PRIVATE HAS_ZLIB)
        endif()
    endif()
endif()
