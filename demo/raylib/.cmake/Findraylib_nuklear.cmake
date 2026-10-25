if (NOT RAYLIB_NUKLEAR_VERSION)
    set(RAYLIB_NUKLEAR_VERSION 6.0.1)
endif()

include(FetchContent)
FetchContent_Declare(
    raylib_nuklear
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/RobLoach/raylib-nuklear/archive/refs/tags/v${RAYLIB_NUKLEAR_VERSION}.tar.gz
)
FetchContent_GetProperties(raylib_nuklear)

if (NOT raylib_nuklear_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib_nuklear)
    add_subdirectory(${raylib_nuklear_SOURCE_DIR} ${raylib_nuklear_BINARY_DIR})
endif()
