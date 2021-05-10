
# Now simply link against gtest or gtest_main as needed. Eg
add_executable(wsclient test_wsclient.cpp)
target_link_libraries(wsclient gtest_main)
add_test(NAME wsclient_test COMMAND wsclient)




# find_package(GTest CONFIG REQUIRED)

