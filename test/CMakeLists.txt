
# Now simply link against gtest or gtest_main as needed. Eg
#add_executable(wsclient test_wsclient.cpp)
#target_link_libraries(wsclient gtest_main)
#add_test(NAME wsclient_test COMMAND wsclient)




# find_package(GTest CONFIG REQUIRED)

# 


# https://cmake.org/cmake/help/latest/command/cmake_parse_arguments.html
#macro(my_install)
#    set(options OPTIONAL FAST)
#    set(oneValueArgs DESTINATION RENAME)
#    set(multiValueArgs TARGETS CONFIGURATIONS)
#    cmake_parse_arguments(MY_INSTALL "${options}" "${oneValueArgs}"
#                          "${multiValueArgs}" ${ARGN} )
#
#
#
# my_install(TARGETS foo bar DESTINATION bin OPTIONAL blub CONFIGURATIONS)

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

macro(add_gtest)
    set(test_name "test_${ARGV0}")
    set(oneValueArgs TESTNAME)
    set(multiValueArgs SOURCES LIBS)
    cmake_parse_arguments(${test_name} "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN} )

    # print_all_variables()
    set(sources  "${test_name}_SOURCES")
    set(libs  "${test_name}_LIBS")
    
    # message(STATUS "print variable " ${test_name})
    # message(STATUS "print variable " "${${sources}}")

    add_executable(${test_name} "${${sources}}")
    target_link_libraries(${test_name} gtest_main ${${libs}})
    add_test(NAME ${test_name} COMMAND ${test_name})
endmacro()


add_gtest(wsclient SOURCES test_wsclient.cpp)