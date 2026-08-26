# Tests for core library
# Use a simple test framework - GoogleTest or Catch2 can be added later
# For MVP, we use a standalone test runner

add_executable(freeeffect_tests
    test_timeline.cpp
    test_commands.cpp
    test_project.cpp
    test_io.cpp
    test_effects.cpp
    main_test.cpp
)

target_link_libraries(freeeffect_tests PRIVATE freeeffect_core)
target_include_directories(freeeffect_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Force linker to include ALL symbols from static lib (needed for EffectRegistrar static init)
if(APPLE)
    target_link_options(freeeffect_tests PRIVATE -Wl,-force_load,$<TARGET_FILE:freeeffect_core>)
elseif(UNIX)
    target_link_options(freeeffect_tests PRIVATE -Wl,--whole-archive,$<TARGET_FILE:freeeffect_core>,--no-whole-archive)
endif()

# Add CTest
add_test(NAME FreeEffectCoreTests COMMAND freeeffect_tests)
