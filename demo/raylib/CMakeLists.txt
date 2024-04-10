cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(nuklear_console_raylib)

# C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# raylib
find_package(raylib QUIET)
if (NOT raylib_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 04afba260f64ee2ea9ef0b4a6d2058c9ffdad66d
        GIT_SHALLOW 1
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
        add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
    endif()
endif()

# raylib_nuklear
find_package(raylib_nuklear QUIET)
if (NOT raylib_nuklear_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        raylib_nuklear
        GIT_REPOSITORY https://github.com/RobLoach/raylib-nuklear.git
        GIT_TAG 33ae426a4c2798e26b0fddf9760669d0aedfb6a0
        GIT_SHALLOW 1
    )
    FetchContent_GetProperties(raylib_nuklear)
    if (NOT raylib_nuklear_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib_nuklear)
        add_subdirectory(${raylib_nuklear_SOURCE_DIR} ${raylib_nuklear_BINARY_DIR})
    endif()
endif()

# nuklear_gamepad
find_package(nuklear_gamepad QUIET)
if (NOT nuklear_gamepad_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        nuklear_gamepad
        GIT_REPOSITORY https://github.com/RobLoach/nuklear_gamepad.git
        GIT_TAG f21610b890011b907bbb502b8313fe5cbc19feca
        GIT_SHALLOW 1
    )
    FetchContent_GetProperties(nuklear_gamepad)
    if (NOT nuklear_gamepad_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(nuklear_gamepad)
        add_subdirectory(${nuklear_gamepad_SOURCE_DIR} ${nuklear_gamepad_BINARY_DIR})
    endif()
endif()

# Setup the example
add_executable(${PROJECT_NAME} main.c)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC
    raylib
    raylib_nuklear
    nuklear_gamepad
)
