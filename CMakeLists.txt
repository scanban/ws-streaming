cmake_minimum_required(VERSION 3.24)

project(ws-streaming VERSION 3.0.2 LANGUAGES CXX)

option(WS_STREAMING_BUILD_EXAMPLES  "Build example programs"            ${PROJECT_IS_TOP_LEVEL})
option(WS_STREAMING_BUILD_TESTS     "Build unit tests"                  ${PROJECT_IS_TOP_LEVEL})
option(WS_STREAMING_BUILD_INTEGRATION_TESTS "Build integration tests"   ${WS_STREAMING_BUILD_TESTS})
option(WS_STREAMING_INSTALL         "Generate CMake install targets"    ON)
option(WS_STREAMING_ENABLE_SANITIZERS "Enable ASan+UBSan for library and tests (GNU only)" OFF)
option(WS_STREAMING_ENABLE_COVERAGE "Enable coverage reporting and checks for tests" OFF)

if(WS_STREAMING_BUILD_INTEGRATION_TESTS)
    set(WS_STREAMING_ENABLE_SANITIZERS ON CACHE BOOL
        "Enable ASan+UBSan for library and tests (GNU only)" FORCE)
endif()

include(CMakePackageConfigHelpers)
include(FetchContent)
include(GNUInstallDirs)
include(cmake/sanitizers.cmake)
if(WS_STREAMING_ENABLE_COVERAGE)
    include(cmake/coverage.cmake)
endif()

include(dependencies/Boost.cmake)
include(dependencies/nlohmann_json.cmake)
include(dependencies/Threads.cmake)

add_subdirectory(src)

if(WS_STREAMING_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(WS_STREAMING_BUILD_TESTS)
    enable_testing()
    include(dependencies/googletest.cmake)
    add_subdirectory(tests)
    if(WS_STREAMING_BUILD_INTEGRATION_TESTS)
        add_subdirectory(integration-tests)
    endif()
endif()
