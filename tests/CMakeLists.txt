set(sources
    ./test_base64.cpp
    ./test_linear_table.cpp
    ./test_semver.cpp
    ./test_url.cpp
    ./test_websocket_protocol.cpp
)

if(WS_STREAMING_ENABLE_COVERAGE)
    message(STATUS "Coverage enabled for tests")
endif()

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

    # Enable coverage for test executables if coverage is enabled
    if(WS_STREAMING_ENABLE_COVERAGE AND DEFINED COVERAGE_TOOLS_FOUND AND COVERAGE_TOOLS_FOUND)
        enable_coverage_for_target(${name})
    endif()

endforeach()

if(WS_STREAMING_ENABLE_COVERAGE AND DEFINED COVERAGE_TOOLS_FOUND AND COVERAGE_TOOLS_FOUND)
    add_custom_target(coverage-check-websocket_protocol
        COMMAND ${CMAKE_COMMAND}
            -DROOT_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/clear_coverage_data.cmake
        COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} -R test_websocket_protocol
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
        COMMAND ${CMAKE_COMMAND}
            -DLCOV_TOOL=${LCOV_TOOL}
            -DCOVERAGE_INFO=${CMAKE_BINARY_DIR}/coverage.info.cleaned
            -DMODULE_INFO=${CMAKE_BINARY_DIR}/websocket_protocol.info
            -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/detail/websocket_protocol.cpp
            -DHEADER_FILE=${CMAKE_SOURCE_DIR}/include/ws-streaming/detail/websocket_protocol.hpp
            -DMODULE_LABEL=websocket_protocol
            -DMIN_LINE_PERCENT=85
            -DMIN_BRANCH_PERCENT=80
            -P ${CMAKE_SOURCE_DIR}/cmake/check_lcov_thresholds.cmake
        COMMENT "Running websocket_protocol tests and coverage threshold checks"
        VERBATIM
    )

    add_custom_target(coverage-check-url
        COMMAND ${CMAKE_COMMAND}
            -DROOT_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/clear_coverage_data.cmake
        COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} -R test_url
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
        COMMAND ${CMAKE_COMMAND}
            -DLCOV_TOOL=${LCOV_TOOL}
            -DCOVERAGE_INFO=${CMAKE_BINARY_DIR}/coverage.info.cleaned
            -DMODULE_INFO=${CMAKE_BINARY_DIR}/url.info
            -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/detail/url.cpp
            -DHEADER_FILE=${CMAKE_SOURCE_DIR}/include/ws-streaming/detail/url.hpp
            -DMODULE_LABEL=url
            -DMIN_LINE_PERCENT=85
            -DMIN_BRANCH_PERCENT=80
            -P ${CMAKE_SOURCE_DIR}/cmake/check_lcov_thresholds.cmake
        COMMENT "Running URL tests and coverage threshold checks"
        VERBATIM
    )

    add_custom_target(coverage-check-semver
        COMMAND ${CMAKE_COMMAND}
            -DROOT_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/clear_coverage_data.cmake
        COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} -R test_semver
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
        COMMAND ${CMAKE_COMMAND}
            -DLCOV_TOOL=${LCOV_TOOL}
            -DCOVERAGE_INFO=${CMAKE_BINARY_DIR}/coverage.info.cleaned
            -DMODULE_INFO=${CMAKE_BINARY_DIR}/semver.info
            -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/detail/semver.cpp
            -DHEADER_FILE=${CMAKE_SOURCE_DIR}/include/ws-streaming/detail/semver.hpp
            -DMODULE_LABEL=semver
            -DMIN_LINE_PERCENT=85
            -DMIN_BRANCH_PERCENT=80
            -P ${CMAKE_SOURCE_DIR}/cmake/check_lcov_thresholds.cmake
        COMMENT "Running semver tests and coverage threshold checks"
        VERBATIM
    )

    add_custom_target(coverage-check-linear_table
        COMMAND ${CMAKE_COMMAND}
            -DROOT_DIR=${CMAKE_BINARY_DIR}
            -P ${CMAKE_SOURCE_DIR}/cmake/clear_coverage_data.cmake
        COMMAND ${CMAKE_COMMAND} -E env CTEST_OUTPUT_ON_FAILURE=1
            ${CMAKE_CTEST_COMMAND} --test-dir ${CMAKE_BINARY_DIR} -R test_linear_table
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target coverage
        COMMAND ${CMAKE_COMMAND}
            -DLCOV_TOOL=${LCOV_TOOL}
            -DCOVERAGE_INFO=${CMAKE_BINARY_DIR}/coverage.info.cleaned
            -DMODULE_INFO=${CMAKE_BINARY_DIR}/linear_table.info
            -DSOURCE_FILE=${CMAKE_SOURCE_DIR}/src/detail/linear_table.cpp
            -DHEADER_FILE=${CMAKE_SOURCE_DIR}/include/ws-streaming/detail/linear_table.hpp
            -DMODULE_LABEL=linear_table
            -DMIN_LINE_PERCENT=85
            -DMIN_BRANCH_PERCENT=80
            -P ${CMAKE_SOURCE_DIR}/cmake/check_lcov_thresholds.cmake
        COMMENT "Running linear_table tests and coverage threshold checks"
        VERBATIM
    )
endif()
