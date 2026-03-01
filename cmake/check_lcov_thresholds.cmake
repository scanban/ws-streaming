if(NOT DEFINED LCOV_TOOL OR LCOV_TOOL STREQUAL "")
    message(FATAL_ERROR "LCOV tool is not configured.")
endif()

if(NOT DEFINED COVERAGE_INFO OR NOT EXISTS "${COVERAGE_INFO}")
    message(FATAL_ERROR "Coverage info file not found: ${COVERAGE_INFO}")
endif()

if(NOT DEFINED MODULE_INFO OR MODULE_INFO STREQUAL "")
    message(FATAL_ERROR "MODULE_INFO must be provided.")
endif()

if(NOT DEFINED SOURCE_FILE OR SOURCE_FILE STREQUAL "")
    message(FATAL_ERROR "SOURCE_FILE must be provided.")
endif()

if(NOT DEFINED HEADER_FILE OR HEADER_FILE STREQUAL "")
    message(FATAL_ERROR "HEADER_FILE must be provided.")
endif()

if(NOT DEFINED MIN_LINE_PERCENT)
    message(FATAL_ERROR "MIN_LINE_PERCENT must be provided.")
endif()

if(NOT DEFINED MIN_BRANCH_PERCENT)
    message(FATAL_ERROR "MIN_BRANCH_PERCENT must be provided.")
endif()

if(NOT DEFINED MODULE_LABEL OR MODULE_LABEL STREQUAL "")
    set(MODULE_LABEL "module")
endif()

execute_process(
    COMMAND "${LCOV_TOOL}"
        --extract "${COVERAGE_INFO}"
        "${SOURCE_FILE}"
        "${HEADER_FILE}"
        --output-file "${MODULE_INFO}"
        --rc lcov_branch_coverage=1
    RESULT_VARIABLE extract_result
    OUTPUT_VARIABLE extract_stdout
    ERROR_VARIABLE extract_stderr
)

if(NOT extract_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to extract module coverage.\n"
        "stdout:\n${extract_stdout}\n"
        "stderr:\n${extract_stderr}")
endif()

execute_process(
    COMMAND "${LCOV_TOOL}"
        --summary "${MODULE_INFO}"
        --rc lcov_branch_coverage=1
    RESULT_VARIABLE summary_result
    OUTPUT_VARIABLE summary_stdout
    ERROR_VARIABLE summary_stderr
)

if(NOT summary_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to summarize module coverage.\n"
        "stdout:\n${summary_stdout}\n"
        "stderr:\n${summary_stderr}")
endif()

message(STATUS "${summary_stdout}")

string(REGEX MATCH "lines\\.+: ([0-9]+(\\.[0-9]+)?)%" _line_match "${summary_stdout}")
if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "Could not parse line coverage from lcov summary.")
endif()
set(line_percent "${CMAKE_MATCH_1}")

string(REGEX MATCH "branches\\.+: ([0-9]+(\\.[0-9]+)?)%" _branch_match "${summary_stdout}")
if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "Could not parse branch coverage from lcov summary.")
endif()
set(branch_percent "${CMAKE_MATCH_1}")

math(EXPR min_line_bp "${MIN_LINE_PERCENT} * 100")
math(EXPR min_branch_bp "${MIN_BRANCH_PERCENT} * 100")

if("${line_percent}" MATCHES "^([0-9]+)(\\.([0-9]+))?$")
    set(line_int "${CMAKE_MATCH_1}")
    set(line_frac "${CMAKE_MATCH_3}")
else()
    message(FATAL_ERROR "Unexpected line coverage format: ${line_percent}")
endif()

if("${branch_percent}" MATCHES "^([0-9]+)(\\.([0-9]+))?$")
    set(branch_int "${CMAKE_MATCH_1}")
    set(branch_frac "${CMAKE_MATCH_3}")
else()
    message(FATAL_ERROR "Unexpected branch coverage format: ${branch_percent}")
endif()

if(line_frac STREQUAL "")
    set(line_frac "0")
endif()

if(branch_frac STREQUAL "")
    set(branch_frac "0")
endif()

set(line_frac_padded "${line_frac}00")
set(branch_frac_padded "${branch_frac}00")

string(SUBSTRING "${line_frac_padded}" 0 2 line_frac_two_digits)
string(SUBSTRING "${branch_frac_padded}" 0 2 branch_frac_two_digits)

math(EXPR line_bp "${line_int} * 100 + ${line_frac_two_digits}")
math(EXPR branch_bp "${branch_int} * 100 + ${branch_frac_two_digits}")

if(line_bp LESS min_line_bp OR branch_bp LESS min_branch_bp)
    message(FATAL_ERROR
        "Coverage thresholds not met for ${MODULE_LABEL}. "
        "Line=${line_percent}% (required >= ${MIN_LINE_PERCENT}%), "
        "Branch=${branch_percent}% (required >= ${MIN_BRANCH_PERCENT}%).")
endif()

message(STATUS
    "Coverage thresholds satisfied for ${MODULE_LABEL}: "
    "Line=${line_percent}%, Branch=${branch_percent}%.")
