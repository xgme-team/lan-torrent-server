add_library(CommonLib STATIC "")
target_include_directories(CommonLib PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include>")
target_link_libraries(CommonLib PUBLIC
    Boost::boost Boost::log Boost::program_options)

file(GLOB SOURCE_FILES *.cpp *.hpp include/*.hpp)
target_sources(CommonLib PRIVATE ${SOURCE_FILES})
