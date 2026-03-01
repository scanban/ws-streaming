function(ws_streaming_enable_sanitizers_for_target TARGET_NAME)
    if(NOT WS_STREAMING_ENABLE_SANITIZERS)
        return()
    endif()

    target_compile_options(${TARGET_NAME} PRIVATE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    target_link_options(${TARGET_NAME} PRIVATE
        -fsanitize=address,undefined
    )
endfunction()

if(WS_STREAMING_ENABLE_SANITIZERS AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR
        "WS_STREAMING_ENABLE_SANITIZERS is currently supported only with GNU compilers.")
endif()
