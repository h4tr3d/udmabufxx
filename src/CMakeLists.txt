set(${PROJECT_NAME}_DEFS "")
if (UDMABUFXX_REAL_PAGE_ALLOCATION_WA)
    list (APPEND ${PROJECT_NAME}_DEFS REAL_PAGE_ALLOCATION_WA=1)
endif()

if (UDMABUFXX_ENABLE_MOC)
    list (APPEND ${PROJECT_NAME}_DEFS TESTS_MOC=1)
    set (MOC_SUFFIX "_moc")
    set (UDMABUFXX_ENABLE_STATIC Off)
    message(STATUS "MOC is enabled")
endif()

set(SRC_LIST
    udmabuf.h
    udmabuf.cpp)

set(NAMESPACE_NAME ${PROJECT_NAME})
set(TARGET_NAME ${PROJECT_NAME}${MOC_SUFFIX})

message(STATUS "${NAMESPACE_NAME} ${TARGET_NAME}")

set(UDMABUFXX_TARGETS ${TARGET_NAME})
if (UDMABUFXX_ENABLE_STATIC)
    list(APPEND UDMABUFXX_TARGETS ${PROJECT_NAME}-static)
endif()

# Common part for all library targets
foreach(TARGET ${UDMABUFXX_TARGETS})
    set(TYPE SHARED)
    if (${TARGET} MATCHES "-static$")
        set(TYPE STATIC)
    endif()

    add_library(${TARGET} ${TYPE} ${SRC_LIST})
    target_compile_options(${TARGET} PRIVATE ${UDMABUFXX_WARNING_OPTIONS})
    target_link_libraries(${TARGET} PRIVATE Threads::Threads fmt::fmt-header-only)
    target_compile_definitions(${TARGET} PRIVATE ${${PROJECT_NAME}_DEFS})
    target_include_directories(${TARGET}
        PUBLIC
          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
          $<INSTALL_INTERFACE:include>
        PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR})
    add_library(${NAMESPACE_NAME}::${TARGET} ALIAS ${TARGET})
endforeach()

# Target-specific
# Shared:
if (TARGET ${TARGET_NAME})
    set_target_properties(${TARGET_NAME}
        PROPERTIES
          PUBLIC_HEADER udmabuf.h)
endif()
# Static
if (TARGET ${TARGET_NAME}-static)
    set_target_properties(${TARGET_NAME}-static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

# skip installation for mocked version
if (UDMABUFXX_ENABLE_MOC)
    return()
endif()

#
# installation
#
if (UDMABUFXX_NOT_SUBPROJECT)

    install(TARGETS ${UDMABUFXX_TARGETS} EXPORT UdmabufxxTargets
        LIBRARY  DESTINATION ${LIB_INSTALL_DIR}
        ARCHIVE  DESTINATION ${LIB_INSTALL_DIR}
        RUNTIME  DESTINATION ${BIN_INSTALL_DIR}
        INCLUDES DESTINATION include)

    install(EXPORT UdmabufxxTargets
        FILE udmabufxx-targets.cmake
        NAMESPACE udmabufxx::
        DESTINATION lib/cmake/udmabufxx)

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
      "Config.cmake.in"
      "udmabufxx-config.cmake"
      INSTALL_DESTINATION ${LIB_INSTALL_DIR}/cmake/udmabufxx
#      PATH_VARS
#        CMAKE_INSTALL_LIBDIR
      )
    write_basic_package_version_file("udmabufxx-config-version.cmake"
        VERSION 0.1
        COMPATIBILITY SameMajorVersion)
    install(
        FILES
          "${CMAKE_CURRENT_BINARY_DIR}/udmabufxx-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/udmabufxx-config-version.cmake"
        DESTINATION
          "${CMAKE_INSTALL_PREFIX}/${LIB_INSTALL_DIR}/cmake/udmabufxx"
      )

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
