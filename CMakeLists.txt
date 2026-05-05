cmake_minimum_required(VERSION 3.15)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(Aurelia)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match Skia's official build runtime / arch per platform
if(MSVC)
  string(REPLACE "/MD"  "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
  string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}")
  # V8 checks __cplusplus on MSVC; ensure it reflects -std:c++20.
  add_compile_options(/arch:AVX2 /wd4244 /wd4267 /Zc:__cplusplus)
elseif(UNIX AND NOT APPLE)
  # Skia's prebuilt was built with -fPIC -fvisibility=hidden -mavx2; mirror.
  add_compile_options(-mavx2 -fPIC -fvisibility=hidden -fvisibility-inlines-hidden)
endif()

# ── Skia prebuilt binaries ────────────────────────────────────────────────────
set(SKIA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/skia)

if(WIN32)
  set(SKIA_ARCHIVE  ${CMAKE_CURRENT_SOURCE_DIR}/external/skia-prebuilt-binary.zip)
  set(SKIA_URL      "https://github.com/arizkami/Aurelia/releases/download/2026.05/skia-prebuilt-binary.zip")
  set(SKIA_SENTINEL ${SKIA_DIR}/lib/skia.lib)
elseif(UNIX AND NOT APPLE)
  set(SKIA_ARCHIVE  ${CMAKE_CURRENT_SOURCE_DIR}/external/skia-prebuilt-linux.tar.gz)
  set(SKIA_URL      "https://github.com/arizkami/Aurelia/releases/download/2026.05/skia-prebuilt-linux.tar.gz")
  set(SKIA_SENTINEL ${SKIA_DIR}/lib/libskia.a)
else()
  message(FATAL_ERROR "Aurelia: unsupported platform '${CMAKE_SYSTEM_NAME}'. "
                        "Only Windows and Linux are wired up so far.")
endif()

if(NOT EXISTS ${SKIA_SENTINEL})
  message(STATUS "Skia binaries not found. Downloading from ${SKIA_URL}...")
  file(DOWNLOAD ${SKIA_URL} ${SKIA_ARCHIVE} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
  list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
  if(NOT STATUS_CODE EQUAL 0)
    message(FATAL_ERROR "Failed to download Skia binaries from ${SKIA_URL}")
  endif()
  message(STATUS "Extracting Skia binaries...")
  file(ARCHIVE_EXTRACT INPUT ${SKIA_ARCHIVE}
         DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/skia)
  file(REMOVE ${SKIA_ARCHIVE})
  message(STATUS "Skia binaries installed successfully!")
else()
  message(STATUS "Skia binaries found at ${SKIA_DIR}")
endif()

# ── V8 prebuilt (Windows x64 monolith) ────────────────────────────────────────
set(V8_ROOT "")
if(WIN32)
  set(V8_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/v8)
  set(V8_ARCHIVE ${CMAKE_CURRENT_SOURCE_DIR}/external/v8-windows_x64.zip)
  set(V8_URL "https://github.com/arizkami/Aurelia/releases/download/2026.04/v8-windows_x64.zip")
  set(V8_SENTINEL ${V8_DIR}/lib/v8_monolith.lib)

  # If a previous configure extracted the zip into `external/prebuilt/{bin,include,lib,...}`
  # instead of `external/prebuilt/v8/...`, repair the layout once.
  if(NOT EXISTS "${V8_DIR}/include/v8.h" AND
     NOT EXISTS "${V8_DIR}/lib/v8_monolith.lib" AND
     EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/include/v8.h" AND
     EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/v8/lib/v8_monolith.lib")
    message(WARNING "V8 was extracted into external/prebuilt/. Moving into external/prebuilt/v8/ ...")
    file(MAKE_DIRECTORY "${V8_DIR}")
    foreach(_d bin include lib src third_party)
      if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/${_d}")
        file(RENAME
          "${CMAKE_CURRENT_SOURCE_DIR}/external/prebuilt/${_d}"
          "${V8_DIR}/${_d}")
      endif()
    endforeach()
  endif()

  if(NOT EXISTS "${V8_DIR}/include/v8.h" OR NOT EXISTS "${V8_DIR}/lib/v8_monolith.lib")
    message(STATUS "V8 prebuilt not found at ${V8_DIR}. Downloading from ${V8_URL}...")
    file(DOWNLOAD ${V8_URL} ${V8_ARCHIVE} SHOW_PROGRESS STATUS V8_DOWNLOAD_STATUS)
    list(GET V8_DOWNLOAD_STATUS 0 V8_STATUS_CODE)
    if(NOT V8_STATUS_CODE EQUAL 0)
      message(FATAL_ERROR "Failed to download V8 from ${V8_URL}")
    endif()
    message(STATUS "Extracting V8 archive into ${V8_DIR} ...")
    file(MAKE_DIRECTORY "${V8_DIR}")
    file(ARCHIVE_EXTRACT INPUT ${V8_ARCHIVE} DESTINATION "${V8_DIR}")
    file(REMOVE ${V8_ARCHIVE})
  endif()

  set(V8_ROOT "${V8_DIR}")
  message(STATUS "V8 root: ${V8_ROOT}")

  add_library(AureliaV8 INTERFACE)
  target_include_directories(AureliaV8 INTERFACE "${V8_ROOT}/include")
  target_link_libraries(AureliaV8 INTERFACE "${V8_ROOT}/lib/v8_monolith.lib")
endif()

include_directories(
    include
    external/prebuilt/skia
    external/prebuilt/skia/include
    external/yoga
    external/rtaudio
    external/nlohmann-json
    external/yaml-cpp/include
)

set(SKIA_LIB_DIR ${SKIA_DIR}/lib)

# ── Yoga layout engine ────────────────────────────────────────────────────────
set(YOGA_SOURCES
    external/yoga/yoga/YGConfig.cpp
    external/yoga/yoga/YGEnums.cpp
    external/yoga/yoga/YGNode.cpp
    external/yoga/yoga/YGNodeLayout.cpp
    external/yoga/yoga/YGNodeStyle.cpp
    external/yoga/yoga/YGPixelGrid.cpp
    external/yoga/yoga/YGValue.cpp
    external/yoga/yoga/algorithm/AbsoluteLayout.cpp
    external/yoga/yoga/algorithm/Baseline.cpp
    external/yoga/yoga/algorithm/Cache.cpp
    external/yoga/yoga/algorithm/CalculateLayout.cpp
    external/yoga/yoga/algorithm/FlexLine.cpp
    external/yoga/yoga/algorithm/PixelGrid.cpp
    external/yoga/yoga/config/Config.cpp
    external/yoga/yoga/debug/AssertFatal.cpp
    external/yoga/yoga/debug/Log.cpp
    external/yoga/yoga/event/event.cpp
    external/yoga/yoga/node/LayoutResults.cpp
    external/yoga/yoga/node/Node.cpp
)

# ── Resource generation ───────────────────────────────────────────────────────
find_package(Python3 REQUIRED)
set(RESOURCES_XML        ${CMAKE_CURRENT_SOURCE_DIR}/example/shared/resources/resrouces.xml)
set(GENERATED_RESOURCES_H   ${CMAKE_CURRENT_BINARY_DIR}/generated/BinaryResources.hpp)
set(GENERATED_RESOURCES_CPP ${CMAKE_CURRENT_BINARY_DIR}/generated/BinaryResources.cpp)

add_custom_command(
    OUTPUT ${GENERATED_RESOURCES_H} ${GENERATED_RESOURCES_CPP}
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_resources.py
            ${RESOURCES_XML} ${GENERATED_RESOURCES_H} ${GENERATED_RESOURCES_CPP}
    DEPENDS ${RESOURCES_XML} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen_resources.py
    COMMENT "Generating binary resources from ${RESOURCES_XML}"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)

# ── Shared compile definitions ────────────────────────────────────────────────
set(MOCHIKIT_DEFINITIONS
    SKIA_STATIC
    SK_RELEASE
    SK_DISABLE_LEGACY_PATH_METHODS
)
if(WIN32)
  list(APPEND MOCHIKIT_DEFINITIONS
        SK_D3D
        SK_DIRECTWRITE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
elseif(UNIX AND NOT APPLE)
  list(APPEND MOCHIKIT_DEFINITIONS
        SK_GL
        SK_VULKAN
    )
endif()

# Skia + system libraries (used by Foundation; inherited downstream)
if(WIN32)
  set(MOCHIKIT_LIBS
        ${SKIA_LIB_DIR}/skia.lib
        ${SKIA_LIB_DIR}/libpng.lib
        ${SKIA_LIB_DIR}/zlib.lib
        ${SKIA_LIB_DIR}/d3d12allocator.lib
        d3d12.lib
        dxgi.lib
        d3dcompiler.lib
        Dwrite.lib
        User32.lib
        Gdi32.lib
        Ole32.lib
        Shell32.lib
        Usp10.lib
        Dwmapi.lib
        Shlwapi.lib
        ComCtl32.lib
        Imm32.lib
        Pdh.lib
        Psapi.lib
    )
elseif(UNIX AND NOT APPLE)
  find_package(Threads    REQUIRED)
  find_package(OpenGL     REQUIRED)
  find_package(Freetype   REQUIRED)
  find_package(Fontconfig REQUIRED)

  # The Linux prebuilt drop ships every Skia module + every vendored
  # third-party as its own libfoo.a. List them in dependency order so that
  # static-link symbol resolution happens in a single pass.
  set(MOCHIKIT_LIBS
        ${SKIA_LIB_DIR}/libskia.a
        ${SKIA_LIB_DIR}/libskcms.a
        ${SKIA_LIB_DIR}/libpng.a
        ${SKIA_LIB_DIR}/libjpeg.a
        ${SKIA_LIB_DIR}/libwebp.a
        ${SKIA_LIB_DIR}/libwuffs.a
        ${SKIA_LIB_DIR}/libz.a
        ${SKIA_LIB_DIR}/libfreetype2.a
        ${SKIA_LIB_DIR}/libharfbuzz.a
        ${SKIA_LIB_DIR}/libicu.a
        ${SKIA_LIB_DIR}/libexpat.a
        Fontconfig::Fontconfig
        Freetype::Freetype
        OpenGL::GL
        Threads::Threads
        ${CMAKE_DL_LIBS}
        m
    )
endif()

# ── Skia SVG module (compiled in-tree because the prebuilt skia.lib does not
#    bundle the SVG renderer). The SVG sources live alongside the prebuilt
#    headers; they reference internal Skia headers (src/xml/SkDOM.h, etc.) which
#    are only present in the full source tree at external/skia.
#    SVG text also pulls in SkShaper, so we compile the primitive (no
#    harfbuzz/icu) shaper from the same Skia source drop. ──────────────────────
set(SKIA_SVG_SOURCES
    external/prebuilt/skia/modules/svg/src/SkSVGAttribute.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGAttributeParser.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGCircle.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGClipPath.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGContainer.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGDOM.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGEllipse.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFe.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeBlend.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeColorMatrix.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeComponentTransfer.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeComposite.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeDisplacementMap.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeFlood.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeGaussianBlur.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeImage.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeLightSource.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeLighting.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeMerge.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeMorphology.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeOffset.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFeTurbulence.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFilter.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGFilterContext.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGGradient.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGImage.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGLine.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGLinearGradient.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGMask.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGNode.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGOpenTypeSVGDecoder.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGPath.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGPattern.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGPoly.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGRadialGradient.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGRect.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGRenderContext.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGSVG.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGShape.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGStop.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGText.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGTransformableNode.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGUse.cpp
    external/prebuilt/skia/modules/svg/src/SkSVGValue.cpp
)

set(SKIA_SHAPER_SOURCES
    external/prebuilt/skia/modules/skshaper/src/SkShaper.cpp
    external/prebuilt/skia/modules/skshaper/src/SkShaper_factory.cpp
    external/prebuilt/skia/modules/skshaper/src/SkShaper_primitive.cpp
)

add_library(skia_svg STATIC ${SKIA_SVG_SOURCES} ${SKIA_SHAPER_SOURCES})
target_include_directories(skia_svg PRIVATE
    ${SKIA_DIR}
    ${SKIA_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/skia
)
target_compile_definitions(skia_svg PRIVATE ${MOCHIKIT_DEFINITIONS})
if(MSVC)
  target_compile_options(skia_svg PRIVATE /wd4244 /wd4267 /wd4291 /wd4146 /wd4661)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  AureliaKit.Foundation  —  core windowing, rendering, fonts, resources
# ══════════════════════════════════════════════════════════════════════════════
set(FOUNDATION_SOURCES
    ${YOGA_SOURCES}
    src/core/Application.cpp
    src/core/events/EventKind.cpp
    src/core/ResourceManager.cpp
    src/utils/FontManager/FontMgr.cpp
    src/utils/Misc/ThemeSwitcher.cpp
    ${GENERATED_RESOURCES_H}
    ${GENERATED_RESOURCES_CPP}
)
if(WIN32)
  list(APPEND FOUNDATION_SOURCES src/platform/windows/Window.cpp)
endif()
# TODO(linux): add src/platform/linux/Window.cpp once an X11/Wayland backend exists.

add_library(AureliaKit_Foundation SHARED ${FOUNDATION_SOURCES})
set_target_properties(AureliaKit_Foundation PROPERTIES
    OUTPUT_NAME "AureliaKit.Foundation"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(AureliaKit_Foundation PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(AureliaKit_Foundation PUBLIC ${MOCHIKIT_LIBS})

# ══════════════════════════════════════════════════════════════════════════════
#  AureliaKit.GraphicInterface  —  widget, menu bar, overlay, AureliaUI layer
# ══════════════════════════════════════════════════════════════════════════════
set(GRAPHIC_INTERFACE_SOURCES
    src/core/AureliaWidget.cpp
    src/gui/AureliaUI.cpp
    src/gui/MenuBar.cpp
    src/gui/AUKImage.cpp
    src/gui/AUKSvg.cpp
)

add_library(AureliaKit_GraphicInterface SHARED ${GRAPHIC_INTERFACE_SOURCES})
set_target_properties(AureliaKit_GraphicInterface PROPERTIES
    OUTPUT_NAME "AureliaKit.GraphicInterface"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(AureliaKit_GraphicInterface PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(AureliaKit_GraphicInterface
    PUBLIC AureliaKit_Foundation
    PRIVATE skia_svg
)

# ══════════════════════════════════════════════════════════════════════════════
#  AureliaKit.GraphicComponent  —  all UI components
# ══════════════════════════════════════════════════════════════════════════════
set(GRAPHIC_COMPONENT_SOURCES
    src/gui/Components/TextNode.cpp
    src/gui/Components/CheckboxNode.cpp
    src/gui/Components/SliderNode.cpp
    src/gui/Components/KnobNode.cpp
    src/gui/Components/VUMeterNode.cpp
    src/gui/Components/ScrollAreaNode.cpp
    src/gui/Components/GroupBox.cpp
    src/gui/Components/TextInput.cpp
    src/gui/Components/NumberInput.cpp
    src/gui/Components/ComboBox.cpp
    src/gui/Components/Table.cpp
    src/gui/Components/ColorPicker.cpp
    src/gui/Components/Calendar.cpp
    src/gui/Components/IconNode.cpp
    src/gui/Components/DatePicker.cpp
    src/gui/Components/Popover.cpp
    src/gui/Components/SwitchNode.cpp
    src/gui/Components/RadioButtonNode.cpp
)

add_library(AureliaKit_GraphicComponent SHARED ${GRAPHIC_COMPONENT_SOURCES})
set_target_properties(AureliaKit_GraphicComponent PROPERTIES
    OUTPUT_NAME "AureliaKit.GraphicComponent"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(AureliaKit_GraphicComponent PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(AureliaKit_GraphicComponent PUBLIC AureliaKit_GraphicInterface)

# ══════════════════════════════════════════════════════════════════════════════
#  AureliaKit.DSL  —  YAML/CSS-style declarative UI loader
# ══════════════════════════════════════════════════════════════════════════════
# yaml-cpp ships its own CMakeLists. Pull it in as a static library so the DSL
# loader can be linked without forcing every consumer to build YAML support.
set(YAML_CPP_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB  OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL        OFF CACHE BOOL "" FORCE)
set(YAML_CPP_FORMAT_SOURCE  OFF CACHE BOOL "" FORCE)
set(YAML_BUILD_SHARED_LIBS  OFF CACHE BOOL "" FORCE)
set(YAML_MSVC_SHARED_RT     OFF CACHE BOOL "" FORCE)
add_subdirectory(external/yaml-cpp EXCLUDE_FROM_ALL)

add_subdirectory(external/libcss EXCLUDE_FROM_ALL)

set(DSL_SOURCES
    src/dsl/DSL.cpp
    src/dsl/LibCssStyle.cpp
)

add_library(AureliaKit_DSL SHARED ${DSL_SOURCES})
set_target_properties(AureliaKit_DSL PROPERTIES
    OUTPUT_NAME "AureliaKit.DSL"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(AureliaKit_DSL PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(AureliaKit_DSL
    PUBLIC  AureliaKit_GraphicComponent
    PRIVATE yaml-cpp::yaml-cpp
            css
)

# ══════════════════════════════════════════════════════════════════════════════
#  AureliaKit.JavaScriptEngine  —  V8 embed + native bindings
# ══════════════════════════════════════════════════════════════════════════════
set(JAVASCRIPT_ENGINE_SOURCES
    src/javascript/JavaScriptEngine.cpp
)

add_library(AureliaKit_JavaScriptEngine SHARED ${JAVASCRIPT_ENGINE_SOURCES})
set_target_properties(AureliaKit_JavaScriptEngine PROPERTIES
    OUTPUT_NAME "AureliaKit.JavaScriptEngine"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(AureliaKit_JavaScriptEngine
    PUBLIC  ${MOCHIKIT_DEFINITIONS}
    PRIVATE AUKJS_BUILD_SHARED
)
target_link_libraries(AureliaKit_JavaScriptEngine
    PUBLIC  AureliaKit_GraphicComponent
    PRIVATE AureliaV8
            dbghelp.lib
            winmm.lib
)

# ══════════════════════════════════════════════════════════════════════════════
#  Aurelia.React  —  JS reconciler bridge
# ══════════════════════════════════════════════════════════════════════════════
set(AURELIA_REACT_SOURCES
    src/react/AureliaReact.cpp
)

add_library(Aurelia_React SHARED ${AURELIA_REACT_SOURCES})
set_target_properties(Aurelia_React PROPERTIES
    OUTPUT_NAME "Aurelia.React"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(Aurelia_React
    PUBLIC  ${MOCHIKIT_DEFINITIONS}
    PRIVATE AUR_BUILD_SHARED
)
target_link_libraries(Aurelia_React
    PUBLIC AureliaKit_GraphicComponent
           AureliaKit_JavaScriptEngine
)

# ══════════════════════════════════════════════════════════════════════════════
#  AureliaKit.DirectAudioEngine  —  WASAPI / DirectSound audio (Windows-only)
# ══════════════════════════════════════════════════════════════════════════════
if(WIN32)
  set(DIRECT_AUDIO_SOURCES
        src/audio/AUKDirectAudioEngine.cpp
        src/audio/Streaming.cpp
        external/rtaudio/RtAudio.cpp
    )

  add_library(AureliaKit_DirectAudioEngine SHARED ${DIRECT_AUDIO_SOURCES})
  set_target_properties(AureliaKit_DirectAudioEngine PROPERTIES
        OUTPUT_NAME "AureliaKit.DirectAudioEngine"
    )
  target_compile_definitions(AureliaKit_DirectAudioEngine PRIVATE DAUX_EXPORTS)
  target_compile_definitions(AureliaKit_DirectAudioEngine PUBLIC
        __WINDOWS_WASAPI__
        __WINDOWS_DS__
    )
  target_link_libraries(AureliaKit_DirectAudioEngine PRIVATE
        mfplat.lib
        mfuuid.lib
        wmcodecdspuuid.lib
        ole32.lib
        User32.lib
        dsound.lib
    )
endif()
# TODO(linux): wire ALSA / PulseAudio / PipeWire backends behind the same target.

# ── Examples ──────────────────────────────────────────────────────────────────
# The examples currently embed a Win32 .rc and use the WIN32 GUI subsystem;
# they only build on Windows until they grow a portable entry point.
if(WIN32)
  add_executable(AureliaExample WIN32
        example/main.cpp
        example/app.rc
    )
  target_link_libraries(AureliaExample PRIVATE
        AureliaKit_GraphicComponent
    )

  add_subdirectory(example/ActivityMonitor)
  add_subdirectory(example/Mixer)
  add_subdirectory(example/Borderless)
  add_subdirectory(example/DSLDemo)
endif()

# ── Frameworks / modules ─────────────────────────────────────────────────────
add_subdirectory(framework/MikoUI)
