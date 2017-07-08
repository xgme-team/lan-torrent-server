add_library(CommonLibTest INTERFACE)
target_link_libraries(CommonLibTest INTERFACE
	GTest::GTest
	CommonLib)

file(GLOB SOURCE_FILES *.cpp *.hpp)
target_sources(CommonLibTest INTERFACE ${SOURCE_FILES})
