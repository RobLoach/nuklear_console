cmake_minimum_required(VERSION 3.11)
project(nuklear_console_demo_pntr
    DESCRIPTION "nuklear_console_demo_pntr"
    HOMEPAGE_URL "https://github.com/robloach/nuklear_console"
    VERSION 0.0.1
    LANGUAGES C
)

# CMAKE Modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/.cmake")

# Targets
option(RAYLIB "Build raylib" TRUE)

# Sources
set(SOURCES
    main.c
)

# Libraries
find_package(pntr_nuklear REQUIRED)
list(APPEND LIBRARIES pntr_nuklear)

find_package(nuklear_gamepad REQUIRED)
list(APPEND LIBRARIES nuklear_gamepad)

# Have pntr_app_starter build the project
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/.cmake")
find_package(pntr_app_starter)
