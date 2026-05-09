set(ACTIVITY_MONITOR_SOURCES
    main.cpp
    ../app.rc
)

add_executable(ActivityMonitor WIN32 ${ACTIVITY_MONITOR_SOURCES})

target_link_libraries(ActivityMonitor PRIVATE
    SphereKit_GraphicComponent
)

# Copy exe to top-level ${CMAKE_BINARY_DIR} (same as project root build folder).
add_custom_command(TARGET ActivityMonitor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ActivityMonitor>
        ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:ActivityMonitor>
    COMMENT "Copy ActivityMonitor -> ${CMAKE_BINARY_DIR}"
)
