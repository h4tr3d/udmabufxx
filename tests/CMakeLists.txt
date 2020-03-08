add_library(tests_main STATIC main-test.cpp)
target_link_libraries(tests_main PUBLIC Catch2::Catch2)

# we need rebuild sources with MOC enabled to avoid dependency from hardware
set(UDMABUFXX_ENABLE_MOC On)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/src-tests)

add_executable(test_executor
    smoke.cpp)
target_link_libraries(test_executor
    PUBLIC
        Catch2::Catch2 tests_main
    PRIVATE
        udmabufxx::udmabufxx_moc
    )

list(APPEND CMAKE_MODULE_PATH "${CATCH_DIR}/contrib")
message(STATUS "${CATCH_DIR}/contrib")

include(CTest)
include(Catch)
catch_discover_tests(test_executor)
