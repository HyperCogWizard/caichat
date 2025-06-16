cmake_minimum_required(VERSION 3.16)
project(caichat-opencog VERSION 0.29.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)

# Try to find Guile
pkg_check_modules(GUILE guile-3.0)
if(NOT GUILE_FOUND)
    pkg_check_modules(GUILE REQUIRED guile-2.2)
endif()

# Find other dependencies
find_package(CURL REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# Try to find OpenCog dependencies (make optional for now)
find_package(AtomSpace QUIET)
find_package(CogUtil QUIET)

if(AtomSpace_FOUND AND CogUtil_FOUND)
    set(HAVE_OPENCOG TRUE)
    message(STATUS "OpenCog found - building with full integration")
else()
    set(HAVE_OPENCOG FALSE)
    message(WARNING "OpenCog not found - building minimal version")
    # Create minimal stub definitions
    set(ATOMSPACE_INCLUDE_DIR "")
    set(ATOMSPACE_LIBRARIES "")
    set(COGUTIL_INCLUDE_DIR "")
    set(COGUTIL_LIBRARIES "")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/opencog
    ${GUILE_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${ATOMSPACE_INCLUDE_DIR}
    ${COGUTIL_INCLUDE_DIR}
)

# Compile flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GUILE_CFLAGS_OTHER} ${CURL_CFLAGS_OTHER} ${JSONCPP_CFLAGS_OTHER}")

if(HAVE_OPENCOG)
    add_definitions(-DHAVE_OPENCOG)
endif()

# Source files
set(CAICHAT_SOURCES
    opencog/caichat/LLMClient.cc
    opencog/caichat/ChatCompletion.cc
    opencog/caichat/SchemeBindings.cc
)

# Create the library
add_library(caichat-opencog SHARED ${CAICHAT_SOURCES})

# Link libraries
target_link_libraries(caichat-opencog
    ${CURL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${GUILE_LIBRARIES}
    ${ATOMSPACE_LIBRARIES}
    ${COGUTIL_LIBRARIES}
)

# Install targets
install(TARGETS caichat-opencog
    LIBRARY DESTINATION lib
)

# Install Scheme files
install(DIRECTORY opencog/scm/
    DESTINATION share/opencog/scm
    FILES_MATCHING PATTERN "*.scm"
)

# Install headers
install(DIRECTORY opencog/caichat/
    DESTINATION include/opencog/caichat
    FILES_MATCHING PATTERN "*.h"
)

# Configure and install module init file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/opencog/caichat.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/opencog/caichat.conf
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/opencog/caichat.conf
    DESTINATION lib/opencog
)

# Create example configuration
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/opencog/caichat.conf.in
    ${CMAKE_CURRENT_BINARY_DIR}/caichat-example.conf
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/caichat-example.conf
    DESTINATION share/doc/caichat-opencog
)