if (NOT NUKLEAR_GAMEPAD_VERSION)
    set(NUKLEAR_GAMEPAD_VERSION 1.1.0)
endif()

include(FetchContent)
FetchContent_Declare(
    nuklear_gamepad
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/RobLoach/nuklear_gamepad/archive/refs/tags/v${NUKLEAR_GAMEPAD_VERSION}.tar.gz
)
FetchContent_GetProperties(nuklear_gamepad)

if (NOT nuklear_gamepad_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(nuklear_gamepad)
    add_subdirectory(${nuklear_gamepad_SOURCE_DIR} ${nuklear_gamepad_BINARY_DIR})
endif()
