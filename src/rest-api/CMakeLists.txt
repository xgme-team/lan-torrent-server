add_library(RestApiLib STATIC "")
target_include_directories(RestApiLib PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>")
target_link_libraries(RestApiLib PUBLIC
    CommonLib Libmicrohttpd)

file(GLOB SOURCE_FILES *.cpp *.hpp include/*.hpp)
target_sources(RestApiLib PRIVATE ${SOURCE_FILES})
