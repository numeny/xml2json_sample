# Definitions required for building
cmake_minimum_required(VERSION 3.12.0)

# Try C++14, then fall back to C++11 and C++98.  Used for feature tests
# for optional features.
set(CMAKE_CXX_STANDARD 17)

project(xml2json)

set(BUILD_SHARED_LIBS FALSE)

aux_source_directory(src/MemParse ALL_SRC)

# Do not include symbols that is not used
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdata-sections")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ASSERTIONS=1")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wunused-command-line-argument")
# for native code can catch exception
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")

macro(add_xml2json_dependency name)
  target_link_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR}/lib)
  # using lib xerces-c-3.2.4
  if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    target_link_libraries(${name} xerces-c-static-em)
  else()
    target_link_libraries(${name} xerces-c-static-linux)
  endif()
  target_include_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR}/src/MemParse)
  target_include_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR}/src/include)
endmacro()

set(TestProgram "${PROJECT_NAME}_bin")

add_executable(${TestProgram} ${ALL_SRC} test/Main.cpp test/PreAction.cpp)
add_xml2json_dependency(${TestProgram})

add_library(${PROJECT_NAME} ${ALL_SRC})
add_xml2json_dependency(${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
set_target_properties(${TestProgram} PROPERTIES LINK_FLAGS "-pg")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-pg")

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
  set_target_properties(${TestProgram} PROPERTIES LINK_FLAGS "--bind -sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sFORCE_FILESYSTEM=1 -sEXIT_RUNTIME=1")
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "--bind -sWASM=1 -sALLOW_MEMORY_GROWTH=1 -sFORCE_FILESYSTEM=1 -sEXIT_RUNTIME=1")
  target_compile_definitions(${TestProgram} PRIVATE EMSCRIPTEN _EMSCRIPTEN)
  target_compile_definitions(${PROJECT_NAME} PRIVATE EMSCRIPTEN _EMSCRIPTEN)
endif ()
