cmake_minimum_required(VERSION 3.20)
project(FreeEffect VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

enable_testing()

add_subdirectory(core)
add_subdirectory(ui)
add_subdirectory(tests)

# ──────────────────────────────────────────────────────────
# CPack Packaging Configuration
# ──────────────────────────────────────────────────────────

set(CPACK_PACKAGE_NAME "FreeEffect")
set(CPACK_PACKAGE_VENDOR "FreeEffect Project")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open-source motion graphics and compositing application")
set(CPACK_PACKAGE_DESCRIPTION "FreeEffect is an open-source Adobe After Effects alternative featuring a full motion graphics and compositing engine with Qt6 UI.")
set(CPACK_PACKAGE_CONTACT "https://github.com/core7608/free-effects")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/core7608/free-effects")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

# ── macOS: Drag-n-Drop DMG ──────────────────────────────
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
    set(CPACK_DMG_VOLUME_NAME "FreeEffect ${PROJECT_VERSION}")
    set(CPACK_DMG_FORMAT "UDBZ")
    set(CPACK_DMG_BACKGROUND_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/resources/dmg_background.png")
    set(CPACK_DMG_WINDOW_SIZE "600,400")
    set(CPACK_DMG_WINDOW_POS "200,120")
    set(CPACK_DMG_ICON_POSITIONS "[200,200],[400,200]")
    set(CPACK_DMG_HIDE_EXTENSIONS "ON")

# ── Linux: DEB + RPM ────────────────────────────────────
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # DEB
    set(CPACK_DEBIAN_PACKAGE_NAME "freeeffect")
    set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
    set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt6-base-dev (>= 6.2), libgl1-mesa-dev")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

    # RPM
    set(CPACK_RPM_PACKAGE_NAME "freeeffect")
    set(CPACK_RPM_PACKAGE_GROUP "Applications/Graphics")
    set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    set(CPACK_RPM_PACKAGE_REQUIRES "qt6-qtbase >= 6.2, mesa-libGL")
    set(CPACK_RPM_PACKAGE_AUTOREQ ON)
    set(CPACK_RPM_PACKAGE_AUTOPROV ON)

    set(CPACK_GENERATOR "DEB;RPM")

# ── Windows: NSIS ───────────────────────────────────────
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_PACKAGE_NAME "FreeEffect")
    set(CPACK_NSIS_DISPLAY_NAME "FreeEffect")
    set(CPACK_NSIS_PUBLISHER "FreeEffect Project")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://github.com/core7608/free-effects")
    set(CPACK_NSIS_HELP_LINK "https://github.com/core7608/free-effects/issues")
    set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES")
    set(CPACK_NSIS_CREATE_ICONS_EXTRA
        "CreateShortCut '$DESKTOP\\\\FreeEffect.lnk' '$INSTDIR\\\\bin\\\\FreeEffect.exe'"
    )
    set(CPACK_NSIS_DELETE_ICONS_EXTRA
        "Delete '$DESKTOP\\\\FreeEffect.lnk'"
    )
    set(CPACK_NSIS_INSTALL_DIRECTORY)
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.ico")
    set(CPACK_NSIS_ENABLE_UNINSTALL ON)

    # WiX for MSI installer (alternative)
    list(APPEND CPACK_GENERATOR "WiX")
    set(CPACK_WIX_PRODUCT_GUID "AUTO")
    set(CPACK_WIX_UPGRADE_GUID "A1B2C3D4-E5F6-7890-ABCD-EF1234567890")
    set(CPACK_WIX_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/resources/WiX/app.wxs")
endif()

include(CPack)
