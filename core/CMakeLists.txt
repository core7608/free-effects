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
    
    # Text Animation
    timeline/text_animator.h
    
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
    
    # Asset management
    project/asset_reference.h
    project/asset_reference.cpp
    project/project_state.h
    project/project_state.cpp
    project/missing_footage_handler.h
    project/missing_footage_handler.cpp
    
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
    
    # Audio
    audio/audio_decoder.h
    audio/audio_decoder.cpp
    audio/waveform_generator.h
    audio/waveform_generator.cpp
    
    # I/O
    io/importer.h
    io/importer.cpp
    io/exporter.h
    io/exporter.cpp
    io/project_file.h
    io/project_file.cpp
    
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
    
    # Keying Effects
    effects/keying/color_key_effect.h
    effects/keying/color_key_effect.cpp
    effects/keying/linear_color_key_effect.h
    effects/keying/linear_color_key_effect.cpp
    effects/keying/color_range_effect.h
    effects/keying/color_range_effect.cpp
    effects/keying/difference_matte_effect.h
    effects/keying/difference_matte_effect.cpp
    
    # Perspective Effects
    effects/perspective/cc_cylinder_effect.h
    effects/perspective/cc_cylinder_effect.cpp
    effects/perspective/bevel_edges_effect.h
    effects/perspective/bevel_edges_effect.cpp
    effects/perspective/perspective_drop_shadow_effect.h
    effects/perspective/perspective_drop_shadow_effect.cpp
    
    # Utility Effects
    effects/utility/transform_effect.h
    effects/utility/transform_effect.cpp
    effects/utility/tile_effect.h
    effects/utility/tile_effect.cpp
    effects/utility/time_displacement_effect.h
    effects/utility/time_displacement_effect.cpp
    effects/utility/posterize_time_effect.h
    effects/utility/posterize_time_effect.cpp
    
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
    
    # Simulation Effects
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
    
    # Transition Effects
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
    
    # Additional Color Effects
    effects/color/selective_color_effect.h
    effects/color/selective_color_effect.cpp
    effects/color/shadow_highlight_effect.h
    effects/color/shadow_highlight_effect.cpp
    effects/color/channel_mixer_effect.h
    effects/color/channel_mixer_effect.cpp
    
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
    
    # Third-Party Plugin Effect Equivalents
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
    
    # Expression Engine
    expressions/expression_engine.h
    expressions/expression_engine.cpp
    
    # Expression Library
    expressions/expression_library.h
    expressions/expression_library.cpp
    
    # Motion Tracking
    tracking/motion_tracker.h
    tracking/motion_tracker.cpp
    tracking/rotoscoping.h
    tracking/rotoscoping.cpp
    
    # Puppet Tool & Data-Driven Animation
    animation/puppet_tool.h
    animation/puppet_tool.cpp
    animation/data_driven.h
    animation/data_driven.cpp
    animation/graph_editor.h
    animation/graph_editor.cpp
    
    # Preferences System
    preferences/preferences.h
    preferences/preferences.cpp
    
    # Color Management
    color/color_management.h
    color/color_management.cpp
)

target_include_directories(freeeffect_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(freeeffect_core PRIVATE FREEEFFECT_BUILTIN_PLUGIN)

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
        target_include_directories(freeeffect_core PRIVATE
            ${AVFORMAT_INCLUDE_DIRS}
            ${AVCODEC_INCLUDE_DIRS}
            ${AVUTIL_INCLUDE_DIRS}
            ${SWSCALE_INCLUDE_DIRS}
        )
        target_link_libraries(freeeffect_core PRIVATE
            ${AVFORMAT_LIBRARIES}
            ${AVCODEC_LIBRARIES}
            ${AVUTIL_LIBRARIES}
            ${SWSCALE_LIBRARIES}
        )
        target_compile_definitions(freeeffect_core PRIVATE HAS_FFMPEG)
        
        if(SWRESAMPLE_FOUND)
            target_include_directories(freeeffect_core PRIVATE ${SWRESAMPLE_INCLUDE_DIRS})
            target_link_libraries(freeeffect_core PRIVATE ${SWRESAMPLE_LIBRARIES})
        endif()
    endif()
endif()
