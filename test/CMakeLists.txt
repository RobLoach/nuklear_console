# pntr
find_package(pntr QUIET)
if (NOT pntr)
    include(FetchContent)
    FetchContent_Declare(
        pntr
        GIT_REPOSITORY https://github.com/RobLoach/pntr.git
        GIT_TAG 650237a
    )
    FetchContent_GetProperties(pntr)
    if (NOT pntr) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(pntr)
        add_subdirectory(${pntr_SOURCE_DIR} ${pntr_BINARY_DIR})
    endif()
endif()

# pntr_nuklear
find_package(pntr_nuklear QUIET)
if (NOT pntr_nuklear)
    include(FetchContent)
    FetchContent_Declare(
        pntr_nuklear
        GIT_REPOSITORY https://github.com/RobLoach/pntr_nuklear.git
        GIT_TAG d651de0
    )
    FetchContent_GetProperties(pntr_nuklear)
    if (NOT pntr_nuklear_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(pntr_nuklear)
        add_subdirectory(${pntr_nuklear_SOURCE_DIR} ${pntr_nuklear_BINARY_DIR})
    endif()
endif()

set(CTEST_OUTPUT_ON_FAILURE TRUE)

# nuklear_console_test
add_executable(nuklear_console_test nuklear_console_test.c)
target_compile_options(nuklear_console_test PRIVATE -Wall -Wextra -Wconversion -Wsign-conversion)
target_link_libraries(nuklear_console_test PUBLIC
    nuklear_console
    nuklear_gamepad
    pntr
    pntr_nuklear
)

# Copy the resources
file(GLOB resources resources/*)
set(test_resources)
list(APPEND test_resources ${resources})
file(COPY ${test_resources} DESTINATION "resources/")

# Set up the test
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
add_test(NAME nuklear_console_test
    COMMAND nuklear_console_test
)
