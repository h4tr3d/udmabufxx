cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED PROJECT_NAME)
    set(UDMABUFXX_NOT_SUBPROJECT ON)
endif()

project(udmabufxx)

# Options
set(LIB_INSTALL_DIR lib        CACHE STRING  "Install location of libraries")
set(BIN_INSTALL_DIR bin        CACHE STRING  "Install location of executables")
set(INC_INSTALL_DIR include    CACHE STRING  "Intall location of headers")
set(UDMABUFXX_ENABLE_STATIC           Off  CACHE BOOL    "Enable static library")
set(UDMABUFXX_REAL_PAGE_ALLOCATION_WA On   CACHE BOOL    "WA for issue #38 of udmabuf")
set(UDMABUFXX_ENABLE_MOC              Off  CACHE BOOL    "Enable mocking")

# where to look first for cmake modules, before ${CMAKE_ROOT}/modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

# C++ standard: 98, 11, 14, 17
#   to use per-terget: https://cmake.org/cmake/help/v3.8/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF) # turn on/off GNU or some other extensions

# C standard: 90, 99, 11
#   to use per-terget: https://cmake.org/cmake/help/v3.8/prop_tgt/C_STANDARD.html
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
#set(CMAKE_C_EXTENSIONS OFF) # turn on/off GNU or some other extensions

find_package(Threads REQUIRED)
find_package(PkgConfig)
find_package(fmt REQUIRED)

# Warnings
set (UDMABUFXX_WARNING_OPTIONS
     $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
       -Wall -Wextra>
     $<$<CXX_COMPILER_ID:MSVC>:
       /W4>)

# sources
add_subdirectory(src)

# testing
set(CATCH_DIR ${CMAKE_CURRENT_LIST_DIR}/externals/catch2)
if (EXISTS ${CATCH_DIR}/CMakeLists.txt)
    enable_testing()
    set(CATCH_BUILD_TESTING Off CACHE STRING "")
    add_subdirectory(${CATCH_DIR})
    add_subdirectory(tests)
else()
    message(STATUS "Catch2 not found, testing skipped")
endif()
