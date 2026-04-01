add_executable(${PROJECT_NAME}_tests test_test.cpp)

target_link_libraries(${PROJECT_NAME}_tests PRIVATE ${PROJECT_NAME}
                                                    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_tests)
