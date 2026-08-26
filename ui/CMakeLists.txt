# UI library - Qt6 based
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(UI_SOURCES
    mainwindow/main.cpp
    mainwindow/main_window.h
    mainwindow/main_window.cpp
    menus/menu_bar.h
    menus/menu_bar.cpp
    menus/shortcut_manager.h
    menus/shortcut_manager.cpp
    panels/project_panel.h
    panels/project_panel.cpp
    panels/composition_panel.h
    panels/composition_panel.cpp
    panels/timeline_panel.h
    panels/timeline_panel.cpp
    panels/effect_controls_panel.h
    panels/effect_controls_panel.cpp
    panels/effects_browser_panel.h
    panels/effects_browser_panel.cpp
    canvas/canvas_widget.h
    canvas/canvas_widget.cpp
    dialogs/new_composition_dialog.h
    dialogs/new_composition_dialog.cpp
    dialogs/preferences_dialog.h
    dialogs/preferences_dialog.cpp
    dialogs/render_queue_dialog.h
    dialogs/render_queue_dialog.cpp
    dialogs/missing_footage_dialog.h
    dialogs/missing_footage_dialog.cpp
    dialogs/about_dialog.h
    dialogs/about_dialog.cpp
    dialogs/startup_dialog.h
    dialogs/startup_dialog.cpp
    theme/theme.h
    theme/theme.cpp
    splash/splash_screen.h
    splash/splash_screen.cpp
    ai/ai_connector.h
    ai/ai_connector.cpp
    ai/ai_panel.h
    ai/ai_panel.cpp
    ai/ai_settings_dialog.h
    ai/ai_settings_dialog.cpp
    ai/ai_command_executor.h
    ai/ai_command_executor.cpp
    panels/timeline_canvas.h
    panels/timeline_canvas.cpp
    panels/info_panel.h
    panels/info_panel.cpp
    panels/audio_panel.h
    panels/audio_panel.cpp
    panels/preview_panel.h
    panels/preview_panel.cpp
    panels/character_panel.h
    panels/character_panel.cpp
    panels/paragraph_panel.h
    panels/paragraph_panel.cpp
    panels/brushes_panel.h
    panels/brushes_panel.cpp
    panels/paint_panel.h
    panels/paint_panel.cpp
    panels/smoother_panel.h
    panels/smoother_panel.cpp
    panels/motion_sketch_panel.h
    panels/motion_sketch_panel.cpp
    panels/wiggler_panel.h
    panels/wiggler_panel.cpp
    panels/posterize_time_panel.h
    panels/posterize_time_panel.cpp
    panels/align_panel.h
    panels/align_panel.cpp
    panels/tracker_panel.h
    panels/tracker_panel.cpp
    panels/content_aware_fill_panel.h
    panels/content_aware_fill_panel.cpp
    panels/roto_brush_panel.h
    panels/roto_brush_panel.cpp
    panels/planar_editor_panel.h
    panels/planar_editor_panel.cpp
    panels/data_driven_panel.h
    panels/data_driven_panel.cpp
    panels/essential_graphics_panel.h
    panels/essential_graphics_panel.cpp
    panels/libraries_panel.h
    panels/libraries_panel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/resources.qrc
)

add_executable(FreeEffect ${UI_SOURCES})

target_link_libraries(FreeEffect PRIVATE
    freeeffect_core
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
    Qt6::Network
)

target_include_directories(FreeEffect PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
)

# Set the app icon for macOS bundle
set_target_properties(FreeEffect PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "FreeEffect"
    MACOSX_BUNDLE_BUNDLE_VERSION "0.1.0"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1.0"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.freeeffect.app"
    MACOSX_BUNDLE_ICON_FILE "FreeEffect.icns"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/../resources/Info.plist.in"
)

# Copy icon to bundle
set(ICON_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../textures/app/FreeEffect.icns")
if(EXISTS "${ICON_PATH}")
    target_sources(FreeEffect PRIVATE "${ICON_PATH}")
    set_source_files_properties("${ICON_PATH}" PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
endif()
