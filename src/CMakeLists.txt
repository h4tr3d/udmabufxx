set(${PROJECT_NAME}_DEFS "")
if (UDMABUFXX_REAL_PAGE_ALLOCATION_WA)
    list (APPEND ${PROJECT_NAME}_DEFS REAL_PAGE_ALLOCATION_WA=1)
endif()

if (UDMABUFXX_ENABLE_MOC)
    list (APPEND ${PROJECT_NAME}_DEFS TESTS_MOC=1)
    set (MOC_SUFFIX "_moc")
    set (ENABLE_STATIC Off)
    message(STATUS "MOC is enabled")
endif()

set(SRC_LIST
    udmabuf.h
    udmabuf.cpp)

set(NAMESPACE_NAME ${PROJECT_NAME})
set(TARGET_NAME ${PROJECT_NAME}${MOC_SUFFIX})

message(STATUS "${NAMESPACE_NAME} ${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED ${SRC_LIST})
target_link_libraries(${TARGET_NAME} Threads::Threads fmt::fmt-header-only)
target_compile_definitions(${TARGET_NAME} PUBLIC ${${PROJECT_NAME}_DEFS})
set_target_properties(${TARGET_NAME} PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")
add_library(${NAMESPACE_NAME}::${TARGET_NAME} ALIAS ${TARGET_NAME})

if (ENABLE_STATIC)
    add_library(${PROJECT_NAME}_static STATIC ${SRC_LIST})
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME}_static Threads::Threads fmt::fmt-header-only)
    target_compile_definitions(${PROJECT_NAME}_static PRIVATE ${${PROJECT_NAME}_DEFS$})
    add_library(${PROJECT_NAME}::${PROJECT_NAME}_static ALIAS ${PROJECT_NAME}_static)
endif()

# skip installation for mocked version
if (UDMABUFXX_ENABLE_MOC)
    return()
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

    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/udmabuf.h
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
