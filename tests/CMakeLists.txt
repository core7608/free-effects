# Tests for core library
# Use a simple test framework - GoogleTest or Catch2 can be added later
# For MVP, we use a standalone test runner

add_executable(freeeffect_tests
    test_timeline.cpp
    test_commands.cpp
    test_project.cpp
    test_io.cpp
    main_test.cpp
)

target_link_libraries(freeeffect_tests PRIVATE freeeffect_core)
target_include_directories(freeeffect_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Add CTest
add_test(NAME FreeEffectCoreTests COMMAND freeeffect_tests)
