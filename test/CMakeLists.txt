cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project(railway)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
add_executable(railway_test runner.cpp railway_test.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(railway_test ws2_32)
endif()

find_package(Doxygen)
if(DOXYGEN_FOUND)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/..//Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(doxygen
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif(DOXYGEN_FOUND)
