# CMake install configuration for FreeEffect
# Used by CPack and `cmake --install`

# Main executable
install(TARGETS FreeEffect
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Core library
install(TARGETS freeeffect_core
    ARCHIVE DESTINATION lib
)

# Resources
install(DIRECTORY ${CMAKE_SOURCE_DIR}/textures/
    DESTINATION share/freeeffect/textures
    PATTERN "*.svg"
    PATTERN "*.png"
    PATTERN "*.icns"
)

# Desktop integration (Linux)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    install(FILES ${CMAKE_SOURCE_DIR}/packaging/linux/com.freeeffect.FreeEffect.desktop
        DESTINATION share/applications
        RENAME com.freeeffect.FreeEffect.desktop
    )

    install(FILES ${CMAKE_SOURCE_DIR}/textures/app/icon.svg
        DESTINATION share/icons/hicolor/scalable/apps
        RENAME com.freeeffect.FreeEffect.svg
    )

    install(FILES ${CMAKE_SOURCE_DIR}/resources/freeeffect-mime.xml
        DESTINATION share/mime/packages
        RENAME com.freeeffect.FreeEffect.xml
    )
endif()

# macOS bundle resources
if(APPLE)
    install(FILES ${CMAKE_SOURCE_DIR}/textures/app/FreeEffect.icns
        DESTINATION ${CMAKE_INSTALL_PREFIX}/FreeEffect.app/Contents/Resources
    )
endif()
