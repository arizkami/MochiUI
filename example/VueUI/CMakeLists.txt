set(VUEUI_SOURCES
    main.cpp
    ../app.rc
)

set(VUEUI_BUNDLE_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/example/App.vue
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/src/index.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/src/core/api.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/src/core/renderer.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/src/payload/types.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/src/wrapper/components.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/src/wrapper/stylesheet.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/build-demo.ts
    ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui/package.json
)
set(VUEUI_BUNDLE_OUT ${CMAKE_CURRENT_SOURCE_DIR}/bundle.js)

add_custom_command(
    OUTPUT  ${VUEUI_BUNDLE_OUT}
    COMMAND ${BUN_EXECUTABLE} run bundle:demo
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../modules/vueui
    DEPENDS ${VUEUI_BUNDLE_SRC} ${SPHERE_MODULES_INSTALL_STAMP}
    COMMENT "Building Vue UI JS bundle (bun -> bundle.js)"
)

set(VUEUI_RESOURCE_XML       ${CMAKE_CURRENT_SOURCE_DIR}/resource.xml)
set(VUEUI_GENERATED_RES_H    ${CMAKE_CURRENT_BINARY_DIR}/generated/VueUIResources.hpp)
set(VUEUI_GENERATED_RES_CPP  ${CMAKE_CURRENT_BINARY_DIR}/generated/VueUIResources.cpp)

add_custom_command(
    OUTPUT  ${VUEUI_GENERATED_RES_H} ${VUEUI_GENERATED_RES_CPP}
    COMMAND ${BUN_EXECUTABLE} run
            ${SPHERE_RESOURCE_GENERATOR}
            ${VUEUI_RESOURCE_XML}
            ${VUEUI_GENERATED_RES_H}
            ${VUEUI_GENERATED_RES_CPP}
            InitVueUIResources
    DEPENDS ${VUEUI_RESOURCE_XML}
            ${VUEUI_BUNDLE_OUT}
            ${SPHERE_RESOURCE_GENERATOR}
    COMMENT "Embedding bundle.js into VueUIResources.cpp"
)

list(APPEND VUEUI_SOURCES ${VUEUI_GENERATED_RES_CPP})

add_executable(VueUIDemo WIN32 ${VUEUI_SOURCES})

target_include_directories(VueUIDemo PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

target_link_libraries(VueUIDemo PRIVATE
    SphereKit_GraphicComponent
    SphereKit_JavaScriptEngine
    Sphere_Vue
)

add_custom_command(TARGET VueUIDemo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:VueUIDemo>
        ${CMAKE_BINARY_DIR}/$<TARGET_FILE_NAME:VueUIDemo>
    COMMENT "Copy VueUIDemo -> ${CMAKE_BINARY_DIR}"
)
