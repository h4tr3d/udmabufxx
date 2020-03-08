cmake_minimum_required(VERSION 3.14)

if(NOT DEFINED PROJECT_NAME)
    set(UDMABUFXX_NOT_SUBPROJECT ON)
endif()

project(udmabufxx)

# Options
set(LIB_INSTALL_DIR lib        CACHE STRING  "Install location of libraries")
set(BIN_INSTALL_DIR bin        CACHE STRING  "Install location of executables")
set(INC_INSTALL_DIR include    CACHE STRING  "Intall location of headers")
set(ENABLE_STATIC   Off        CACHE BOOL    "Enable static library")
set(REAL_PAGE_ALLOCATION_WA On CACHE BOOL    "WA for issue #38 of udmabuf")

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

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
      string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
endif()

set(${PROJECT_NAME}_DEFS "")
if (REAL_PAGE_ALLOCATION_WA)
    list(APPEND ${PROJECT_NAME}_DEFS REAL_PAGE_ALLOCATION_WA=1)
endif()

set(SRC_LIST
    udmabuf.h
    udmabuf.cpp)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_compile_definitions(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_DEFS})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if (ENABLE_STATIC)
    add_library(${PROJECT_NAME}_static STATIC ${SRC_LIST})
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME}_static Threads::Threads)
    target_compile_definitions(${PROJECT_NAME}_static PRIVATE ${${PROJECT_NAME}_DEFS$})
    add_library(${PROJECT_NAME}::${PROJECT_NAME}_static ALIAS ${PROJECT_NAME}_static)
endif()

#
# installation
#
if (UDMABUFXX_NOT_SUBPROJECT)

    install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
        RUNTIME DESTINATION ${BIN_INSTALL_DIR})

    if (ENABLE_STATIC)
        install(TARGETS ${PROJECT_NAME}_static
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}
            ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
            RUNTIME DESTINATION ${BIN_INSTALL_DIR})
    endif()

    install(FILES ${CMAKE_SOURCE_DIR}/udmabuf.h
            DESTINATION ${INC_INSTALL_DIR}/udmabufxx)

    set(UdmabufxxConfigPackageLocation "${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME}")
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/UdmabufxxConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/UdmabufxxConfig.cmake"
        @ONLY
    )

    install(FILES
                "${CMAKE_CURRENT_BINARY_DIR}/UdmabufxxConfig.cmake"
            DESTINATION
                "${UdmabufxxConfigPackageLocation}"
            COMPONENT
                Devel)

    if (PKG_CONFIG_FOUND)
        # convert lists of link libraries into -lstdc++ -lm and etc
        string(REPLACE " " ";" CXX_STD_LIBS "${CMAKE_CXX_STANDARD_LIBRARIES}")
        string(REPLACE " " ";" C_STD_LIBS "${CMAKE_C_STANDARD_LIBRARIES}")
        set(TOTAL_LIBS ${CMAKE_IMPLICIT_LINK_LIBRARIES} ${CXX_STD_LIBS} ${C_STD_LIBS} ${PLATFORM_LIBS})
        foreach (LIB ${TOTAL_LIBS})
            if (IS_ABSOLUTE ${LIB} AND EXISTS ${LIB})
                string(REPLACE
                    "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}"
                    "\${libdir}"
                    LIB_OUT
                    ${LIB})
                list(APPEND PLIBLIST "${LIB_OUT}")
            else()
                string(SUBSTRING ${LIB} 0 2 LIB_BEGIN)
                if (LIB_BEGIN STREQUAL "-l")
                    list(APPEND PLIBLIST "${LIB}")
                else()
                    list(APPEND PLIBLIST "-l${LIB}")
                endif()
            endif()
        endforeach()

        # blacklink of the libraries that should not be in Libs.private
        if (PLIBLIST)
            list(REMOVE_ITEM PLIBLIST "-lc" "-lpthread" "-pthread" "-l-pthread")
            list(REMOVE_DUPLICATES PLIBLIST)
            string(REPLACE ";" " " PRIVATE_LIBS "${PLIBLIST}")
        else()
            set(PRIVATE_LIBS "")
        endif()

        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/udmabufxx.pc.in"
            "${CMAKE_CURRENT_BINARY_DIR}/udmabufxx.pc"
            @ONLY
            )

        install(FILES "${CMAKE_CURRENT_BINARY_DIR}/udmabufxx.pc"
            DESTINATION "${LIB_INSTALL_DIR}/pkgconfig")
    endif()

endif()
