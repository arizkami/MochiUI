set(MIXER_SOURCES
    mixer.cpp
    ../app.rc
)

add_executable(MixerConsole WIN32 ${MIXER_SOURCES})

target_link_libraries(MixerConsole PRIVATE
    MochiKit_GraphicComponent
)

# Copy exe to top-level ${CMAKE_BINARY_DIR} (same as project root build folder).
add_custom_command(TARGET MixerConsole POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:MixerConsole>
        ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:MixerConsole>
    COMMENT "Copy MixerConsole -> ${CMAKE_BINARY_DIR}"
)
