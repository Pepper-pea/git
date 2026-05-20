if(NOT BUILD_CONFIG STREQUAL "Release")
    return()
endif()

if(NOT EXISTS "${OPENCV_RUNTIME_DLL}")
    message(FATAL_ERROR "OpenCV runtime DLL not found: ${OPENCV_RUNTIME_DLL}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${OPENCV_RUNTIME_DLL}"
        "${TARGET_RUNTIME_DIR}"
    RESULT_VARIABLE copy_result
)

if(NOT copy_result EQUAL 0)
    message(FATAL_ERROR "Failed to copy OpenCV runtime DLL.")
endif()
