add_library(MikoUI INTERFACE)

target_include_directories(MikoUI INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(MikoUI INTERFACE
    AureliaKit_GraphicComponent
    AureliaKit_JavaScriptEngine
    Aurelia_React
)

