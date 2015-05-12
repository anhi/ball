# Try to find the OSPRay ray tracing libraries
# Once done, this will define
#
#  OSPRay_FOUND - system has OSPRay
#  OSPRay_INCLUDE_DIRS - the OSPRay include directories
#  OSPRay_LIBRARIES - link these to use OSPRay

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(OSPRay_PKGCONF OSPRay)

# Include dir
find_path(OSPRay_INCLUDE_DIR
  NAMES ospray/ospray.h
	PATHS ${OSPRay_PKGCONF_INCLUDE_DIRS}
)

find_path(OSPRay_Embree_INCLUDE_DIR
	NAMES ospray/embree/common/math/vec2.h
	PATHS ${OSPRay_PKGCONF_INCLUDE_DIRS}
)

find_path(OSPRay_Embree_COMMON_INCLUDE_DIR
	NAMES common/sys/platform.h
	PATHS ${OSPRay_PKGCONF_INCLUDE_DIRS}
)


# Finally the libraries themselves
find_library(OSPRay_LIBRARY
  NAMES ospray
	PATHS ${OSPRay_PKGCONF_LIBRARY_DIRS}
)

find_library(OSPRay_Embree_LIBRARY
  NAMES ospray_embree
	PATHS ${OSPRay_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
set(OSPRay_PROCESS_INCLUDES OSPRay_INCLUDE_DIR OSPRay_Embree_INCLUDE_DIR OSPRay_Embree_COMMON_INCLUDE_DIR)
set(OSPRay_PROCESS_LIBS OSPRay_LIBRARY OSPRay_Embree_LIBRARY)
libfind_process(OSPRay)
