add_library(RestApiLibTest INTERFACE)
target_link_libraries(RestApiLibTest INTERFACE
    GTest::GTest
    RestApiLib)

file(GLOB SOURCE_FILES *.cpp *.hpp)
target_sources(RestApiLibTest INTERFACE ${SOURCE_FILES})
