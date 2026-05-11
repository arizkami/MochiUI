cmake_minimum_required(VERSION 3.15)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(Sphere)

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
  set(SKIA_URL      "https://github.com/arizkami/SphereEngine/releases/download/2026.05/skia-prebuilt-binary.zip")
  set(SKIA_SENTINEL ${SKIA_DIR}/lib/skia.lib)
elseif(UNIX AND NOT APPLE)
  set(SKIA_ARCHIVE  ${CMAKE_CURRENT_SOURCE_DIR}/external/skia-prebuilt-linux.tar.gz)
  set(SKIA_URL      "https://github.com/arizkami/SphereEngine/releases/download/2026.05/skia-prebuilt-linux.tar.gz")
  set(SKIA_SENTINEL ${SKIA_DIR}/lib/libskia.a)
else()
  message(FATAL_ERROR "Sphere: unsupported platform '${CMAKE_SYSTEM_NAME}'. "
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
  set(V8_URL "https://github.com/arizkami/SphereEngine/releases/download/2026.04/v8-windows_x64.zip")
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

  add_library(SphereV8 INTERFACE)
  target_include_directories(SphereV8 INTERFACE "${V8_ROOT}/include")
  target_link_libraries(SphereV8 INTERFACE "${V8_ROOT}/lib/v8_monolith.lib")
  # The prebuilt v8_monolith was compiled with pointer compression + sandbox
  # enabled.  Every embedder TU that includes V8 headers must see the same
  # flags or the runtime config-mismatch check fires a fatal SIGTRAP.
  target_compile_definitions(SphereV8 INTERFACE
      V8_COMPRESS_POINTERS
      V8_COMPRESS_POINTERS_IN_SHARED_CAGE
  )

  # Copy V8 runtime DLLs and ICU data file to the binary dir so every
  # executable can find them without a manual PATH adjustment.
  file(GLOB V8_RUNTIME_FILES
      "${V8_DIR}/bin/*.dll"
      "${V8_DIR}/bin/*.dat"
  )
  if(V8_RUNTIME_FILES)
    file(COPY ${V8_RUNTIME_FILES} DESTINATION "${CMAKE_BINARY_DIR}")
    message(STATUS "V8 runtime files staged to ${CMAKE_BINARY_DIR}")
  endif()
endif()

include_directories(
    include
    external/prebuilt/skia
    external/prebuilt/skia/include
    external/yoga
    external/rtaudio
    external/nlohmann-json
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
if (WIN32)
    find_program(SPHX_PYTHON_CMD NAMES py py.exe python python.exe)
endif()
if (NOT SPHX_PYTHON_CMD)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(SPHX_PYTHON_CMD ${Python3_EXECUTABLE})
endif()

# ── Bun / JS workspace setup ─────────────────────────────────────────────────
set(SPHERE_MODULES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/modules)

find_program(BUN_EXECUTABLE
    NAMES bun bun.exe
    PATHS
        "$ENV{USERPROFILE}/.bun/bin"
        "$ENV{HOME}/.bun/bin"
)

if(WIN32 AND NOT BUN_EXECUTABLE AND NOT DEFINED ENV{CI})
    message(STATUS "Bun not found. Installing Bun with PowerShell...")
    execute_process(
        COMMAND powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "irm bun.sh/install.ps1|iex"
        RESULT_VARIABLE BUN_INSTALL_RESULT
    )
    if(NOT BUN_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install Bun. Install it manually or rerun CMake after fixing the installer error.")
    endif()

    find_program(BUN_EXECUTABLE
        NAMES bun bun.exe
        PATHS
            "$ENV{USERPROFILE}/.bun/bin"
            "$ENV{HOME}/.bun/bin"
        NO_CACHE
    )
endif()

if(NOT BUN_EXECUTABLE)
    message(FATAL_ERROR "Bun is required to build the JavaScript UI bundles, but no bun executable was found.")
endif()

set(SPHERE_MODULES_INSTALL_STAMP ${CMAKE_BINARY_DIR}/modules-bun-install.stamp)
if(DEFINED ENV{CI} AND EXISTS "${SPHERE_MODULES_DIR}/node_modules")
    add_custom_command(
        OUTPUT  ${SPHERE_MODULES_INSTALL_STAMP}
        COMMAND ${CMAKE_COMMAND} -E touch ${SPHERE_MODULES_INSTALL_STAMP}
        DEPENDS
            ${SPHERE_MODULES_DIR}/package.json
            ${SPHERE_MODULES_DIR}/bun.lock
        COMMENT "Using JavaScript workspace dependencies installed by CI"
    )
else()
    add_custom_command(
        OUTPUT  ${SPHERE_MODULES_INSTALL_STAMP}
        COMMAND ${BUN_EXECUTABLE} install
        COMMAND ${CMAKE_COMMAND} -E touch ${SPHERE_MODULES_INSTALL_STAMP}
        WORKING_DIRECTORY ${SPHERE_MODULES_DIR}
        DEPENDS
            ${SPHERE_MODULES_DIR}/package.json
            ${SPHERE_MODULES_DIR}/bun.lock
        COMMENT "Installing JavaScript workspace dependencies (bun install)"
    )
endif()
add_custom_target(SphereModulesSetup DEPENDS ${SPHERE_MODULES_INSTALL_STAMP})

set(RESOURCES_XML        ${CMAKE_CURRENT_SOURCE_DIR}/example/shared/resources/resrouces.xml)
set(GENERATED_RESOURCES_H   ${CMAKE_CURRENT_BINARY_DIR}/generated/BinaryResources.hpp)
set(GENERATED_RESOURCES_CPP ${CMAKE_CURRENT_BINARY_DIR}/generated/BinaryResources.cpp)

add_custom_command(
    OUTPUT ${GENERATED_RESOURCES_H} ${GENERATED_RESOURCES_CPP}
    COMMAND ${SPHX_PYTHON_CMD}
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
#  SphereKit.Foundation  —  core windowing, rendering, fonts, resources
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

add_library(SphereKit_Foundation SHARED ${FOUNDATION_SOURCES})
set_target_properties(SphereKit_Foundation PROPERTIES
    OUTPUT_NAME "SphereKit.Foundation"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(SphereKit_Foundation PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(SphereKit_Foundation PUBLIC ${MOCHIKIT_LIBS})

# ══════════════════════════════════════════════════════════════════════════════
#  SphereKit.GraphicInterface  —  widget, menu bar, overlay, SphereUI layer
# ══════════════════════════════════════════════════════════════════════════════
set(GRAPHIC_INTERFACE_SOURCES
    src/core/SphereWidget.cpp
    src/gui/SphereUI.cpp
    src/gui/MenuBar.cpp
    src/gui/SPHXImage.cpp
    src/gui/SPHXSvg.cpp
)

add_library(SphereKit_GraphicInterface SHARED ${GRAPHIC_INTERFACE_SOURCES})
set_target_properties(SphereKit_GraphicInterface PROPERTIES
    OUTPUT_NAME "SphereKit.GraphicInterface"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(SphereKit_GraphicInterface PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(SphereKit_GraphicInterface
    PUBLIC SphereKit_Foundation
    PRIVATE skia_svg
)

# ══════════════════════════════════════════════════════════════════════════════
#  SphereKit.GraphicComponent  —  all UI components
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

add_library(SphereKit_GraphicComponent SHARED ${GRAPHIC_COMPONENT_SOURCES})
set_target_properties(SphereKit_GraphicComponent PROPERTIES
    OUTPUT_NAME "SphereKit.GraphicComponent"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(SphereKit_GraphicComponent PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(SphereKit_GraphicComponent PUBLIC SphereKit_GraphicInterface)

# ══════════════════════════════════════════════════════════════════════════════
#  SphereKit.JavaScriptEngine  —  V8 embed + native bindings
# ══════════════════════════════════════════════════════════════════════════════
set(JAVASCRIPT_ENGINE_SOURCES
    src/javascript/JavaScriptEngine.cpp
)

add_library(SphereKit_JavaScriptEngine SHARED ${JAVASCRIPT_ENGINE_SOURCES})
set_target_properties(SphereKit_JavaScriptEngine PROPERTIES
    OUTPUT_NAME "SphereKit.JavaScriptEngine"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(SphereKit_JavaScriptEngine
    PUBLIC  ${MOCHIKIT_DEFINITIONS}
    PRIVATE SPHXJS_BUILD_SHARED
)
target_link_libraries(SphereKit_JavaScriptEngine
    PUBLIC  SphereKit_GraphicComponent
    PRIVATE SphereV8
            dbghelp.lib
            winmm.lib
)

# ══════════════════════════════════════════════════════════════════════════════
#  Sphere.React  —  JS reconciler bridge
# ══════════════════════════════════════════════════════════════════════════════
set(Sphere_REACT_SOURCES
    src/react/SphereReact.cpp
)

add_library(Sphere_React SHARED ${Sphere_REACT_SOURCES})
set_target_properties(Sphere_React PROPERTIES
    OUTPUT_NAME "Sphere.React"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(Sphere_React
    PUBLIC  ${MOCHIKIT_DEFINITIONS}
    PRIVATE AUR_BUILD_SHARED
)
target_link_libraries(Sphere_React
    PUBLIC SphereKit_GraphicComponent
           SphereKit_JavaScriptEngine
)

# ══════════════════════════════════════════════════════════════════════════════
#  SphereKit.Vue  —  JS renderer bridge
# ══════════════════════════════════════════════════════════════════════════════
set(Sphere_VUE_SOURCES
    src/vue/SphereVUE.cpp
)

add_library(Sphere_Vue SHARED ${Sphere_VUE_SOURCES})
set_target_properties(Sphere_Vue PROPERTIES
    OUTPUT_NAME "SphereKit.Vue"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(Sphere_Vue
    PUBLIC  ${MOCHIKIT_DEFINITIONS}
    PRIVATE AUR_BUILD_SHARED
)
target_link_libraries(Sphere_Vue
    PUBLIC SphereKit_GraphicComponent
           SphereKit_JavaScriptEngine
)

# ══════════════════════════════════════════════════════════════════════════════
#  SphereKit.DirectAudioEngine  —  WASAPI / DirectSound audio (Windows-only)
# ══════════════════════════════════════════════════════════════════════════════
if(WIN32)
  set(DIRECT_AUDIO_SOURCES
        src/audio/SPHXDirectAudioEngine.cpp
        src/audio/Streaming.cpp
        external/rtaudio/RtAudio.cpp
    )

  add_library(SphereKit_DirectAudioEngine SHARED ${DIRECT_AUDIO_SOURCES})
  set_target_properties(SphereKit_DirectAudioEngine PROPERTIES
        OUTPUT_NAME "SphereKit.DirectAudioEngine"
    )
  target_compile_definitions(SphereKit_DirectAudioEngine PRIVATE DAUX_EXPORTS)
  target_compile_definitions(SphereKit_DirectAudioEngine PUBLIC
        __WINDOWS_WASAPI__
        __WINDOWS_DS__
    )
  target_link_libraries(SphereKit_DirectAudioEngine PRIVATE
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
  add_executable(SphereExample WIN32
        example/main.cpp
        example/app.rc
    )
  target_link_libraries(SphereExample PRIVATE
        SphereKit_GraphicComponent
    )

  add_subdirectory(example/ActivityMonitor)
  add_subdirectory(example/Mixer)
  add_subdirectory(example/Borderless)
  add_subdirectory(example/ReactUI)
  add_subdirectory(example/VueUI)
endif()
