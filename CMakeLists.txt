cmake_minimum_required(VERSION 3.16)
project(function-ref CXX)

get_directory_property(has_parent PARENT_DIRECTORY)
set(top_level $<NOT:$<BOOL:${has_parent}>>)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(cmake/compiler_warnings.cmake)
include(cmake/static_analyzers.cmake)
include(cmake/sanitizers.cmake)

add_library(function-ref src/function_ref.cpp)

target_include_directories(
  function-ref PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(top_level AND ENABLE_TESTING)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)

  add_library(project_warnings INTERFACE)
  add_library(project_options INTERFACE)

  target_link_libraries(function-ref INTERFACE project_options project_warnings)

  include(cmake/extra_local_settings.cmake)

  set_project_warnings(project_warnings)
  enable_sanitizers(project_options)

  enable_testing()
  add_subdirectory(test)
endif()
