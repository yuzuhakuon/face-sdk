file(GLOB_RECURSE JNI_SRC_NESTED ./*.cc)

set(java_face_sdk gawrs_face_jsdk)
add_library(${java_face_sdk} SHARED ${JNI_SRC_NESTED})
target_include_directories(
  ${java_face_sdk} PUBLIC ${PROJECT_SOURCE_DIR}/jni
                          ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${java_face_sdk} PUBLIC core_obj)

if(NOT ANDROID)
  set(JAVA_AWT_LIBRARY NotNeeded)
  set(JAVA_JVM_LIBRARY NotNeeded)
  set(JAVA_AWT_INCLUDE_PATH NotNeeded)
  find_package(JNI REQUIRED)
  target_include_directories(${java_face_sdk} PUBLIC ${JAVA_INCLUDE_PATH}
                                                     ${JAVA_INCLUDE_PATH2})
endif()

set_target_properties(
  ${java_face_sdk}
  PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}"
             RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../${ARTICRAFT_OUT}")
