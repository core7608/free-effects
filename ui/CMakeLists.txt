# UI library - Qt6 based
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Gui)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/../resources/resources.qrc
)

add_executable(FreeEffect ${UI_SOURCES})

target_link_libraries(FreeEffect PRIVATE
    freeeffect_core
    Qt6::Widgets
    Qt6::Core
    Qt6::Gui
)

target_include_directories(FreeEffect PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../core
)
