set(REACTUI_SOURCES
    main.cpp
    ../app.rc
)

# ── Step 1: build the JS bundle with bun ─────────────────────────────────────
set(REACTUI_BUNDLE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui/examples/App.tsx
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui/src/index.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui/package.json
)
set(REACTUI_BUNDLE_OUT ${CMAKE_CURRENT_SOURCE_DIR}/bundle.js)

add_custom_command(
    OUTPUT  ${REACTUI_BUNDLE_OUT}
    COMMAND ${BUN_EXECUTABLE} run bundle:demo
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/reactui
    DEPENDS ${REACTUI_BUNDLE_SRC} ${SPHERE_MODULES_INSTALL_STAMP}
    COMMENT "Building React UI JS bundle (bun -> bundle.js)"
)

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
