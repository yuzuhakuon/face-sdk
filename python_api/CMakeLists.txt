set(app_name "gawrs_face_py")

find_package(Python3 COMPONENTS Interpreter Development)
if(Python3_FOUND)
  execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pybind11 --cmakedir
    OUTPUT_VARIABLE pybind11_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "Python3 found: ${Python3_EXECUTABLE}")
  message(STATUS "pybind11 found: ${pybind11_DIR}")
else()
  message(FATAL_ERROR "Python3 not found")
endif()

find_package(pybind11 CONFIG)

pybind11_add_module(${app_name} main.cc)
target_include_directories(${app_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${app_name} PUBLIC core_obj)
set_target_properties(
  ${app_name}
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}")
