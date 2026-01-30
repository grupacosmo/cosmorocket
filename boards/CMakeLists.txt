# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

find_package(Git QUIET)
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                RESULT_VARIABLE GIT_SUBMODULE_RESULT)

set(CMAKE_CXX_STANDARD 23)

set(IDF_TOOLCHAIN clang)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)
project(boards)
