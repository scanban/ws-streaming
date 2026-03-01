# CMake module for enabling code coverage support with lcov
#
# This module provides functionality to enable code coverage reporting
# using gcov and lcov when building with GCC or Clang.
#
# Usage:
#   include(coverage)
#   enable_coverage()
#
# This will:
#   - Detect available coverage tools (gcov, lcov, genhtml)
#   - Set up compiler and linker flags for coverage
#   - Create coverage targets (coverage, coverage-clean, coverage-report)
#
# Requirements:
#   - Tests must be enabled (WS_STREAMING_BUILD_TESTS=ON)
#   - Coverage tools must be available on the system
#   - GCC or Clang compiler must be used

# Only proceed if we're using GCC or Clang
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Find gcov tool
    find_program(GCOV_TOOL NAMES gcov-${CMAKE_CXX_COMPILER_VERSION} gcov)
    if(GCOV_TOOL)
        message(STATUS "Found gcov: ${GCOV_TOOL}")
    else()
        message(WARNING "gcov not found - coverage reporting will be disabled")
    endif()

    # Find lcov tool
    find_program(LCOV_TOOL NAMES lcov)
    if(LCOV_TOOL)
        message(STATUS "Found lcov: ${LCOV_TOOL}")
    else()
        message(WARNING "lcov not found - coverage reporting will be disabled")
    endif()

    # Find genhtml tool
    find_program(GENHTML_TOOL NAMES genhtml)
    if(GENHTML_TOOL)
        message(STATUS "Found genhtml: ${GENHTML_TOOL}")
    else()
        message(WARNING "genhtml not found - HTML coverage reports will be disabled")
    endif()

    # Check if all required tools are available
    if(GCOV_TOOL AND LCOV_TOOL)
        set(COVERAGE_TOOLS_FOUND TRUE)
        message(STATUS "Coverage tools found - coverage support enabled")
    else()
        set(COVERAGE_TOOLS_FOUND FALSE)
        message(STATUS "Coverage tools not found - coverage support disabled")
    endif()

    # Function to enable coverage for a target
    function(enable_coverage_for_target TARGET_NAME)
        if(COVERAGE_TOOLS_FOUND)
            # Add coverage compiler flags
            target_compile_options(${TARGET_NAME} PRIVATE
                --coverage
                -fprofile-arcs
                -ftest-coverage
            )
            
            # Add coverage linker flags
            target_link_options(${TARGET_NAME} PRIVATE
                --coverage
                -fprofile-arcs
                -ftest-coverage
            )
            
            message(STATUS "Coverage enabled for target: ${TARGET_NAME}")
        else()
            message(WARNING "Coverage tools not available - skipping coverage for ${TARGET_NAME}")
        endif()
    endfunction()

    # Function to enable coverage globally
    function(enable_coverage)
        if(COVERAGE_TOOLS_FOUND)
            # Add coverage flags to all targets
            add_compile_options(
                --coverage
                -fprofile-arcs
                -ftest-coverage
            )
            
            add_link_options(
                --coverage
                -fprofile-arcs
                -ftest-coverage
            )
            
            message(STATUS "Global coverage enabled")
        else()
            message(WARNING "Coverage tools not available - global coverage disabled")
        endif()
    endfunction()

    # Create coverage targets only if tools are available
    if(COVERAGE_TOOLS_FOUND)
        # Coverage clean target
        add_custom_target(coverage-clean
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/*.info ${CMAKE_BINARY_DIR}/*.info.cleaned
            COMMENT "Cleaning coverage data and reports"
        )

        # Coverage target - generates coverage report
        add_custom_target(coverage
            COMMAND ${LCOV_TOOL} --directory . --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info --rc lcov_branch_coverage=1
            COMMAND ${LCOV_TOOL} --remove ${CMAKE_BINARY_DIR}/coverage.info '/usr/*' '${CMAKE_SOURCE_DIR}/tests/*' --output-file ${CMAKE_BINARY_DIR}/coverage.info.cleaned --rc lcov_branch_coverage=1
            COMMAND ${GENHTML_TOOL} --branch-coverage --output-directory ${CMAKE_BINARY_DIR}/coverage ${CMAKE_BINARY_DIR}/coverage.info.cleaned
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS coverage-clean
            COMMENT "Generating coverage report"
            VERBATIM
        )

        # Coverage report target - just shows the report location
        add_custom_target(coverage-report
            COMMAND ${CMAKE_COMMAND} -E echo "Coverage report generated at: file://${CMAKE_BINARY_DIR}/coverage/index.html"
            COMMENT "Coverage report available at: ${CMAKE_BINARY_DIR}/coverage/index.html"
        )

        # Make coverage-report depend on coverage
        add_dependencies(coverage-report coverage)

        message(STATUS "Coverage targets created: coverage, coverage-clean, coverage-report")
    endif()

else()
    message(STATUS "Coverage not supported for compiler: ${CMAKE_CXX_COMPILER_ID}")
    set(COVERAGE_TOOLS_FOUND FALSE)
endif()