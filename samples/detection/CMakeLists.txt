set(app_name "detection")

find_package(OpenCV REQUIRED)

add_executable(${app_name} main.cc)
target_include_directories(
  ${app_name}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${app_name} PUBLIC core_obj ${OpenCV_LIBS})

set_target_properties(
  ${app_name}
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}")
