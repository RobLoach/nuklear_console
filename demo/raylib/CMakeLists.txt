cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(nuklear_console_demo_raylib)

# C
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Register the cmake folder for find_package()
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.cmake")

# raylib
find_package(raylib REQUIRED)

# raylib_nuklear
find_package(raylib_nuklear REQUIRED)

# nuklear_gamepad
find_package(nuklear_gamepad REQUIRED)

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
target_compile_options(${PROJECT_NAME} PRIVATE -g)

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
