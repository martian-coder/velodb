add_executable(basic_test basic_test.cpp)
target_link_libraries(basic_test PRIVATE velodb)
add_test(NAME basic_test COMMAND basic_test)

add_executable(stress_test stress_test.cpp)
target_link_libraries(stress_test PRIVATE velodb)
add_test(NAME stress_test COMMAND stress_test)
