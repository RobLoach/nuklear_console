find_package(pntr REQUIRED)
find_package(pntr_app REQUIRED)

list(APPEND LIBRARIES
    pntr
    pntr_app
)

list(TRANSFORM SOURCES PREPEND ${PROJECT_SOURCE_DIR}/)

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

if (RAYLIB)
    find_package(raylib QUIET)
    set(project_name_raylib ${PROJECT_NAME}_raylib)

    add_executable(${project_name_raylib}
        ${SOURCES}
    )

    target_link_libraries(${project_name_raylib} PUBLIC
        raylib_static
        ${LIBRARIES}
    )

    # Platform Updates
    if (EMSCRIPTEN)
        set_target_properties(${project_name_raylib} PROPERTIES OUTPUT_NAME "index")
        set_target_properties(${project_name_raylib} PROPERTIES SUFFIX ".html")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY --preload-file ${PROJECT_SOURCE_DIR}/resources@/resources --shell-file ${CMAKE_CURRENT_LIST_DIR}/shell.html")

        target_compile_definitions(${project_name_raylib} PUBLIC
            PLATFORM=Web
        )
    else()
        set_property(TARGET ${project_name_raylib} PROPERTY C_STANDARD 99)
    endif()

    if (APPLE AND NOT EMSCRIPTEN)
        target_link_libraries(${project_name_raylib} PUBLIC "-framework IOKit")
        target_link_libraries(${project_name_raylib} PUBLIC "-framework Cocoa")
        target_link_libraries(${project_name_raylib} PUBLIC "-framework OpenGL")
    endif()

    target_compile_definitions(${project_name_raylib} PUBLIC
        PNTR_APP_RAYLIB
    )
endif()
