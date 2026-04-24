set(ACTIVITY_MONITOR_SOURCES
    main.cpp
    ../app.rc
)

add_executable(ActivityMonitor WIN32 ${ACTIVITY_MONITOR_SOURCES})

target_link_libraries(ActivityMonitor
    PRIVATE
    MochiKit
)