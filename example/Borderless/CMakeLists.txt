set(BORDERLESS_SOURCES
    borderless.cpp
    ../app.rc
)

add_executable(BorderlessPlayer WIN32 ${BORDERLESS_SOURCES})

target_link_libraries(BorderlessPlayer PRIVATE
    AureliaKit_GraphicComponent
)

add_custom_command(TARGET BorderlessPlayer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:BorderlessPlayer>
        ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:BorderlessPlayer>
    COMMENT "Copy BorderlessPlayer -> ${CMAKE_BINARY_DIR}"
)
