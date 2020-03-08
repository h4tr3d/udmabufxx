add_library(tests_main STATIC main-test.cpp)
target_link_libraries(tests_main PUBLIC Catch2::Catch2)

add_executable(test_executor
    smoke.cpp)
target_link_libraries(test_executor
    PUBLIC
        Catch2::Catch2 tests_main
    #PRIVATE
    #    AvCpp::AvCpp
    )

list(APPEND CMAKE_MODULE_PATH "${CATCH_DIR}/contrib")
message(STATUS "${CATCH_DIR}/contrib")

include(CTest)
include(Catch)
catch_discover_tests(test_executor)
