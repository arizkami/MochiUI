cmake_minimum_required(VERSION 3.10)
project(SkiaGUIExample)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Match Skia's Official Build Runtime and Architecture
if(MSVC)
    add_compile_options(
        $<$<CONFIG:Release>:/MT>
        $<$<CONFIG:Debug>:/MTd>
        /arch:AVX2  
    )
endif()

include_directories(
    .
    include
    External/skia/include
)

set(SOURCES
    src/main.cpp
    src/gui/MochiUI.cpp
    src/gui/MenuBar.cpp
    src/gui/Components/TextNode.cpp
    src/gui/Components/CheckboxNode.cpp
    src/gui/Components/SliderNode.cpp
    src/gui/Components/KnobNode.cpp
    src/gui/Components/VUMeterNode.cpp
    src/gui/Components/ScrollAreaNode.cpp
    src/utils/FontManager/FontMgr.cpp
    src/utils/Misc/ThemeSwitcher.cpp
)

add_executable(SkiaGUIExample WIN32 ${SOURCES})

set(SKIA_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/skia/lib)

# 2. Updated Definitions to match modern Skia Builders
target_compile_definitions(SkiaGUIExample PRIVATE
    SKIA_STATIC
    SK_RELEASE
    # Backend Alignment
    SK_D3D
    SK_DIRECTWRITE
    # Force the headers to use the same logic as your .lib
    SK_DISABLE_LEGACY_PATH_METHODS
    # Windows defaults
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)

target_link_libraries(SkiaGUIExample
    PRIVATE
    ${SKIA_LIB_DIR}/skia.lib
    ${SKIA_LIB_DIR}/libpng.lib
    ${SKIA_LIB_DIR}/zlib.lib
    # Windows System Libraries
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
)