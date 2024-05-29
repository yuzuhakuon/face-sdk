function(ncnnmem_generate)
  include(CMakeParseArguments)

  set(_options APPEND_PATH)
  set(_singleargs OUT_VAR NCNN_OUT_DIR)
  if(COMMAND target_sources)
    list(APPEND _singleargs TARGET)
  endif()
  set(_multiargs MODELS)

  cmake_parse_arguments(ncnnmem_generate "${_options}" "${_singleargs}"
                        "${_multiargs}" "${ARGN}")

  if(NOT ncnnmem_generate_NCNN_OUT_DIR)
    set(ncnnmem_generate_NCNN_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  set(ncnnmem_generate_MODEL_EXTENSIONS .param .bin)
  set(ncnnmem_generate_GENERATE_EXTENSIONS .id.h .mem.h)

  if(NOT ncnnmem_generate_MODELS)
    message(SEND_ERROR "Error: ncnnmem_generate could not find any model files")
    return()
  endif()

  set(_generated_srcs_all)
  foreach(_model ${ncnnmem_generate_MODELS})
    get_filename_component(_abs_file ${_model} ABSOLUTE)
    get_filename_component(_abs_dir ${_abs_file} DIRECTORY)
    get_filename_component(_file_full_name ${_model} NAME)
    get_filename_component(_basename ${_model} NAME_WLE)

    set(_model_files)
    foreach(_ext ${ncnnmem_generate_MODEL_EXTENSIONS})
      list(APPEND _model_files "${_abs_file}${_ext}")
    endforeach()

    set(_generated_srcs)
    foreach(_ext ${ncnnmem_generate_GENERATE_EXTENSIONS})
      list(APPEND _generated_srcs
           "${ncnnmem_generate_NCNN_OUT_DIR}/${_basename}${_ext}")
    endforeach()
    list(APPEND _generated_srcs_all ${_generated_srcs})

    set(_comment "Running ncnn2mem on ${_basename}")
    message(STATUS ${_comment})
    add_custom_command(
      OUTPUT ${_generated_srcs}
      COMMAND ncnn2mem ${_model_files} ${_generated_srcs}
      DEPENDS ${_model_files}
      COMMENT ${_comment})
  endforeach()

  set_source_files_properties(${_generated_srcs_all} PROPERTIES GENERATED TRUE)

  message(STATUS ${_generated_srcs_all})
  message(STATUS ${ncnnmem_generate_NCNN_OUT_DIR})

  if(ncnnmem_generate_OUT_VAR)
    set(${ncnnmem_generate_OUT_VAR}
        ${_generated_srcs_all}
        PARENT_SCOPE)
  endif()

  if(ncnnmem_generate_TARGET)
    target_include_directories(${ncnnmem_generate_TARGET}
                               PUBLIC ${ncnnmem_generate_NCNN_OUT_DIR})
    target_sources(${ncnnmem_generate_TARGET} PRIVATE ${_generated_srcs_all})
  endif()

  message(STATUS ${ncnnmem_generate_OUT_VAR})

endfunction()

function(set_ncnn_root_dir)
  if(NOT DEFINED NCNN_ROOT)
    if(ANDROID)
      # Android version
      set(NCNN_ROOT /usr/local/android/ncnn)
    else()
      if(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(NCNN_ROOT /usr/local/ncnn)
      elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
        set(NCNN_ROOT /usr/local/ncnn)
      endif()
    endif()
  endif()

  if(ANDROID)
    set(ncnn_DIR
        "${NCNN_ROOT}/${ANDROID_ABI}/lib/cmake/ncnn"
        PARENT_SCOPE)
  else()
    set(ncnn_DIR
        "${NCNN_ROOT}/lib/cmake/ncnn"
        PARENT_SCOPE)
  endif()
endfunction()
