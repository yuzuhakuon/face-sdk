FetchContent_Declare(
  doctest
  GIT_REPOSITORY https://github.com/doctest/doctest.git
  GIT_TAG v2.4.11)

FetchContent_MakeAvailable(doctest)
# ---------------------------------------------------------
# TESTS
# ---------------------------------------------------------
file(GLOB_RECURSE TEST_FILES ./*.test.cc)
list(APPEND TEST_NAMES "")
foreach(FILE_PATH ${TEST_FILES})
  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" FILE_NAME ${FILE_PATH})
  list(APPEND TEST_NAMES ${FILE_NAME})
  add_executable(${FILE_NAME})
  target_sources(${FILE_NAME} PRIVATE ${FILE_PATH})
  list(APPEND targets ${FILE_NAME})
endforeach(FILE_PATH)

foreach(_target ${targets})
  target_link_libraries(${_target} PRIVATE core_obj doctest::doctest)
  target_include_directories(${_target} PUBLIC ${PROJECT_SOURCE_DIR}/include)

  set_target_properties(
    ${_target}
    PROPERTIES ARCHIVE_OUTPUT_DIRECTORY
               "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
               LIBRARY_OUTPUT_DIRECTORY
               "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
               RUNTIME_OUTPUT_DIRECTORY
               "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}")
endforeach()
