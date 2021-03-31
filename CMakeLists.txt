cmake_minimum_required(VERSION 3.13...3.19)

project(
  wampev
  VERSION 0.1
  LANGUAGES CXX)

find_package(Boost REQUIRED COMPONENTS filesystem coroutine)
find_package(Threads)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)


set(CMAKE_CXX_CLANG_TIDY
  clang-tidy;
  -header-filter=.;
  -checks=*;
  -warnings-as-errors=*;)


add_executable(wampev src/main.cpp)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(wampev PRIVATE ${Boost_LIBRARIES} Threads::Threads)
target_compile_features(wampev PUBLIC cxx_std_17)

include(ExternalProject)
find_package(Git REQUIRED)


# https://github.com/ainfosec/ci_helloworld

# ----------------------------
# CLANG TIDY
# ----------------------------



#list(APPEND RUN_CLANG_TIDY_BIN_ARGS
#    -clang-tidy-binary ${CLANG_TIDY_BIN}
#    -header-filter=.*
#    -checks=clan*,cert*,misc*,perf*,cppc*,read*,mode*,-cert-err58-cpp,-misc-noexcept-move-constructor
#)

#add_custom_target(
#    tidy
#    COMMAND ${RUN_CLANG_TIDY_BIN} ${RUN_CLANG_TIDY_BIN_ARGS}
#    COMMENT "running clang tidy"
#)



# ----------------------------
# CPPCHECK
# ----------------------------

list(APPEND CPPCHECK_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
)

ExternalProject_Add(
    cppcheck
    GIT_REPOSITORY      https://github.com/danmar/cppcheck.git
    GIT_TAG             2.3
    GIT_SHALLOW         1
    CMAKE_ARGS          ${CPPCHECK_CMAKE_ARGS}
    PREFIX              ${CMAKE_BINARY_DIR}/external/cppcheck/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/cppcheck/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/cppcheck/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/cppcheck/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/src
    BINARY_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/build
)

list(APPEND CPPCHECK_ARGS
    --enable=warning,style,performance,portability,unusedFunction
    --std=c++11
    --verbose
    --error-exitcode=1
    --language=c++
    -DMAIN=main
    -I ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/*.h
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/test/*.cpp
)

add_custom_target(
    check
    COMMAND ${CMAKE_BINARY_DIR}/bin/cppcheck ${CPPCHECK_ARGS}
    COMMENT "running cppcheck"
)




# ----------------------------
# COVERAGE
# ----------------------------

if(ENABLE_COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()




# ----------------------------
# Sanitizer
# ----------------------------

if(ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=leak")
endif()

if(ENABLE_USAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()

if(ENABLE_TSAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

