cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(nuklear_console_demo_raylib)

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
        GIT_TAG 5.5
    )
    FetchContent_GetProperties(raylib)
    if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(raylib)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
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
        GIT_TAG d2bd2b7
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
        GIT_TAG 49c1cba
    )
    FetchContent_GetProperties(nuklear_gamepad)
    if (NOT nuklear_gamepad_POPULATED) # Have we downloaded raylib yet?
        set(FETCHCONTENT_QUIET NO)
        FetchContent_Populate(nuklear_gamepad)
        add_subdirectory(${nuklear_gamepad_SOURCE_DIR} ${nuklear_gamepad_BINARY_DIR})
    endif()
endif()

# Resources
if (EXISTS ${PROJECT_SOURCE_DIR}/resources)
    file(GLOB resources ${PROJECT_SOURCE_DIR}/resources/*)
    set(examples_resources)
    list(APPEND examples_resources ${resources})
    file(COPY ${examples_resources} DESTINATION "resources/")
    set(EMSCRIPTEN_RESOURCES "--preload-file ${PROJECT_SOURCE_DIR}/resources@/resources")
else()
    set(EMSCRIPTEN_RESOURCES "")
endif()

# Setup the example
add_executable(${PROJECT_NAME} main.c)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC
    raylib
    raylib_nuklear
    nuklear_gamepad
)

# Web Configurations
#if (${PLATFORM} STREQUAL "Web")
if (EMSCRIPTEN)
    # Tell Emscripten to build an example.html file.
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sUSE_GLFW=3")
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "index")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file resources")
    #set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3")
    target_compile_definitions(${PROJECT_NAME} PUBLIC PLATFORM=Web)
    target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_BUILD_TYPE=Release)
endif()
