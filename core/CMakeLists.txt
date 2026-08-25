# Core library - no Qt dependency
add_library(freeeffect_core STATIC
    # Timeline data model
    timeline/types.h
    timeline/keyframe.h
    timeline/keyframe.cpp
    timeline/property_track.h
    timeline/property_track.cpp
    timeline/layer.h
    timeline/layer.cpp
    timeline/composition.h
    timeline/composition.cpp
    
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
)

target_include_directories(freeeffect_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

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
