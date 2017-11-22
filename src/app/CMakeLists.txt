add_executable(App "")
set_target_properties(App PROPERTIES
    OUTPUT_NAME "${XLTS_EXECUTABLE}"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
target_link_libraries(App PRIVATE
    CommonLib)

file(GLOB SOURCE_FILES *.cpp *.hpp)
target_sources(App PRIVATE ${SOURCE_FILES})

install(TARGETS App
    DESTINATION "${CMAKE_INSTALL_BINDIR}")
