if(NOT DEFINED ROOT_DIR OR ROOT_DIR STREQUAL "")
    message(FATAL_ERROR "ROOT_DIR must be provided.")
endif()

file(GLOB_RECURSE gcda_files "${ROOT_DIR}/*.gcda")
file(GLOB_RECURSE gcov_files "${ROOT_DIR}/*.gcov")

set(coverage_files ${gcda_files} ${gcov_files})

if(coverage_files)
    file(REMOVE ${coverage_files})
endif()
