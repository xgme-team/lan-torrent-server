add_executable(TestApp "")
set_target_properties(TestApp PROPERTIES
    OUTPUT_NAME "${XLTS_TESTS_EXE}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
target_link_libraries(TestApp PRIVATE
    GTest::Main
    CommonLibTest)
add_test(NAME "AllTests" COMMAND TestApp)

add_subdirectory("common")
