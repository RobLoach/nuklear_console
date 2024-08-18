cmake_minimum_required(VERSION 3.11)
project(nuklear_console
    DESCRIPTION "nuklear_console: Console-like interface for Nuklear GUI"
    HOMEPAGE_URL "https://github.com/robloach/nuklear_console"
    VERSION 0.0.1
    LANGUAGES C
)

# nuklear_gamepad
find_package(nuklear_gamepad QUIET)
if (NOT nuklear_gamepad_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nuklear_gamepad
        GIT_REPOSITORY https://github.com/RobLoach/nuklear_gamepad.git
        GIT_TAG c0de10a
    )
    FetchContent_GetProperties(nuklear_gamepad)
    if (NOT nuklear_gamepad_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(nuklear_gamepad)
        add_subdirectory(${nuklear_gamepad_SOURCE_DIR} ${nuklear_gamepad_BINARY_DIR})
    endif()
endif()

# nuklear_console
add_library(nuklear_console INTERFACE)
target_include_directories(nuklear_console INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(nuklear_console INTERFACE nuklear_gamepad)

# Options
if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(NUKLEAR_CONSOLE_IS_MAIN TRUE)
else()
    set(NUKLEAR_CONSOLE_IS_MAIN FALSE)
endif()

option(NUKLEAR_CONSOLE_BUILD_TESTS "Tests" ${NUKLEAR_CONSOLE_IS_MAIN})

# Examples
if (NUKLEAR_CONSOLE_BUILD_TESTS)
    include(CTest)
    enable_testing()
    if (BUILD_TESTING)
        # set(CTEST_CUSTOM_TESTS_IGNORE
        #     pkg-config--static
        # )
        # Always print verbose output when tests fail if run using `make test`.
        list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
        add_subdirectory(test)
    endif()
endif()
