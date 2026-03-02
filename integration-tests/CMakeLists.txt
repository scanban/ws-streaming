set(sources
    ./test_client_source_integration.cpp
)

foreach(source ${sources})

    get_filename_component(name ${source} NAME_WE)

    add_executable(${name} ${source})
    ws_streaming_enable_sanitizers_for_target(${name})

    set_target_properties(${name} PROPERTIES
        CXX_STANDARD            17
        CXX_STANDARD_REQUIRED   ON
        CXX_EXTENSIONS          ON)

    target_link_libraries(${name} PUBLIC
        GTest::gtest_main
        ws-streaming::ws-streaming)

    add_test(NAME ${name} COMMAND ${name})

    if(WS_STREAMING_ENABLE_COVERAGE AND DEFINED COVERAGE_TOOLS_FOUND AND COVERAGE_TOOLS_FOUND)
        enable_coverage_for_target(${name})
    endif()

endforeach()
