cmake_minimum_required(VERSION 3.20)
project(myexecution LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(myexecution
  src/execution.cpp
)
target_include_directories(myexecution
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
)
target_compile_features(myexecution PUBLIC cxx_std_23)

add_executable(exec_examples examples/quickstart.cpp)
target_link_libraries(exec_examples PRIVATE myexecution)

# enable_testing()
# add_subdirectory(tests)
