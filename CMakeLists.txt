cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project("LAN Torrent Server" VERSION 0.1.0 LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules/")

## Load packages for CMake and find dependencies
include(GNUInstallDirs)
find_package(Doxygen)
find_package(GTest)
find_package(Boost 1.58 REQUIRED COMPONENTS program_options)
find_package(Libmicrohttpd REQUIRED)
find_package(LibtorrentRasterbar 1.1 REQUIRED)
find_package(Threads REQUIRED)

## Build configuration
set(XLTS_EXECUTABLE "lan-torrent-server"                           CACHE STRING
    "Name of the resulting executable."                                       )
set(XLTS_TESTS_EXE  "lan-torrent-server-test"                      CACHE STRING
    "Name of the resulting test executable."                                  )
set(XLTS_SERVICE    "lan-torrent-server"                           CACHE STRING
    "Service name when using systemd."                                        )

option(XLTS_DOC_TARGET
    "Add a target to create the HTML based documentation (requires Doxygen)"
    ${DOXYGEN_FOUND})

option(XLTS_TESTS_BUILD
    "Build unit tests (requires GTest)"
    ${GTEST_FOUND})

option(XLTS_USE_SYSTEMD
    "Use logging and notify service manager of systemd. (requires Systemd)"
    ON)

set(XLTS_DEFAULT_INIFILE       ""                                CACHE FILEPATH
    "Default path to configuration file"                                      )
set(XLTS_DEFAULT_TORRENTDIR    "downloads/.torrents"                 CACHE PATH
    "Default directory to save torrent files"                                 )
set(XLTS_DEFAULT_RESUMEDATADIR "downloads/.torrents"                 CACHE PATH
    "Default directory to save resume data"                                   )
set(XLTS_DEFAULT_DOWNLOADDIR   "downloads"                           CACHE PATH
    "Default directory to save downloaded data"                               )

## Use Systemd when desired
if (XLTS_USE_SYSTEMD)
    find_package(Systemd REQUIRED)
    add_definitions(-DXLTS_USE_SYSTEMD)
    link_libraries(Systemd::Systemd)
endif()

## Create header with build information
configure_file(
    "${PROJECT_SOURCE_DIR}/buildconf.h.in"
    "${PROJECT_BINARY_DIR}/include/buildconf.h" ESCAPE_QUOTES)
include_directories("${PROJECT_BINARY_DIR}/include")

## Add rule for Doxygen
if (XLTS_DOC_TARGET)
    if (NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen not found")
    endif()

    configure_file(
        "${PROJECT_SOURCE_DIR}/Doxyfile.in"
        "${PROJECT_BINARY_DIR}/Doxyfile" ESCAPE_QUOTES @ONLY)
    add_custom_target(doc
        "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/Doxyfile"
        WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
        COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

## Use C++14 per default
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED on)
## Enable thread support
link_libraries(Threads::Threads)

## Add components
add_subdirectory("src")

if (XLTS_TESTS_BUILD)
    enable_testing()
    add_subdirectory("test")
endif()

## Add installation rule for Systemd service file and configuration
if (XLTS_USE_SYSTEMD)
    configure_file(
        "${PROJECT_SOURCE_DIR}/lan-torrent-server.ini.in"
        "${PROJECT_BINARY_DIR}/${XLTS_SERVICE}.ini" @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/${XLTS_SERVICE}.ini"
        DESTINATION "${CMAKE_INSTALL_SYSCONFDIR}")

    configure_file(
        "${PROJECT_SOURCE_DIR}/lan-torrent-server.service.in"
        "${PROJECT_BINARY_DIR}/${XLTS_SERVICE}.service" ESCAPE_QUOTES @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/${XLTS_SERVICE}.service"
        DESTINATION "lib/systemd/system") # Works ´with prefix / and /usr only

    install(CODE "message(
        \"You may have to run `systemctl daemon-reload` before using Systemd\"
        \"\nto control the service '${XLTS_SERVICE}'.\")")
endif()
