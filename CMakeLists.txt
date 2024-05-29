# cmake minimum required version
cmake_minimum_required(VERSION 3.19)

# ----------------------------------------------
# Start face sdk project
# ----------------------------------------------
project(
  face_sdk
  VERSION 0.0.1
  LANGUAGES CXX)
message(STATUS "Build FaceSDK")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(FetchContent)
include(cmake/CPM.cmake)
include(cmake/ncnn_utils.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ARTICRAFT_OUT "out")
set(CMAKE_INSTALL_PREFIX "install")

# ------------------------------------------------------------------
# Compile options
# ------------------------------------------------------------------
option(ENABLE_LOG "write to local file" ON)
option(java_REQUIRED "build with java interface" OFF)
# option(TensorRT_REQUIRED "add tensorrt support" OFF)
# option(onnxruntime_REQUIRED "add onnxruntime support" ON)

# ------------------------------------------------------------------
# Set default build to release
# ------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      "Release"
      CACHE STRING "Choose Release or Debug" FORCE)
endif()
# build type
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -O0")
  add_definitions(-DDEBUG -D_DEBUG)
  message("Debug mode:${CMAKE_C_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -O3")
  add_definitions(-DNDEBUG -DRELEASE_MODE)
  message("Release mode:${CMAKE_C_FLAGS_RELEASE}")
endif()

message("Identifying the OS...")
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  message("This is Windows.")
  if(MSVC)
    add_compile_options(/utf-8)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -DUNICODE -D_UNICODE)
  endif()
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
  message("This is Linux.")
  add_compile_options(-fPIC)
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  message("This is MacOS.")
endif()

add_definitions(-DSPDLOG_FMT_EXTERNAL_HO)
if(ENABLE_LOG)
  add_definitions(-DENABLE_LOG)
endif()

cpmaddpackage("gh:fmtlib/fmt#10.2.1")
cpmaddpackage("gh:gabime/spdlog@1.14.1")

set_ncnn_root_dir()
find_package(ncnn REQUIRED)
add_definitions(-DNCNN_INMEMORY)

if(NOT ANDROID)
  find_package(OpenCV REQUIRED)
endif()

if(ANDROID)
  if(TARGET ncnn)
    set_target_properties(ncnn PROPERTIES INTERFACE_COMPILE_OPTIONS
                                          "-fno-rtti;-fexceptions")
  endif()
endif()

file(GLOB_RECURSE SRC_NESTED src/*.cc)
add_library(core_obj OBJECT ${SRC_NESTED})
add_library(core_obj::core_obj ALIAS core_obj)
set_target_properties(core_obj PROPERTIES NO_SYSTEM_FROM_IMPORTED ON)
target_link_libraries(core_obj PUBLIC fmt::fmt-header-only)
target_link_libraries(core_obj PUBLIC spdlog::spdlog)
target_link_libraries(core_obj PUBLIC ncnn)
target_include_directories(core_obj PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

ncnnmem_generate(TARGET core_obj MODELS resources/models/scrfd
                 resources/models/mbf)

add_subdirectory(tests)

if(OpenCV_FOUND)
  add_subdirectory(samples)
endif()

# ------------------------------------------------------------------
# Compile
# ------------------------------------------------------------------
add_library(${CMAKE_PROJECT_NAME} SHARED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE core_obj)
target_include_directories(${CMAKE_PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include)

set_target_properties(
  ${CMAKE_PROJECT_NAME}
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}")

file(GLOB_RECURSE SDK_HEADER_FILES ./include/gawrs_face/sdk_interface/*.h)
install(
  TARGETS face_sdk
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES ${SDK_HEADER_FILES} DESTINATION include/gawrs_face)

if(java_REQUIRED)
  add_subdirectory(jni)
  message(STATUS "build with java interface")
endif()
