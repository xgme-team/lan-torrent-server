include(GoogleTest)

add_executable(TestApp "")
set_target_properties(TestApp PROPERTIES
    OUTPUT_NAME "${XLTS_TESTS_EXE}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
target_link_libraries(TestApp PRIVATE
    GTest::Main
    CommonLibTest
    RestApiLibTest)
gtest_discover_tests(TestApp)

add_subdirectory("common")
add_subdirectory("rest-api")
