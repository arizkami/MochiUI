set(REACTUI_SOURCES
    main.cpp
    ../app.rc
)

# ── Step 1: build the JS bundle with bun ─────────────────────────────────────
# Search the user-local bun install location in addition to PATH, because
# vcvars64.bat may not propagate it when cmake reconfigures.
find_program(BUN_EXECUTABLE bun
    PATHS
        "$ENV{USERPROFILE}/.bun/bin"
        "$ENV{HOME}/.bun/bin"
)
if(NOT BUN_EXECUTABLE)
    message(WARNING "ReactUI: 'bun' not found. Using pre-built bundle.js. "
                    "Run `bun run bundle:demo` in modules/reactui/ to rebuild from examples/App.tsx.")
endif()

set(REACTUI_BUNDLE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui/examples/App.tsx
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui/src/index.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui/package.json
)
set(REACTUI_BUNDLE_OUT ${CMAKE_CURRENT_SOURCE_DIR}/bundle.js)

if(BUN_EXECUTABLE)
    add_custom_command(
        OUTPUT  ${REACTUI_BUNDLE_OUT}
        COMMAND ${BUN_EXECUTABLE} run bundle:demo
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui
        DEPENDS ${REACTUI_BUNDLE_SRC}
        COMMENT "Building React UI JS bundle (bun -> bundle.js)"
    )
else()
    # bun unavailable: treat pre-built bundle.js as always up-to-date.
    add_custom_command(
        OUTPUT  ${REACTUI_BUNDLE_OUT}
        COMMAND ${CMAKE_COMMAND} -E echo "Using pre-built bundle.js"
        COMMENT "ReactUI: skipping bun build (bun not found)"
    )
endif()

# ── Step 2: embed bundle.js as a C++ byte array ───────────────────────────────
set(REACTUI_RESOURCE_XML     ${CMAKE_CURRENT_SOURCE_DIR}/resource.xml)
set(REACTUI_GENERATED_RES_H   ${CMAKE_CURRENT_BINARY_DIR}/generated/ReactUIResources.hpp)
set(REACTUI_GENERATED_RES_CPP ${CMAKE_CURRENT_BINARY_DIR}/generated/ReactUIResources.cpp)

add_custom_command(
    OUTPUT  ${REACTUI_GENERATED_RES_H} ${REACTUI_GENERATED_RES_CPP}
    COMMAND ${SPHX_PYTHON_CMD}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/gen_resources.py
            ${REACTUI_RESOURCE_XML}
            ${REACTUI_GENERATED_RES_H}
            ${REACTUI_GENERATED_RES_CPP}
            InitReactUIResources
    DEPENDS ${REACTUI_RESOURCE_XML}
            ${REACTUI_BUNDLE_OUT}
            ${CMAKE_CURRENT_SOURCE_DIR}/../../scripts/gen_resources.py
    COMMENT "Embedding bundle.js into ReactUIResources.cpp"
)

list(APPEND REACTUI_SOURCES ${REACTUI_GENERATED_RES_CPP})

# ── Executable ────────────────────────────────────────────────────────────────
add_executable(ReactUIDemo WIN32 ${REACTUI_SOURCES})

target_include_directories(ReactUIDemo PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(ReactUIDemo PRIVATE
    SphereKit_GraphicComponent
    SphereKit_JavaScriptEngine
    Sphere_React
)

# ── Stage artefacts ───────────────────────────────────────────────────────────
add_custom_command(TARGET ReactUIDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:ReactUIDemo>
        ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:ReactUIDemo>
    COMMENT "Copy ReactUIDemo -> ${CMAKE_BINARY_DIR}"
)
