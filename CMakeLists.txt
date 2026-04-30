cmake_minimum_required(VERSION 3.15)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)

project(MochiUI)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Match Skia's official build runtime and architecture
if(MSVC)
    string(REPLACE "/MD"  "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
    string(REPLACE "/MDd" "" CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}")
    add_compile_options(/arch:AVX2 /wd4244 /wd4267)
endif()

# ── Skia prebuilt binaries ────────────────────────────────────────────────────
set(SKIA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/prebuilt/skia)
set(SKIA_ZIP ${CMAKE_CURRENT_SOURCE_DIR}/External/skia-prebuilt-binary.zip)
set(SKIA_URL "https://github.com/arizkami/MochiUI/releases/download/2026.04/skia-prebuilt-binary.zip")

if(NOT EXISTS ${SKIA_DIR}/lib/skia.lib)
    message(STATUS "Skia binaries not found. Downloading...")
    file(DOWNLOAD ${SKIA_URL} ${SKIA_ZIP} SHOW_PROGRESS STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
        message(FATAL_ERROR "Failed to download Skia binaries from ${SKIA_URL}")
    endif()
    message(STATUS "Extracting Skia binaries...")
    file(ARCHIVE_EXTRACT INPUT ${SKIA_ZIP}
         DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/External/prebuilt/skia)
    file(REMOVE ${SKIA_ZIP})
    message(STATUS "Skia binaries installed successfully!")
else()
    message(STATUS "Skia binaries found at ${SKIA_DIR}")
endif()

include_directories(
    include
    External/prebuilt/skia
    External/prebuilt/skia/include
    External/yoga
    External/rtaudio
)

set(SKIA_LIB_DIR ${SKIA_DIR}/lib)

# ── Yoga layout engine ────────────────────────────────────────────────────────
set(YOGA_SOURCES
    External/yoga/yoga/YGConfig.cpp
    External/yoga/yoga/YGEnums.cpp
    External/yoga/yoga/YGNode.cpp
    External/yoga/yoga/YGNodeLayout.cpp
    External/yoga/yoga/YGNodeStyle.cpp
    External/yoga/yoga/YGPixelGrid.cpp
    External/yoga/yoga/YGValue.cpp
    External/yoga/yoga/algorithm/AbsoluteLayout.cpp
    External/yoga/yoga/algorithm/Baseline.cpp
    External/yoga/yoga/algorithm/Cache.cpp
    External/yoga/yoga/algorithm/CalculateLayout.cpp
    External/yoga/yoga/algorithm/FlexLine.cpp
    External/yoga/yoga/algorithm/PixelGrid.cpp
    External/yoga/yoga/config/Config.cpp
    External/yoga/yoga/debug/AssertFatal.cpp
    External/yoga/yoga/debug/Log.cpp
    External/yoga/yoga/event/event.cpp
    External/yoga/yoga/node/LayoutResults.cpp
    External/yoga/yoga/node/Node.cpp
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
    SK_D3D
    SK_DIRECTWRITE
    SK_DISABLE_LEGACY_PATH_METHODS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

# Skia + D3D12 + Windows system libraries (used by Foundation; inherited downstream)
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

# ══════════════════════════════════════════════════════════════════════════════
#  MochiKit.Foundation  —  core windowing, rendering, fonts, resources
# ══════════════════════════════════════════════════════════════════════════════
set(FOUNDATION_SOURCES
    ${YOGA_SOURCES}
    src/core/Application.cpp
    src/core/ResourceManager.cpp
    src/utils/FontManager/FontMgr.cpp
    src/utils/Misc/ThemeSwitcher.cpp
    src/platform/windows/Window.cpp
    ${GENERATED_RESOURCES_H}
    ${GENERATED_RESOURCES_CPP}
)

add_library(MochiKit_Foundation SHARED ${FOUNDATION_SOURCES})
set_target_properties(MochiKit_Foundation PROPERTIES
    OUTPUT_NAME "MochiKit.Foundation"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(MochiKit_Foundation PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(MochiKit_Foundation PUBLIC ${MOCHIKIT_LIBS})

# ══════════════════════════════════════════════════════════════════════════════
#  MochiKit.GraphicInterface  —  widget, menu bar, overlay, MochiUI layer
# ══════════════════════════════════════════════════════════════════════════════
set(GRAPHIC_INTERFACE_SOURCES
    src/core/MochiWidget.cpp
    src/gui/MochiUI.cpp
    src/gui/MenuBar.cpp
)

add_library(MochiKit_GraphicInterface SHARED ${GRAPHIC_INTERFACE_SOURCES})
set_target_properties(MochiKit_GraphicInterface PROPERTIES
    OUTPUT_NAME "MochiKit.GraphicInterface"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(MochiKit_GraphicInterface PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(MochiKit_GraphicInterface PUBLIC MochiKit_Foundation)

# ══════════════════════════════════════════════════════════════════════════════
#  MochiKit.GraphicComponent  —  all UI components
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

add_library(MochiKit_GraphicComponent SHARED ${GRAPHIC_COMPONENT_SOURCES})
set_target_properties(MochiKit_GraphicComponent PROPERTIES
    OUTPUT_NAME "MochiKit.GraphicComponent"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(MochiKit_GraphicComponent PUBLIC ${MOCHIKIT_DEFINITIONS})
target_link_libraries(MochiKit_GraphicComponent PUBLIC MochiKit_GraphicInterface)

# ══════════════════════════════════════════════════════════════════════════════
#  MochiKit.DirectAudioEngine  —  WASAPI / DirectSound audio (was DAUx)
# ══════════════════════════════════════════════════════════════════════════════
set(DIRECT_AUDIO_SOURCES
    src/audio/MCKDirectAudioEngine.cpp
    External/rtaudio/RtAudio.cpp
)

add_library(MochiKit_DirectAudioEngine SHARED ${DIRECT_AUDIO_SOURCES})
set_target_properties(MochiKit_DirectAudioEngine PROPERTIES
    OUTPUT_NAME "MochiKit.DirectAudioEngine"
)
target_compile_definitions(MochiKit_DirectAudioEngine PRIVATE DAUX_EXPORTS)
target_compile_definitions(MochiKit_DirectAudioEngine PUBLIC
    __WINDOWS_WASAPI__
    __WINDOWS_DS__
)
target_link_libraries(MochiKit_DirectAudioEngine PRIVATE
    mfplat.lib
    mfuuid.lib
    wmcodecdspuuid.lib
    ole32.lib
    User32.lib
    dsound.lib
)

# ── Examples ──────────────────────────────────────────────────────────────────
add_executable(MochiExample WIN32
    example/main.cpp
    example/app.rc
)
target_link_libraries(MochiExample PRIVATE
    MochiKit_GraphicComponent
)

add_subdirectory(example/ActivityMonitor)
