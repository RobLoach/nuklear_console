# nuklear_console_test
add_executable(nuklear_console_test nuklear_console_test.c)
target_compile_options(nuklear_console_test PRIVATE -Wall -Wextra -Wconversion -Wsign-conversion)
target_link_libraries(nuklear_console_test PUBLIC
    nuklear_console
    nuklear_gamepad
)

# Copy the resources
# file(GLOB resources resources/*)
# set(test_resources)
# list(APPEND test_resources ${resources})
# file(COPY ${test_resources} DESTINATION "resources/")

# Set up the test
list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")
add_test(NAME nuklear_console_test COMMAND nuklear_console_test)
