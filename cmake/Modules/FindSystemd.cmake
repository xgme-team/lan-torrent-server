# - Try to find systemd library
#
# Once done this will define
#  Systemd_FOUND - System has systemd
#  Systemd_INCLUDE_DIRS - The systemd include directories
#  Systemd_LIBRARIES - The libraries needed to use systemd
#  Systemd_DEFINITIONS - Compiler switches required for using systemd


find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_SYSTEMD QUIET libsystemd)
endif()

# Probably, there is no static library of systemd
#if(Systemd_USE_STATIC_LIBS)
#    set(Systemd_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
#    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
#endif()

if(PC_SYSTEMD_FOUND)
    foreach(item ${PC_SYSTEMD_CFLAGS})
        if("${item}" MATCHES "-D.*")
            list(APPEND Systemd_DEFINITIONS ${item})
        endif()
    endforeach()
else()
    if(Systemd_CUSTOM_DEFINITIONS)
        set(Systemd_DEFINITIONS ${Systemd_CUSTOM_DEFINITIONS})
    endif()
endif()

find_path(Systemd_INCLUDE_DIR systemd/sd-login.h
    HINTS ${PC_SYSTEMD_INCLUDEDIR} ${PC_SYSTEMD_INCLUDE_DIRS})

find_library(Systemd_LIBRARY NAMES systemd
    HINTS ${PC_SYSTEMD_LIBDIR} ${PC_SYSTEMD_LIBRARY_DIRS})

#if(Systemd_USE_STATIC_LIBS)
#    set(CMAKE_FIND_LIBRARY_SUFFIXES ${Systemd_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
#endif()

set(Systemd_LIBRARIES ${Systemd_LIBRARY})
set(Systemd_INCLUDE_DIRS ${Systemd_INCLUDE_DIR})


include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set Systemd_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Systemd DEFAULT_MSG
    Systemd_LIBRARY Systemd_INCLUDE_DIR)
mark_as_advanced(Systemd_INCLUDE_DIR Systemd_LIBRARY)

if (Systemd_FOUND AND NOT TARGET Systemd::Systemd)
    add_library(Systemd::Systemd UNKNOWN IMPORTED)

    set_target_properties(Systemd::Systemd PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${Systemd_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Systemd_INCLUDE_DIRS}"
        INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${Systemd_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${Systemd_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${Systemd_DEFINITIONS}"
    )
endif()
