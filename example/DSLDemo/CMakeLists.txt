set(DSL_DEMO_SOURCES
    main.cpp
    ../app.rc
)

# ── DSLDemo resource bundle (res://ui/*.xml) ─────────────────────────────────
set(DSLDEMO_RESOURCES_XML        ${CMAKE_CURRENT_SOURCE_DIR}/resource.xml)
set(DSLDEMO_GENERATED_RES_H      ${CMAKE_CURRENT_BINARY_DIR}/generated/DSLDemoResources.hpp)
set(DSLDEMO_GENERATED_RES_CPP    ${CMAKE_CURRENT_BINARY_DIR}/generated/DSLDemoResources.cpp)

add_custom_command(
    OUTPUT ${DSLDEMO_GENERATED_RES_H} ${DSLDEMO_GENERATED_RES_CPP}
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/gen_resources.py
            ${DSLDEMO_RESOURCES_XML} ${DSLDEMO_GENERATED_RES_H} ${DSLDEMO_GENERATED_RES_CPP}
            InitDSLDemoResources
    DEPENDS ${DSLDEMO_RESOURCES_XML}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/gen_resources.py
            ${CMAKE_CURRENT_SOURCE_DIR}/ui/navbar.yml
            ${CMAKE_CURRENT_SOURCE_DIR}/ui/container.yml
    COMMENT "Generating DSLDemo binary resources from ${DSLDEMO_RESOURCES_XML}"
)

list(APPEND DSL_DEMO_SOURCES ${DSLDEMO_GENERATED_RES_CPP})

add_executable(DSLDemo WIN32 ${DSL_DEMO_SOURCES})

target_include_directories(DSLDemo PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(DSLDemo PRIVATE
    AureliaKit_GraphicComponent
    AureliaKit_DSL
)

# ── Stage YAML assets next to the executable ─────────────────────────────────
# The DSL demo loads `ui/navbar.yml` at runtime, so mirror the source `ui/`
# folder into the build output every time we link.
add_custom_command(TARGET DSLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        $<TARGET_FILE_DIR:DSLDemo>/ui
    COMMENT "Stage DSLDemo UI assets -> $<TARGET_FILE_DIR:DSLDemo>/ui"
)

# Copy exe to top-level ${CMAKE_BINARY_DIR} (same as project root build folder)
# along with its `ui/` payload so it can be launched from there too.
add_custom_command(TARGET DSLDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:DSLDemo>
        ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:DSLDemo>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_BINARY_DIR}/ui
    COMMENT "Copy DSLDemo + UI assets -> ${CMAKE_BINARY_DIR}"
)
