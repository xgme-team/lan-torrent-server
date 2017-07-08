# - Try to find libmicrohttpd
#
# Once done this will define
#  Libmicrohttpd_FOUND - System has libmicrohttpd
#  Libmicrohttpd_INCLUDE_DIRS - The libmicrohttpd include directories
#  Libmicrohttpd_LIBRARIES - The libraries needed to use libmicrohttpd

find_package(Threads REQUIRED)
find_package(PkgConfig QUIET)

if(PKG_CONFIG_FOUND)
	pkg_check_modules(PC_LIBMICROHTTPD QUIET libmicrohttpd)
endif()

if(Libmicrohttpd_USE_STATIC_LIBS)
	set(Libmicrohttpd_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

find_path(Libmicrohttpd_INCLUDE_DIR microhttpd.h
	HINTS ${PC_LIBMICROHTTPD_INCLUDEDIR} ${PC_LIBMICROHTTPD_INCLUDE_DIRS})

find_library(Libmicrohttpd_LIBRARY NAMES microhttpd
	HINTS ${PC_LIBMICROHTTPD_LIBDIR} ${PC_LIBMICROHTTPD_LIBRARY_DIRS})

if(Libmicrohttpd_USE_STATIC_LIBS)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ${Libmicrohttpd_ORIG_CMAKE_FIND_LIBRARY_SUFFIXES})
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set Libmicrohttpd_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(Libmicrohttpd DEFAULT_MSG
	Libmicrohttpd_LIBRARY Libmicrohttpd_INCLUDE_DIR)

mark_as_advanced(Libmicrohttpd_INCLUDE_DIR Libmicrohttpd_LIBRARY)

set(Libmicrohttpd_LIBRARIES ${Libmicrohttpd_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})
set(Libmicrohttpd_INCLUDE_DIRS ${Libmicrohttpd_INCLUDE_DIR})

if (Libmicrohttpd_FOUND AND NOT TARGET Libmicrohttpd)
	add_library(Libmicrohttpd UNKNOWN IMPORTED)

	set_target_properties(Libmicrohttpd PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "C"
        IMPORTED_LOCATION "${Libmicrohttpd_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Libmicrohttpd_INCLUDE_DIRS}"
		INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${Libmicrohttpd_INCLUDE_DIRS}"
		INTERFACE_LINK_LIBRARIES "${Libmicrohttpd_LIBRARIES}"
		#INTERFACE_COMPILE_OPTIONS "${Libmicrohttpd_DEFINITIONS}"
    )
endif()
