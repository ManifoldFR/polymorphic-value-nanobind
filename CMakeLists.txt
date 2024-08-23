cmake_minimum_required(VERSION 3.20)
project(nanobind-poly-value)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel"
                                               "RelWithDebInfo")
endif()

find_package(Python 3.8 REQUIRED COMPONENTS Development.Module Interpreter)

add_subdirectory(external/robin-map)
add_subdirectory(external/nanobind)
add_subdirectory(external/value_types)

set(SOURCES src/lib.cpp)
nanobind_add_module(myext ${SOURCES})
target_link_libraries(myext PUBLIC xyz_value_types::polymorphic_cxx17)
target_link_libraries(myext PRIVATE tsl::robin_map)
